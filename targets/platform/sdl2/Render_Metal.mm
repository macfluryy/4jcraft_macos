#ifdef USE_METAL

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <SDL.h>
#include <SDL_metal.h>
#include <SDL_syswm.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../PlatformTypes.h"
#include "Render.h"

#define CPP_MSL_INCLUDE
static const char* MSL_SRC =
#include "sdl2/shaders/shader.metal"
    ;
#undef CPP_MSL_INCLUDE

C4JRender RenderManager;

struct alignas(16) MetalUniforms {
    glm::mat4 uMVP;           // 0
    glm::mat4 uMV;            // 64
    glm::mat4 uTexMat0;       // 128
    glm::vec4 uNormalRow0;    // 192
    glm::vec4 uNormalRow1;    // 208
    glm::vec4 uNormalRow2;    // 224

    glm::vec4 uBaseColor;     // 240
    glm::vec4 uFogColor;      // 256
    glm::vec4 uLMTransform;   // 272

    glm::vec4 uLight0Dir;     // 288
    glm::vec4 uLight1Dir;     // 304
    glm::vec4 uLightDiffuse;  // 320
    glm::vec4 uLightAmbient;  // 336
    glm::vec4 uChunkOffset;   // 352
    glm::vec4 uScalars;       // 368: fogStart, fogEnd, fogDensity, alphaRef
    glm::vec4 uScalars2;      // 384: invGamma, normalSign, globalLM.x, globalLM.y
    glm::ivec4 uFlags;        // 400: lighting, fogMode, fogEnable, useTexture
    glm::ivec4 uFlags2;       // 416: useLightmap, srgbOutput, 0, 0
};

static SDL_Window* s_window = nullptr;
static SDL_MetalView s_metalView = nullptr;
static bool s_shouldClose = false;
static int s_windowWidth = 1920;
static int s_windowHeight = 1080;
static int s_reqWidth = 1920;
static int s_reqHeight = 1080;
static bool s_fullscreen = false;

static id<MTLDevice> s_device = nil;
static id<MTLCommandQueue> s_cmdQueue = nil;
static id<MTLLibrary> s_library = nil;
static id<MTLFunction> s_vertFn = nil;
static id<MTLFunction> s_fragFn = nil;
static MTLVertexDescriptor* s_vertDesc = nil;
static id<MTLRenderPipelineState> s_pipelineOpaque = nil;
static id<MTLRenderPipelineState> s_pipelineBlend = nil;
static std::unordered_map<uint64_t, id<MTLRenderPipelineState>> s_pipelineBlendCache;
static id<MTLDepthStencilState> s_dsStateDefault = nil;
static id<MTLDepthStencilState> s_dsStateNoWrite = nil;
static id<MTLDepthStencilState> s_dsStateDisabled = nil;
static id<MTLSamplerState> s_sampNearest = nil;
static id<MTLSamplerState> s_sampLinear = nil;
static CAMetalLayer* s_metalLayer = nil;

static id<MTLCommandBuffer> s_cmdBuf = nil;
static id<MTLRenderCommandEncoder> s_encoder = nil;
static id<CAMetalDrawable> s_drawable = nil;
static id<MTLTexture> s_depthTex = nil;
static id<MTLTexture> s_stencilTex = nil;

static const int FRAMES_IN_FLIGHT = 3;
static const int UNIFORM_ALIGN = 256;
static const int UNIFORM_SLOT = (sizeof(MetalUniforms) + UNIFORM_ALIGN - 1) & ~(UNIFORM_ALIGN - 1);
static const int UNIFORM_RING_BYTES = 16 * 1024 * 1024;
static id<MTLBuffer> s_uniformBufs[FRAMES_IN_FLIGHT] = {};
static int s_uniformOffset = 0;
static int s_frameIndex = 0;
static dispatch_semaphore_t s_frameSema = nullptr;

static const int STREAM_VBO_SIZE = 16 * 1024 * 1024;
static id<MTLBuffer> s_streamBufs[FRAMES_IN_FLIGHT] = {};
static int s_streamOffset = 0;
static id<MTLBuffer> s_quadIBO = nil;
static int s_quadIBOMaxQuads = 0;
static id<MTLTexture> s_whiteTex = nil;

static const int STACK_DEPTH = 64;
struct MatrixStack {
    glm::mat4 stack[STACK_DEPTH];
    int top = 0;
    MatrixStack() { stack[0] = glm::mat4(1.f); }
    glm::mat4& cur() { return stack[top]; }
    void push() {
        if (top < STACK_DEPTH - 1) {
            stack[top + 1] = stack[top];
            ++top;
        }
    }
    void pop() {
        if (top > 0) --top;
    }
    void load(const glm::mat4& m) { cur() = m; }
    void mul(const glm::mat4& m) { cur() = cur() * m; }
};
static thread_local MatrixStack s_proj, s_mv, s_tex[2];
static thread_local int s_matMode = 0;

static thread_local bool s_uniformDirty = true;
static bool s_needsFirstEncoderClear = true;

static inline MatrixStack& activeStack() {
    switch (s_matMode) {
        case 1:
            return s_proj;
        case 2:
            return s_tex[0];
        case 3:
            return s_tex[1];
    }
    return s_mv;
}

static inline void markDirty() { s_uniformDirty = true; }

struct RenderState {
    glm::vec4 baseColor = {1, 1, 1, 1};
    glm::vec4 fogColor = {0, 0, 0, 1};
    float fogStart = 0, fogEnd = 1000, fogDensity = 0;
    int fogMode = 0;
    bool fogEnable = false;
    float alphaRef = 0.1f;
    float gamma = 1.0f;
    bool useTexture = true, useLightmap = false, lighting = false;
    glm::vec3 l0 = {0.173913f, 0.869565f, -0.608696f};
    glm::vec3 l1 = {-0.173913f, 0.869565f, 0.608696f};
    glm::vec3 ldiff = {0.6f, 0.6f, 0.6f};
    glm::vec3 lamb = {0.4f, 0.4f, 0.4f};
    glm::vec4 lmt = {1, 1, 0, 0};
    glm::vec2 globalLM = {240.f, 240.f};
    int activeTexture = 0;

    bool blendEnable = true;
    bool cullEnable = true;
    bool depthEnable = true;
    bool depthWrite = true;
    bool colorWrite[4] = {true, true, true, true};
    int blendSrc = 0x0302;  // GL_SRC_ALPHA
    int blendDst = 0x0303;  // GL_ONE_MINUS_SRC_ALPHA
    bool frontFaceCW = false;

    glm::vec3 chunkOffset = {0, 0, 0};
    float clearColor[4] = {0, 0, 0, 1};
};
static thread_local RenderState s_rs;

struct MetalTexEntry {
    id<MTLTexture> tex = nil;
    int w = 0, h = 0;
    int maxLevel = 0;
};
static std::unordered_map<int, MetalTexEntry> s_textures;
static int s_nextTexId = 1;
static int s_boundTex0 = -1;
static int s_boundTex1 = -1;

struct ChunkDrawCall {
    int prim;
    int first;
    int count;
};
struct ChunkBuffer {
    id<MTLBuffer> vbo = nil;
    std::vector<ChunkDrawCall> draws;
    std::vector<uint8_t> rawVerts;
    bool valid = false;
    bool uploaded = false;
    void destroy() {
        vbo = nil;
        draws.clear();
        rawVerts.clear();
        rawVerts.shrink_to_fit();
        valid = false;
        uploaded = false;
    }
};
static std::unordered_map<int, ChunkBuffer> s_chunkPool;
static int s_nextListBase = 1;
static std::mutex s_chunkMtx;

static thread_local int s_recListId = -1;
static thread_local std::vector<uint8_t> s_recVerts;
static thread_local std::vector<ChunkDrawCall> s_recDraws;

static bool isQuadPrim(int pt) {
    return (pt == 0x0007 || pt == (int)C4JRender::PRIMITIVE_TYPE_QUAD_LIST);
}
static MTLPrimitiveType mapPrim(int pt) {
    if (isQuadPrim(pt)) return MTLPrimitiveTypeTriangle;
    switch (pt) {
        case 0:
            return MTLPrimitiveTypeTriangle;
        case 1:
            return MTLPrimitiveTypeLine;
        case 2:
            return MTLPrimitiveTypeTriangle;
        case 3:
            return MTLPrimitiveTypeLineStrip;
        case 4:
            return MTLPrimitiveTypeTriangle;
        case 5:
            return MTLPrimitiveTypeTriangleStrip;
        default:
            return MTLPrimitiveTypeTriangle;
    }
}

static void ensureQuadIBO(int numQuads) {
    if (numQuads <= s_quadIBOMaxQuads) return;
    int cap = numQuads < 4096 ? 4096 : numQuads;
    std::vector<uint32_t> idx(cap * 6);
    for (int q = 0; q < cap; q++) {
        uint32_t base = (uint32_t)(q * 4);
        idx[q * 6 + 0] = base;
        idx[q * 6 + 1] = base + 1;
        idx[q * 6 + 2] = base + 2;
        idx[q * 6 + 3] = base;
        idx[q * 6 + 4] = base + 2;
        idx[q * 6 + 5] = base + 3;
    }
    s_quadIBO = [s_device newBufferWithBytes:idx.data()
                                      length:cap * 6 * sizeof(uint32_t)
                                     options:MTLResourceStorageModeShared];
    s_quadIBOMaxQuads = cap;
}

static void createDepthStencilTextures(int w, int h) {
    MTLTextureDescriptor* dd =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float_Stencil8
                                                           width:w
                                                          height:h
                                                       mipmapped:NO];
    dd.usage = MTLTextureUsageRenderTarget;
    dd.storageMode = MTLStorageModePrivate;
    s_depthTex = [s_device newTextureWithDescriptor:dd];
    s_stencilTex = s_depthTex;
}

static id<MTLTexture> createWhiteTexture() {
    MTLTextureDescriptor* td =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                           width:1
                                                          height:1
                                                       mipmapped:NO];
    td.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> t = [s_device newTextureWithDescriptor:td];
    uint32_t white = 0xFFFFFFFF;
    [t replaceRegion:MTLRegionMake2D(0, 0, 1, 1) mipmapLevel:0 withBytes:&white bytesPerRow:4];
    return t;
}

static int s_currentUniformOffset = 0;

static void flushUniforms() {
    if (!s_uniformDirty) {
        return;
    }
    s_uniformDirty = false;

    MetalUniforms u = {};
    // remap opengl-style clip-space z from [-1,1] to metal's [0,1]:
    // new_clip_z = 0.5 * old_clip_z + 0.5 * w
    glm::mat4 zRemap(1.0f);
    zRemap[2][2] = 0.5f;
    zRemap[3][2] = 0.5f;
    u.uMVP = zRemap * s_proj.cur() * s_mv.cur();
    u.uMV = s_mv.cur();
    u.uTexMat0 = s_tex[0].cur();

    glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(s_mv.cur())));
    u.uNormalRow0 = glm::vec4(nm[0][0], nm[0][1], nm[0][2], 0);
    u.uNormalRow1 = glm::vec4(nm[1][0], nm[1][1], nm[1][2], 0);
    u.uNormalRow2 = glm::vec4(nm[2][0], nm[2][1], nm[2][2], 0);

    u.uBaseColor = s_rs.baseColor;
    u.uFogColor = s_rs.fogColor;
    u.uLMTransform = s_rs.lmt;
    u.uLight0Dir = glm::vec4(s_rs.l0, 0);
    u.uLight1Dir = glm::vec4(s_rs.l1, 0);
    u.uLightDiffuse = glm::vec4(s_rs.ldiff, 0);
    u.uLightAmbient = glm::vec4(s_rs.lamb, 0);
    u.uChunkOffset = glm::vec4(s_rs.chunkOffset, 0);

    float normalSign = glm::determinant(glm::mat3(s_mv.cur())) < 0.f ? -1.f : 1.f;
    u.uScalars = glm::vec4(s_rs.fogStart, s_rs.fogEnd, s_rs.fogDensity, s_rs.alphaRef);
    u.uScalars2 = glm::vec4(1.0f / s_rs.gamma, normalSign, s_rs.globalLM.x, s_rs.globalLM.y);
    u.uFlags = glm::ivec4(s_rs.lighting ? 1 : 0, s_rs.fogMode,
                          s_rs.fogEnable ? 1 : 0, s_rs.useTexture ? 1 : 0);
    int srgb = 0;
#ifdef SRGB_OUTPUT
    srgb = 1;
#endif
    u.uFlags2 = glm::ivec4(s_rs.useLightmap ? 1 : 0, srgb, 0, 0);

    if (s_uniformOffset + UNIFORM_SLOT > UNIFORM_RING_BYTES) s_uniformOffset = 0;
    s_currentUniformOffset = s_uniformOffset;
    id<MTLBuffer> buf = s_uniformBufs[s_frameIndex];
    memcpy((uint8_t*)[buf contents] + s_currentUniformOffset, &u, sizeof(MetalUniforms));
    s_uniformOffset += UNIFORM_SLOT;
}

static MTLBlendFactor mapBlendFactor(int gl) {
    switch (gl) {
        case 0:      return MTLBlendFactorZero;                      // GL_ZERO
        case 1:      return MTLBlendFactorOne;                       // GL_ONE
        case 0x0300: return MTLBlendFactorSourceColor;               // GL_SRC_COLOR
        case 0x0301: return MTLBlendFactorOneMinusSourceColor;       // GL_ONE_MINUS_SRC_COLOR
        case 0x0302: return MTLBlendFactorSourceAlpha;               // GL_SRC_ALPHA
        case 0x0303: return MTLBlendFactorOneMinusSourceAlpha;       // GL_ONE_MINUS_SRC_ALPHA
        case 0x0304: return MTLBlendFactorDestinationAlpha;          // GL_DST_ALPHA
        case 0x0305: return MTLBlendFactorOneMinusDestinationAlpha;  // GL_ONE_MINUS_DST_ALPHA
        case 0x0306: return MTLBlendFactorDestinationColor;          // GL_DST_COLOR
        case 0x0307: return MTLBlendFactorOneMinusDestinationColor;  // GL_ONE_MINUS_DST_COLOR
        case 0x0308: return MTLBlendFactorSourceAlphaSaturated;      // GL_SRC_ALPHA_SATURATE
        default:     return MTLBlendFactorOne;
    }
}

static id<MTLRenderPipelineState> getBlendPipeline(int glSrc, int glDst) {
    if (glSrc == 0x0302 && glDst == 0x0303) return s_pipelineBlend;
    uint64_t key = ((uint64_t)(uint32_t)glSrc << 32) | (uint32_t)glDst;
    auto it = s_pipelineBlendCache.find(key);
    if (it != s_pipelineBlendCache.end()) return it->second;

    MTLRenderPipelineDescriptor* pd = [[MTLRenderPipelineDescriptor alloc] init];
    pd.vertexFunction = s_vertFn;
    pd.fragmentFunction = s_fragFn;
    pd.vertexDescriptor = s_vertDesc;
    pd.colorAttachments[0].pixelFormat = s_metalLayer.pixelFormat;
    pd.colorAttachments[0].blendingEnabled = YES;
    pd.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pd.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    MTLBlendFactor srcBF = mapBlendFactor(glSrc);
    MTLBlendFactor dstBF = mapBlendFactor(glDst);
    pd.colorAttachments[0].sourceRGBBlendFactor = srcBF;
    pd.colorAttachments[0].destinationRGBBlendFactor = dstBF;
    pd.colorAttachments[0].sourceAlphaBlendFactor = srcBF;
    pd.colorAttachments[0].destinationAlphaBlendFactor = dstBF;
    pd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    pd.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    NSError* err = nil;
    id<MTLRenderPipelineState> pso = [s_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!pso) {
        fprintf(stderr, "[Metal] Pipeline (blend src=0x%x dst=0x%x): %s\n",
                glSrc, glDst, [[err localizedDescription] UTF8String]);
        return s_pipelineBlend;
    }
    s_pipelineBlendCache[key] = pso;
    return pso;
}

static id<MTLRenderPipelineState> currentPipeline() {
    if (!s_rs.blendEnable) return s_pipelineOpaque;
    return getBlendPipeline(s_rs.blendSrc, s_rs.blendDst);
}

static void ensureEncoder() {
    if (s_encoder) return;
    if (!s_metalLayer) {
        fprintf(stderr, "[Metal] ensureEncoder: s_metalLayer is nil!\n");
        return;
    }
    if (!s_drawable) {
        s_drawable = [s_metalLayer nextDrawable];
        if (!s_drawable) {
            fprintf(stderr, "[Metal] ensureEncoder: nextDrawable returned nil\n");
            return;
        }
    }
    if (!s_cmdQueue) {
        fprintf(stderr, "[Metal] ensureEncoder: s_cmdQueue is nil!\n");
        return;
    }
    if (!s_cmdBuf) s_cmdBuf = [s_cmdQueue commandBuffer];
    if (!s_cmdBuf) {
        fprintf(stderr, "[Metal] ensureEncoder: commandBuffer returned nil\n");
        return;
    }
    if (!s_depthTex) {
        fprintf(stderr, "[Metal] ensureEncoder: s_depthTex is nil!\n");
        return;
    }

    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor new];
    rpd.colorAttachments[0].texture = s_drawable.texture;
    if (s_needsFirstEncoderClear) {
        rpd.colorAttachments[0].loadAction = MTLLoadActionClear;
        rpd.colorAttachments[0].clearColor = MTLClearColorMake(
            s_rs.clearColor[0], s_rs.clearColor[1], s_rs.clearColor[2], s_rs.clearColor[3]);
        s_needsFirstEncoderClear = false;
    } else {
        rpd.colorAttachments[0].loadAction = MTLLoadActionLoad;
    }
    rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
    rpd.depthAttachment.texture = s_depthTex;
    rpd.depthAttachment.loadAction = MTLLoadActionLoad;
    rpd.depthAttachment.storeAction = MTLStoreActionStore;
    rpd.stencilAttachment.texture = s_stencilTex;
    rpd.stencilAttachment.loadAction = MTLLoadActionLoad;
    rpd.stencilAttachment.storeAction = MTLStoreActionStore;

    s_encoder = [s_cmdBuf renderCommandEncoderWithDescriptor:rpd];
    if (!s_encoder) {
        fprintf(stderr, "[Metal] ensureEncoder: renderCommandEncoder returned nil\n");
        return;
    }

    [s_encoder setRenderPipelineState:currentPipeline()];
    [s_encoder setDepthStencilState:s_rs.depthEnable
                                        ? (s_rs.depthWrite ? s_dsStateDefault : s_dsStateNoWrite)
                                        : s_dsStateDisabled];
    [s_encoder setCullMode:s_rs.cullEnable ? MTLCullModeBack : MTLCullModeNone];
    [s_encoder
        setFrontFacingWinding:s_rs.frontFaceCW ? MTLWindingClockwise : MTLWindingCounterClockwise];
    [s_encoder setFragmentSamplerState:s_sampNearest atIndex:0];
    [s_encoder setFragmentSamplerState:s_sampLinear atIndex:1];
}

static void bindCurrentTextures() {
    if (!s_encoder) return;
    auto it0 = s_textures.find(s_boundTex0);
    id<MTLTexture> t0 = (it0 != s_textures.end() && it0->second.tex) ? it0->second.tex : s_whiteTex;
    auto it1 = s_textures.find(s_boundTex1);
    id<MTLTexture> t1 = (it1 != s_textures.end() && it1->second.tex) ? it1->second.tex : s_whiteTex;
    [s_encoder setFragmentTexture:t0 atIndex:0];
    [s_encoder setFragmentTexture:t1 atIndex:1];
}

void C4JRender::Initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[Metal] SDL_Init: %s\n", SDL_GetError());
        return;
    }
    SDL_DisplayMode dm;
    if (s_reqWidth > 0 && s_reqHeight > 0) {
        s_windowWidth = s_reqWidth;
        s_windowHeight = s_reqHeight;
    } else if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
        s_windowWidth = (int)(dm.w * 0.4f);
        s_windowHeight = (int)(dm.h * 0.4f);
    }

    Uint32 wf = SDL_WINDOW_METAL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    if (s_fullscreen) wf |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    s_window = SDL_CreateWindow("Minecraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                s_windowWidth, s_windowHeight, wf);
    if (!s_window) {
        fprintf(stderr, "[Metal] Window: %s\n", SDL_GetError());
        return;
    }

    s_metalView = SDL_Metal_CreateView(s_window);
    s_metalLayer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(s_metalView);
    s_device = s_metalLayer.device;
    if (!s_device) s_device = MTLCreateSystemDefaultDevice();
    s_metalLayer.device = s_device;
    s_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
#ifdef SRGB_OUTPUT
    s_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
#endif
    s_metalLayer.framebufferOnly = YES;
    s_metalLayer.drawableSize = CGSizeMake(s_windowWidth, s_windowHeight);

    s_cmdQueue = [s_device newCommandQueue];

    NSError* err = nil;
    NSString* src = [NSString stringWithUTF8String:MSL_SRC];
    MTLCompileOptions* opts = [[MTLCompileOptions alloc] init];
    opts.languageVersion = MTLLanguageVersion2_4;
    s_library = [s_device newLibraryWithSource:src options:opts error:&err];
    if (!s_library) {
        fprintf(stderr, "[Metal] Shader compile error: %s\n",
                [[err localizedDescription] UTF8String]);
        return;
    }
    s_vertFn = [s_library newFunctionWithName:@"vertexMain"];
    s_fragFn = [s_library newFunctionWithName:@"fragmentMain"];

    MTLVertexDescriptor* vd = [[MTLVertexDescriptor alloc] init];
    vd.attributes[0].format = MTLVertexFormatFloat3;
    vd.attributes[0].offset = 0;
    vd.attributes[0].bufferIndex = 0;
    vd.attributes[1].format = MTLVertexFormatFloat2;
    vd.attributes[1].offset = 12;
    vd.attributes[1].bufferIndex = 0;
    vd.attributes[2].format = MTLVertexFormatUChar4;
    vd.attributes[2].offset = 20;
    vd.attributes[2].bufferIndex = 0;
    vd.attributes[3].format = MTLVertexFormatChar3;
    vd.attributes[3].offset = 24;
    vd.attributes[3].bufferIndex = 0;
    vd.attributes[4].format = MTLVertexFormatShort2;
    vd.attributes[4].offset = 28;
    vd.attributes[4].bufferIndex = 0;
    vd.layouts[0].stride = 32;
    vd.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    s_vertDesc = vd;

    MTLRenderPipelineDescriptor* pd = [[MTLRenderPipelineDescriptor alloc] init];
    pd.vertexFunction = s_vertFn;
    pd.fragmentFunction = s_fragFn;
    pd.vertexDescriptor = vd;
    pd.colorAttachments[0].pixelFormat = s_metalLayer.pixelFormat;
    pd.colorAttachments[0].blendingEnabled = NO;
    pd.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    pd.stencilAttachmentPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    s_pipelineOpaque = [s_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!s_pipelineOpaque)
        fprintf(stderr, "[Metal] Pipeline (opaque): %s\n", [[err localizedDescription] UTF8String]);

    pd.colorAttachments[0].blendingEnabled = YES;
    pd.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pd.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    pd.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pd.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pd.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
    pd.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    s_pipelineBlend = [s_device newRenderPipelineStateWithDescriptor:pd error:&err];
    if (!s_pipelineBlend)
        fprintf(stderr, "[Metal] Pipeline (blend): %s\n", [[err localizedDescription] UTF8String]);

    MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
    dsd.depthCompareFunction = MTLCompareFunctionLessEqual;
    dsd.depthWriteEnabled = YES;
    s_dsStateDefault = [s_device newDepthStencilStateWithDescriptor:dsd];

    dsd.depthWriteEnabled = NO;
    s_dsStateNoWrite = [s_device newDepthStencilStateWithDescriptor:dsd];

    dsd.depthCompareFunction = MTLCompareFunctionAlways;
    dsd.depthWriteEnabled = NO;
    s_dsStateDisabled = [s_device newDepthStencilStateWithDescriptor:dsd];

    MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
    sd.minFilter = MTLSamplerMinMagFilterNearest;
    sd.magFilter = MTLSamplerMinMagFilterNearest;
    sd.sAddressMode = MTLSamplerAddressModeRepeat;
    sd.tAddressMode = MTLSamplerAddressModeRepeat;
    s_sampNearest = [s_device newSamplerStateWithDescriptor:sd];

    sd.minFilter = MTLSamplerMinMagFilterLinear;
    sd.magFilter = MTLSamplerMinMagFilterLinear;
    sd.sAddressMode = MTLSamplerAddressModeClampToEdge;
    sd.tAddressMode = MTLSamplerAddressModeClampToEdge;
    s_sampLinear = [s_device newSamplerStateWithDescriptor:sd];

    s_frameSema = dispatch_semaphore_create(FRAMES_IN_FLIGHT);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        s_uniformBufs[i] = [s_device newBufferWithLength:UNIFORM_RING_BYTES
                                                 options:MTLResourceStorageModeShared |
                                                         MTLResourceCPUCacheModeWriteCombined];
        s_streamBufs[i] = [s_device newBufferWithLength:STREAM_VBO_SIZE
                                                options:MTLResourceStorageModeShared |
                                                        MTLResourceCPUCacheModeWriteCombined];
    }

    createDepthStencilTextures(s_windowWidth, s_windowHeight);
    s_whiteTex = createWhiteTexture();

    fprintf(stderr, "[Metal] Initialised: %s — %dx%d\n", [[s_device name] UTF8String],
            s_windowWidth, s_windowHeight);
}

void C4JRender::InitialiseContext() {
}

void C4JRender::StartFrame() {
    if (s_window) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                s_shouldClose = true;
            else if (ev.type == SDL_WINDOWEVENT) {
                if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
                    s_shouldClose = true;
                else if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                    s_windowWidth = ev.window.data1 > 0 ? ev.window.data1 : 1;
                    s_windowHeight = ev.window.data2 > 0 ? ev.window.data2 : 1;
                    s_metalLayer.drawableSize = CGSizeMake(s_windowWidth, s_windowHeight);
                    createDepthStencilTextures(s_windowWidth, s_windowHeight);
                }
            }
        }
    }
    dispatch_semaphore_wait(s_frameSema, DISPATCH_TIME_FOREVER);
    s_streamOffset = 0;
    s_uniformOffset = 0;
    s_drawable = nil;
    s_encoder = nil;
    s_cmdBuf = nil;
    s_uniformDirty = true;
    s_needsFirstEncoderClear = true;
}

void C4JRender::Present() {
    if (!s_window) return;
    if (s_encoder) {
        [s_encoder endEncoding];
        s_encoder = nil;
    }
    if (s_drawable && s_cmdBuf) {
        [s_cmdBuf presentDrawable:s_drawable];
        __block dispatch_semaphore_t sema = s_frameSema;
        [s_cmdBuf addCompletedHandler:^(id<MTLCommandBuffer>) {
          dispatch_semaphore_signal(sema);
        }];
        [s_cmdBuf commit];
    } else {
        dispatch_semaphore_signal(s_frameSema);
    }
    s_cmdBuf = nil;
    s_drawable = nil;
    s_frameIndex = (s_frameIndex + 1) % FRAMES_IN_FLIGHT;
}

void C4JRender::Clear(int flags) {
    if (s_encoder) {
        [s_encoder endEncoding];
        s_encoder = nil;
    }
    if (!s_drawable) {
        s_drawable = [s_metalLayer nextDrawable];
        if (!s_drawable) return;
    }
    if (!s_cmdBuf) s_cmdBuf = [s_cmdQueue commandBuffer];

    MTLRenderPassDescriptor* rpd = [MTLRenderPassDescriptor new];
    rpd.colorAttachments[0].texture = s_drawable.texture;
    bool clearColor = (flags & 0x4000) || s_needsFirstEncoderClear;
    rpd.colorAttachments[0].loadAction = clearColor ? MTLLoadActionClear : MTLLoadActionLoad;
    if (clearColor) s_needsFirstEncoderClear = false;
    rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
    rpd.colorAttachments[0].clearColor = MTLClearColorMake(s_rs.clearColor[0], s_rs.clearColor[1],
                                                           s_rs.clearColor[2], s_rs.clearColor[3]);
    rpd.depthAttachment.texture = s_depthTex;
    rpd.depthAttachment.loadAction = (flags & 0x100) ? MTLLoadActionClear : MTLLoadActionLoad;
    rpd.depthAttachment.storeAction = MTLStoreActionStore;
    rpd.depthAttachment.clearDepth = 1.0;
    rpd.stencilAttachment.texture = s_stencilTex;
    rpd.stencilAttachment.loadAction = MTLLoadActionLoad;
    rpd.stencilAttachment.storeAction = MTLStoreActionStore;

    s_encoder = [s_cmdBuf renderCommandEncoderWithDescriptor:rpd];
    [s_encoder setRenderPipelineState:currentPipeline()];
    [s_encoder setDepthStencilState:s_rs.depthEnable
                                        ? (s_rs.depthWrite ? s_dsStateDefault : s_dsStateNoWrite)
                                        : s_dsStateDisabled];
    [s_encoder setCullMode:s_rs.cullEnable ? MTLCullModeBack : MTLCullModeNone];
    [s_encoder
        setFrontFacingWinding:s_rs.frontFaceCW ? MTLWindingClockwise : MTLWindingCounterClockwise];
    [s_encoder setFragmentSamplerState:s_sampNearest atIndex:0];
    [s_encoder setFragmentSamplerState:s_sampLinear atIndex:1];
}

void C4JRender::SetClearColour(const float c[4]) {
    s_rs.clearColor[0] = c[0];
    s_rs.clearColor[1] = c[1];
    s_rs.clearColor[2] = c[2];
    s_rs.clearColor[3] = c[3];
}

void C4JRender::DrawVertices(ePrimitiveType ptype, int count, void* dataIn, eVertexType vType,
                             ePixelShaderType) {
    if (count <= 0 || !dataIn) return;

    bool wasQuad = isQuadPrim((int)ptype);
    static thread_local std::vector<uint8_t> stdData;
    static thread_local std::vector<uint8_t> triData;
    stdData.clear();
    triData.clear();

    if (vType == VERTEX_TYPE_COMPRESSED) {
        stdData.resize((size_t)count * 32);
        const int16_t* src = (const int16_t*)dataIn;
        uint8_t* dst = stdData.data();
        for (int i = 0; i < count; i++) {
            float* dstF = (float*)dst;
            dstF[0] = src[0] / 1024.0f;
            dstF[1] = src[1] / 1024.0f;
            dstF[2] = src[2] / 1024.0f;
            dstF[3] = src[4] / 8192.0f;
            dstF[4] = src[5] / 8192.0f;
            uint16_t packed = (uint16_t)((int)src[3] + 32768);
            dst[20] = 255;
            dst[21] = (uint8_t)((packed & 0x1F) * 255 / 31);
            dst[22] = (uint8_t)(((packed >> 5) & 0x3F) * 255 / 63);
            dst[23] = (uint8_t)(((packed >> 11) & 0x1F) * 255 / 31);
            dst[24] = 0;
            dst[25] = 127;
            dst[26] = 0;
            dst[27] = 0;
            int16_t* dstS = (int16_t*)(dst + 28);
            dstS[0] = src[6];
            dstS[1] = src[7];
            src += 8;
            dst += 32;
        }
        dataIn = stdData.data();
    }

    static const size_t stride = 32;

    if (wasQuad && s_recListId >= 0) {
        int numQuads = count / 4;
        int triVerts = numQuads * 6;
        triData.resize((size_t)triVerts * stride);
        const uint8_t* src = (const uint8_t*)dataIn;
        uint8_t* dst = triData.data();
        for (int q = 0; q < numQuads; q++) {
            const uint8_t* v0 = src + (q * 4 + 0) * stride;
            const uint8_t* v1 = src + (q * 4 + 1) * stride;
            const uint8_t* v2 = src + (q * 4 + 2) * stride;
            const uint8_t* v3 = src + (q * 4 + 3) * stride;
            memcpy(dst + 0 * stride, v0, stride);
            memcpy(dst + 1 * stride, v1, stride);
            memcpy(dst + 2 * stride, v2, stride);
            memcpy(dst + 3 * stride, v0, stride);
            memcpy(dst + 4 * stride, v2, stride);
            memcpy(dst + 5 * stride, v3, stride);
            dst += 6 * stride;
        }
        dataIn = triData.data();
        count = triVerts;
        wasQuad = false;
    }

    size_t bytes = (size_t)count * stride;

    if (s_recListId >= 0) {
        int first = (int)(s_recVerts.size() / stride);
        s_recVerts.insert(s_recVerts.end(), (const uint8_t*)dataIn, (const uint8_t*)dataIn + bytes);
        s_recDraws.push_back({(int)MTLPrimitiveTypeTriangle, first, count});
        return;
    }

    ensureEncoder();
    if (!s_encoder) return;
    flushUniforms();

    [s_encoder setVertexBuffer:s_uniformBufs[s_frameIndex] offset:s_currentUniformOffset atIndex:1];
    [s_encoder setFragmentBuffer:s_uniformBufs[s_frameIndex] offset:s_currentUniformOffset atIndex:1];
    bindCurrentTextures();

    if (wasQuad) {
        int numQuads = count / 4;
        if (s_streamOffset + (int)bytes > STREAM_VBO_SIZE) s_streamOffset = 0;
        memcpy((uint8_t*)[s_streamBufs[s_frameIndex] contents] + s_streamOffset, dataIn, bytes);
        ensureQuadIBO(numQuads);
        [s_encoder setVertexBuffer:s_streamBufs[s_frameIndex] offset:s_streamOffset atIndex:0];
        [s_encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                              indexCount:numQuads * 6
                               indexType:MTLIndexTypeUInt32
                             indexBuffer:s_quadIBO
                       indexBufferOffset:0];
        s_streamOffset += (int)bytes;
    } else {
        if (s_streamOffset + (int)bytes > STREAM_VBO_SIZE) s_streamOffset = 0;
        memcpy((uint8_t*)[s_streamBufs[s_frameIndex] contents] + s_streamOffset, dataIn, bytes);
        [s_encoder setVertexBuffer:s_streamBufs[s_frameIndex] offset:s_streamOffset atIndex:0];
        [s_encoder drawPrimitives:mapPrim((int)ptype) vertexStart:0 vertexCount:count];
        s_streamOffset += (int)bytes;
    }
    s_streamOffset = (s_streamOffset + 255) & ~255;
}

void C4JRender::ReadPixels(int x, int y, int w, int h, void* buf) {
    if (!buf || !s_drawable) return;
}

int C4JRender::CBuffCreate(int count) {
    std::lock_guard lk(s_chunkMtx);
    int b = s_nextListBase;
    s_nextListBase += count;
    return b;
}
void C4JRender::CBuffDelete(int first, int count) {
    std::lock_guard lk(s_chunkMtx);
    for (int i = first; i < first + count; i++) {
        auto it = s_chunkPool.find(i);
        if (it != s_chunkPool.end()) {
            it->second.destroy();
            s_chunkPool.erase(it);
        }
    }
}
void C4JRender::CBuffDeleteAll() {
    std::lock_guard lk(s_chunkMtx);
    for (auto& kv : s_chunkPool) kv.second.destroy();
    s_chunkPool.clear();
    s_nextListBase = 1;
}
void C4JRender::CBuffStart(int index, bool) {
    s_recListId = index;
    s_recVerts.clear();
    s_recDraws.clear();
}
void C4JRender::CBuffEnd() {
    if (s_recListId < 0) return;
    std::lock_guard lk(s_chunkMtx);
    ChunkBuffer& cb = s_chunkPool[s_recListId];
    cb.destroy();
    if (s_recVerts.empty()) {
        s_chunkPool.erase(s_recListId);
        s_recListId = -1;
        return;
    }
    cb.rawVerts = std::move(s_recVerts);
    cb.draws = std::move(s_recDraws);
    cb.valid = true;
    cb.uploaded = false;
    s_recListId = -1;
}
void C4JRender::CBuffClear(int index) {
    std::lock_guard lk(s_chunkMtx);
    auto it = s_chunkPool.find(index);
    if (it != s_chunkPool.end()) {
        it->second.destroy();
        s_chunkPool.erase(it);
    }
}
bool C4JRender::CBuffCall(int index, bool) {
    std::lock_guard lk(s_chunkMtx);
    auto it = s_chunkPool.find(index);
    if (it == s_chunkPool.end() || !it->second.valid) return false;
    ChunkBuffer& cb = it->second;

    if (!cb.uploaded) {
        if (cb.rawVerts.empty()) return false;
        cb.vbo = [s_device newBufferWithBytes:cb.rawVerts.data()
                                       length:cb.rawVerts.size()
                                      options:MTLResourceStorageModeShared];
        cb.rawVerts.clear();
        cb.rawVerts.shrink_to_fit();
        cb.uploaded = true;
    }

    ensureEncoder();
    if (!s_encoder) return false;
    flushUniforms();
    [s_encoder setVertexBuffer:s_uniformBufs[s_frameIndex] offset:s_currentUniformOffset atIndex:1];
    [s_encoder setFragmentBuffer:s_uniformBufs[s_frameIndex] offset:s_currentUniformOffset atIndex:1];
    bindCurrentTextures();
    [s_encoder setVertexBuffer:cb.vbo offset:0 atIndex:0];
    for (const auto& dc : cb.draws)
        [s_encoder drawPrimitives:(MTLPrimitiveType)dc.prim
                      vertexStart:dc.first
                      vertexCount:dc.count];
    return true;
}

void C4JRender::MatrixMode(int t) { s_matMode = (t == 0x1701) ? 1 : (t == 0x1702) ? 2 : 0; }
void C4JRender::MatrixSetIdentity() {
    activeStack().load(glm::mat4(1.f));
    markDirty();
}
void C4JRender::MatrixPush() {
    activeStack().push();
    markDirty();
}
void C4JRender::MatrixPop() {
    activeStack().pop();
    markDirty();
}
void C4JRender::MatrixTranslate(float x, float y, float z) {
    activeStack().mul(glm::translate(glm::mat4(1.f), {x, y, z}));
    markDirty();
}
void C4JRender::MatrixRotate(float a, float x, float y, float z) {
    activeStack().mul(glm::rotate(glm::mat4(1.f), a, {x, y, z}));
    markDirty();
}
void C4JRender::MatrixScale(float x, float y, float z) {
    activeStack().mul(glm::scale(glm::mat4(1.f), {x, y, z}));
    markDirty();
}
void C4JRender::MatrixPerspective(float fovy, float asp, float zn, float zf) {
    s_proj.cur() = glm::perspective(glm::radians(fovy), asp, zn, zf);
    markDirty();
}
void C4JRender::MatrixOrthogonal(float l, float r, float b, float t, float zn, float zf) {
    s_proj.cur() = glm::ortho(l, r, b, t, zn, zf);
    markDirty();
}
void C4JRender::MatrixMult(float* m) {
    activeStack().mul(glm::make_mat4(m));
    markDirty();
}
const float* C4JRender::MatrixGet(int t) {
    thread_local static float buf[16];
    glm::mat4* m = (t == 0x0BA6) ? &s_mv.cur() : (t == 0x0BA7) ? &s_proj.cur() : nullptr;
    if (m) memcpy(buf, glm::value_ptr(*m), 64);
    return buf;
}
void C4JRender::Set_matrixDirty() { s_uniformDirty = true; }

void C4JRender::SetWindowSize(int w, int h) {
    s_reqWidth = w;
    s_reqHeight = h;
}
void C4JRender::SetFullscreen(bool fs) { s_fullscreen = fs; }
bool C4JRender::ShouldClose() { return !s_window || s_shouldClose; }
void C4JRender::GetFramebufferSize(int& w, int& h) {
    w = s_windowWidth;
    h = s_windowHeight;
}
void C4JRender::Close() { s_shouldClose = true; }
bool C4JRender::IsWidescreen() { return true; }
bool C4JRender::IsHiDef() { return true; }
void C4JRender::UpdateGamma(unsigned short g) {
    constexpr unsigned short GAMMA_MAX = 32768;
    s_rs.gamma = 0.5f + ((float)g * (1.0f / GAMMA_MAX));
}

void C4JRender::Shutdown() {
    {
        std::lock_guard lk(s_chunkMtx);
        for (auto& kv : s_chunkPool) kv.second.destroy();
        s_chunkPool.clear();
    }
    s_textures.clear();
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        s_uniformBufs[i] = nil;
        s_streamBufs[i] = nil;
    }
    s_quadIBO = nil;
    s_whiteTex = nil;
    s_depthTex = nil;
    s_stencilTex = nil;
    s_pipelineOpaque = nil;
    s_pipelineBlend = nil;
    s_pipelineBlendCache.clear();
    s_vertFn = nil;
    s_fragFn = nil;
    s_vertDesc = nil;
    s_dsStateDefault = nil;
    s_dsStateNoWrite = nil;
    s_dsStateDisabled = nil;
    s_sampNearest = nil;
    s_sampLinear = nil;
    s_library = nil;
    s_cmdQueue = nil;
    s_device = nil;
    if (s_metalView) {
        SDL_Metal_DestroyView(s_metalView);
        s_metalView = nullptr;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = nullptr;
    }
    SDL_Quit();
}

void C4JRender::StateSetColour(float r, float g, float b, float a) {
    s_rs.baseColor = {r, g, b, a};
    markDirty();
}
void C4JRender::SetChunkOffset(float x, float y, float z) {
    s_rs.chunkOffset = {x, y, z};
    markDirty();
}
void C4JRender::StateSetDepthMask(bool e) {
    s_rs.depthWrite = e;
    if (s_encoder)
        [s_encoder setDepthStencilState:s_rs.depthEnable ? (e ? s_dsStateDefault : s_dsStateNoWrite)
                                                         : s_dsStateDisabled];
}
void C4JRender::StateSetBlendEnable(bool e) {
    s_rs.blendEnable = e;
    if (s_encoder) [s_encoder setRenderPipelineState:currentPipeline()];
}
void C4JRender::StateSetBlendFunc(int src, int dst) {
    if (s_rs.blendSrc == src && s_rs.blendDst == dst) return;
    s_rs.blendSrc = src;
    s_rs.blendDst = dst;
    if (s_encoder && s_rs.blendEnable)
        [s_encoder setRenderPipelineState:currentPipeline()];
}
void C4JRender::StateSetDepthFunc(int) {}
void C4JRender::StateSetFaceCull(bool e) {
    s_rs.cullEnable = e;
    if (s_encoder) [s_encoder setCullMode:e ? MTLCullModeBack : MTLCullModeNone];
}
void C4JRender::StateSetFaceCullCW(bool e) {
    s_rs.frontFaceCW = e;
    if (s_encoder)
        [s_encoder setFrontFacingWinding:e ? MTLWindingClockwise : MTLWindingCounterClockwise];
}
void C4JRender::StateSetLineWidth(float) {}
void C4JRender::StateSetWriteEnable(bool r, bool g, bool b, bool a) {
    s_rs.colorWrite[0] = r;
    s_rs.colorWrite[1] = g;
    s_rs.colorWrite[2] = b;
    s_rs.colorWrite[3] = a;
}
void C4JRender::StateSetDepthTestEnable(bool e) {
    s_rs.depthEnable = e;
    if (s_encoder)
        [s_encoder setDepthStencilState:e ? (s_rs.depthWrite ? s_dsStateDefault : s_dsStateNoWrite)
                                          : s_dsStateDisabled];
}
void C4JRender::StateSetAlphaTestEnable(bool e) {
    s_rs.alphaRef = e ? 0.1f : 0.f;
    markDirty();
}
void C4JRender::StateSetAlphaFunc(int, float p) {
    s_rs.alphaRef = p;
    markDirty();
}
void C4JRender::StateSetDepthSlopeAndBias(float s, float b) {
    if (s_encoder) [s_encoder setDepthBias:b slopeScale:s clamp:0];
}
void C4JRender::StateSetBlendFactor(unsigned int) {}
void C4JRender::StateSetFogEnable(bool e) {
    s_rs.fogEnable = e;
    markDirty();
}
void C4JRender::StateSetFogMode(int mode) {
    int v = (mode == 0x2601) ? 1 : (mode == 0x0800) ? 2 : (mode == 0x0801) ? 3 : 0;
    s_rs.fogMode = v;
    markDirty();
}
void C4JRender::StateSetFogNearDistance(float d) {
    s_rs.fogStart = d;
    markDirty();
}
void C4JRender::StateSetFogFarDistance(float d) {
    s_rs.fogEnd = d;
    markDirty();
}
void C4JRender::StateSetFogDensity(float d) {
    s_rs.fogDensity = d;
    markDirty();
}
void C4JRender::StateSetFogColour(float r, float g, float b) {
    s_rs.fogColor = {r, g, b, 1};
    markDirty();
}
void C4JRender::StateSetLightingEnable(bool e) {
    s_rs.lighting = e;
    markDirty();
}
void C4JRender::StateSetLightColour(int, float r, float g, float b) {
    s_rs.ldiff = {r, g, b};
    markDirty();
}
void C4JRender::StateSetLightAmbientColour(float r, float g, float b) {
    s_rs.lamb = {r, g, b};
    markDirty();
}
void C4JRender::StateSetLightDirection(int light, float x, float y, float z) {
    glm::vec3 d = glm::normalize(glm::mat3(s_mv.cur()) * glm::vec3(x, y, z));
    if (light == 0)
        s_rs.l0 = d;
    else
        s_rs.l1 = d;
    markDirty();
}
void C4JRender::StateSetViewport(eViewportType) {
    if (s_encoder)
        [s_encoder
            setViewport:(MTLViewport){0, 0, (double)s_windowWidth, (double)s_windowHeight, 0, 1}];
}
void C4JRender::StateSetVertexTextureUV(float u, float v) {
    s_rs.globalLM = {u, v};
    markDirty();
}
void C4JRender::StateSetStencil(int, uint8_t, uint8_t, uint8_t) {
}
void C4JRender::StateSetTextureEnable(bool e) {
    if (s_rs.activeTexture == 0) {
        s_rs.useTexture = e;
        markDirty();
    }
}
void C4JRender::StateSetActiveTexture(int tex) { s_rs.activeTexture = (tex == 0x84C1) ? 1 : 0; }

int C4JRender::TextureCreate() {
    int id = s_nextTexId++;
    s_textures[id] = {};
    return id;
}
void C4JRender::TextureFree(int i) {
    auto it = s_textures.find(i);
    if (it != s_textures.end()) {
        it->second.tex = nil;
        s_textures.erase(it);
    }
}
void C4JRender::TextureBind(int idx) { s_boundTex0 = idx; }
void C4JRender::TextureBindVertex(int idx, bool scaleLight) {
    if (idx < 0) {
        s_rs.useLightmap = false;
        markDirty();
        return;
    }
    s_boundTex1 = idx;
    s_rs.useLightmap = true;
    s_rs.lmt = scaleLight ? glm::vec4{1, 1, 8.f / 256.f, 8.f / 256.f} : glm::vec4{1, 1, 0, 0};
    markDirty();
}
void C4JRender::TextureSetTextureLevels(int l) {
    auto it = s_textures.find(s_boundTex0);
    if (it != s_textures.end()) it->second.maxLevel = l;
}
int C4JRender::TextureGetTextureLevels() { return 1; }
void C4JRender::TextureData(int w, int h, void* d, int lvl, eTextureFormat) {
    auto it = s_textures.find(s_boundTex0);
    if (it == s_textures.end()) return;

    if (lvl == 0 || !it->second.tex) {
        MTLTextureDescriptor* td =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                               width:w
                                                              height:h
                                                           mipmapped:(it->second.maxLevel > 1)];
        td.usage = MTLTextureUsageShaderRead;
        td.storageMode = MTLStorageModeShared;
        it->second.tex = [s_device newTextureWithDescriptor:td];
        it->second.w = w;
        it->second.h = h;
    }
    if (d) {
        [it->second.tex replaceRegion:MTLRegionMake2D(0, 0, w, h)
                          mipmapLevel:lvl
                            withBytes:d
                          bytesPerRow:w * 4];
    }
}
void C4JRender::TextureDataUpdate(int xo, int yo, int w, int h, void* d, int lvl) {
    auto it = s_textures.find(s_boundTex0);
    if (it == s_textures.end() || !it->second.tex || !d) return;
    [it->second.tex replaceRegion:MTLRegionMake2D(xo, yo, w, h)
                      mipmapLevel:lvl
                        withBytes:d
                      bytesPerRow:w * 4];
}
void C4JRender::TextureSetParam(int, int) {
}

static int stbLoadMetal(unsigned char* data, int w, int h, D3DXIMAGE_INFO* info, int** out) {
    int* px = new int[w * h];
    for (int i = 0; i < w * h; i++) {
        unsigned char r = data[i * 4], g = data[i * 4 + 1], b = data[i * 4 + 2],
                      a = data[i * 4 + 3];
        px[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
    if (info) {
        info->Width = w;
        info->Height = h;
    }
    *out = px;
    return 0;
}
int C4JRender::LoadTextureData(const char* fn, D3DXIMAGE_INFO* i, int** o) {
    int w, h, c;
    unsigned char* d = stbi_load(fn, &w, &h, &c, 4);
    if (!d) return -1;
    int hr = stbLoadMetal(d, w, h, i, o);
    stbi_image_free(d);
    return hr;
}
int C4JRender::LoadTextureData(uint8_t* pb, uint32_t nb, D3DXIMAGE_INFO* i, int** o) {
    int w, h, c;
    unsigned char* d = stbi_load_from_memory(pb, (int)nb, &w, &h, &c, 4);
    if (!d) return -1;
    int hr = stbLoadMetal(d, w, h, i, o);
    stbi_image_free(d);
    return hr;
}

int glGenTextures_4J() { return RenderManager.TextureCreate(); }
void glGenTextures_4J(int n, unsigned int* textures) {
    for (int i = 0; i < n; i++) textures[i] = (unsigned int)RenderManager.TextureCreate();
}
void glDeleteTextures_4J(int id) { RenderManager.TextureFree(id); }
void glDeleteTextures_4J(int n, const unsigned int* textures) {
    for (int i = 0; i < n; i++) RenderManager.TextureFree((int)textures[i]);
}

void glBeginQuery_4J_Helper(unsigned int, unsigned int) {}
void glEndQuery_4J_Helper(unsigned int) {}
void glGenQueries_4J_Helper(unsigned int* id) { *id = 0; }
void glGetQueryObjectu_4J_Helper(unsigned int, unsigned int, unsigned int* val) { *val = 0; }

#endif