#pragma once

#include <cstdint>
#include <cstdlib>

class ImageFileBuffer {
public:
    enum EImageType { e_typePNG, e_typeJPG };

    EImageType m_type;
    void* m_pBuffer;
    int m_bufferSize;

    int GetType() { return m_type; }
    void* GetBufferPointer() { return m_pBuffer; }
    int GetBufferSize() { return m_bufferSize; }
    void Release() {
        std::free(m_pBuffer);
        m_pBuffer = nullptr;
    }
    bool Allocated() { return m_pBuffer != nullptr; }
};

typedef struct {
    int Width;
    int Height;
} D3DXIMAGE_INFO;

typedef struct _XSOCIAL_PREVIEWIMAGE {
    std::uint8_t* pBytes;
    std::uint32_t Pitch;
    std::uint32_t Width;
    std::uint32_t Height;
    //    D3DFORMAT Format;
} XSOCIAL_PREVIEWIMAGE, *PXSOCIAL_PREVIEWIMAGE;

class C4JRender {
public:
    void Tick();
    void UpdateGamma(unsigned short usGamma);

    // Matrix stack
    void MatrixMode(int type);
    void MatrixSetIdentity();
    void MatrixTranslate(float x, float y, float z);
    void MatrixRotate(float angle, float x, float y, float z);
    void MatrixScale(float x, float y, float z);
    void MatrixPerspective(float fovy, float aspect, float zNear, float zFar);
    void MatrixOrthogonal(float left, float right, float bottom, float top,
                          float zNear, float zFar);
    void MatrixPop();
    void MatrixPush();
    void MatrixMult(float* mat);
    const float* MatrixGet(int type);
    void Set_matrixDirty();

    // Core
    void Initialise();
    void InitialiseContext();
    // Call before Initialise() to override window size and/or fullscreen mode.
    // If not called, the primary monitor's native resolution is used.
    void SetWindowSize(int w, int h);
    void SetFullscreen(bool fs);
    void StartFrame();
    void DoScreenGrabOnNextPresent();
    void Present();
    void Clear(int flags);
    void SetClearColour(const float colourRGBA[4]);
    bool IsWidescreen();
    bool IsHiDef();
    void GetFramebufferSize(int& width, int& height);
    void CaptureThumbnail(ImageFileBuffer* pngOut);
    void CaptureScreen(ImageFileBuffer* jpgOut,
                       XSOCIAL_PREVIEWIMAGE* previewOut);
    void BeginConditionalSurvey(int identifier);
    void EndConditionalSurvey();
    void BeginConditionalRendering(int identifier);
    void EndConditionalRendering();

    // Vertex data handling
    typedef enum {
        VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,  // Position 3 x float, texture 2 x
                                          // float, colour 4 x byte, normal 4 x
                                          // byte, padding 1 32-bit word
        VERTEX_TYPE_COMPRESSED,  // Compressed format - see comment at top of
                                 // VS_PS3_TS2_CS1.hlsl for description of
                                 // layout
        VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_LIT,  // as
                                              // VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1
                                              // with lighting applied,
        VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN,  // as
                                                 // VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1
                                                 // with tex gen
        VERTEX_TYPE_COUNT
    } eVertexType;

    // Pixel shader
    typedef enum {
        PIXEL_SHADER_TYPE_STANDARD,
        PIXEL_SHADER_TYPE_PROJECTION,
        PIXEL_SHADER_TYPE_FORCELOD,
        PIXEL_SHADER_COUNT
    } ePixelShaderType;

    typedef enum {
        VIEWPORT_TYPE_FULLSCREEN,
        VIEWPORT_TYPE_SPLIT_TOP,
        VIEWPORT_TYPE_SPLIT_BOTTOM,
        VIEWPORT_TYPE_SPLIT_LEFT,
        VIEWPORT_TYPE_SPLIT_RIGHT,
        VIEWPORT_TYPE_QUADRANT_TOP_LEFT,
        VIEWPORT_TYPE_QUADRANT_TOP_RIGHT,
        VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT,
        VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT,
    } eViewportType;

    typedef enum {
        PRIMITIVE_TYPE_TRIANGLE_LIST,
        PRIMITIVE_TYPE_TRIANGLE_STRIP,
        PRIMITIVE_TYPE_TRIANGLE_FAN,
        PRIMITIVE_TYPE_QUAD_LIST,
        PRIMITIVE_TYPE_LINE_LIST,
        PRIMITIVE_TYPE_LINE_STRIP,
        PRIMITIVE_TYPE_COUNT
    } ePrimitiveType;

    void DrawVertices(ePrimitiveType PrimitiveType, int count, void* dataIn,
                      eVertexType vType, C4JRender::ePixelShaderType psType);

    // Command buffers
    void CBuffLockStaticCreations();
    int CBuffCreate(int count);
    void CBuffDelete(int first, int count);
    void CBuffStart(int index, bool full = false);
    void CBuffClear(int index);
    int CBuffSize(int index);
    void CBuffEnd();
    bool CBuffCall(int index, bool full = true);
    void CBuffTick();
    void CBuffDeferredModeStart();
    void CBuffDeferredModeEnd();

    typedef enum {
        TEXTURE_FORMAT_RxGyBzAw,  // Normal 32-bit RGBA texture, 8 bits per
                                  // component
        /* Don't think these are all directly available on D3D 11 - leaving for
        now TEXTURE_FORMAT_R0G0B0Ax,		// One 8-bit component mapped to
        alpha channel, R=G=B=0 TEXTURE_FORMAT_R1G1B1Ax,		// One 8-bit
        component mapped to alpha channel, R=G=B=1 TEXTURE_FORMAT_RxGxBxAx,
        // One 8-bit component mapped to all channels
        */
        MAX_TEXTURE_FORMATS
    } eTextureFormat;

    // Textures
    int TextureCreate();
    void TextureFree(int idx);
    void TextureBind(int idx);
    void TextureBindVertex(int idx, bool scaleLight = false);
    void TextureSetTextureLevels(int levels);
    int TextureGetTextureLevels();
    void TextureData(int width, int height, void* data, int level,
                     eTextureFormat format = TEXTURE_FORMAT_RxGyBzAw);
    void TextureDataUpdate(int xoffset, int yoffset, int width, int height,
                           void* data, int level);
    void TextureSetParam(int param, int value);
    void TextureDynamicUpdateStart();
    void TextureDynamicUpdateEnd();
    int LoadTextureData(const char* szFilename, D3DXIMAGE_INFO* pSrcInfo,
                        int** ppDataOut);
    int LoadTextureData(std::uint8_t* pbData, std::uint32_t byteCount,
                        D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut);
    int SaveTextureData(const char* szFilename, D3DXIMAGE_INFO* pSrcInfo,
                        int* ppDataOut);
    int SaveTextureDataToMemory(void* pOutput, int outputCapacity,
                                int* outputLength, int width, int height,
                                int* ppDataIn);
    void TextureGetStats();
    void* TextureGetTexture(int idx);

    // State control
    void StateSetColour(float r, float g, float b, float a);
    void StateSetDepthMask(bool enable);
    void StateSetBlendEnable(bool enable);
    void StateSetBlendFunc(int src, int dst);
    void StateSetBlendFactor(unsigned int colour);
    void StateSetAlphaFunc(int func, float param);
    void StateSetDepthFunc(int func);
    void StateSetFaceCull(bool enable);
    void StateSetFaceCullCW(bool enable);
    void StateSetLineWidth(float width);
    void StateSetWriteEnable(bool red, bool green, bool blue, bool alpha);
    void StateSetDepthTestEnable(bool enable);
    void StateSetAlphaTestEnable(bool enable);
    void StateSetDepthSlopeAndBias(float slope, float bias);
    void StateSetFogEnable(bool enable);
    void StateSetFogMode(int mode);
    void StateSetFogNearDistance(float dist);
    void StateSetFogFarDistance(float dist);
    void StateSetFogDensity(float density);
    void StateSetFogColour(float red, float green, float blue);
    void StateSetLightingEnable(bool enable);
    void StateSetVertexTextureUV(float u, float v);
    void StateSetLightColour(int light, float red, float green, float blue);
    void StateSetLightAmbientColour(float red, float green, float blue);
    void StateSetLightDirection(int light, float x, float y, float z);
    void StateSetLightEnable(int light, bool enable);
    void StateSetViewport(eViewportType viewportType);
    void StateSetEnableViewportClipPlanes(bool enable);
    void StateSetTexGenCol(int col, float x, float y, float z, float w,
                           bool eyeSpace);
    void StateSetStencil(int Function, std::uint8_t stencil_ref,
                         std::uint8_t stencil_func_mask,
                         std::uint8_t stencil_write_mask);
    void StateSetForceLOD(int LOD);

    // Event tracking
    void BeginEvent(const wchar_t* eventName);
    void EndEvent();

    // PLM event handling
    void Suspend();
    bool Suspended();
    void Resume();

    // Linux window management
    bool ShouldClose();
    void Close();
    void Shutdown();
};

const int GL_MODELVIEW_MATRIX = 0x0BA6;
const int GL_PROJECTION_MATRIX = 0x0BA7;
const int GL_MODELVIEW = 0x1700;
const int GL_PROJECTION = 0x1701;
const int GL_TEXTURE = 0x1702;

// These things required for tex gen

const int GL_S = 0x2000;
const int GL_T = 0x2001;
const int GL_R = 0x2002;
const int GL_Q = 0x2003;

const int GL_TEXTURE_GEN_S = 0x0C60;
const int GL_TEXTURE_GEN_T = 0x0C61;
const int GL_TEXTURE_GEN_Q = 0x0C63;
const int GL_TEXTURE_GEN_R = 0x0C62;

const int GL_TEXTURE_GEN_MODE = 0x2500;
const int GL_OBJECT_LINEAR = 0x2401;
const int GL_EYE_LINEAR = 0x2400;
const int GL_OBJECT_PLANE = 0x2501;
const int GL_EYE_PLANE = 0x2502;

// These things are used by glEnable/glDisable so must be different and non-zero
// (zero is used by things we haven't assigned yet)
const int GL_TEXTURE_2D = 0x0DE1;
const int GL_BLEND = 0x0BE2;
const int GL_CULL_FACE = 0x0B44;
const int GL_ALPHA_TEST = 0x0BC0;
const int GL_DEPTH_TEST = 0x0B71;
const int GL_FOG = 0x0B60;
const int GL_LIGHTING = 0x0B50;
const int GL_LIGHT0 = 0x4000;
const int GL_LIGHT1 = 0x4001;

const int CLEAR_DEPTH_FLAG = 0x00000100;
const int CLEAR_COLOUR_FLAG = 0x00004000;

const int GL_DEPTH_BUFFER_BIT = CLEAR_DEPTH_FLAG;
const int GL_COLOR_BUFFER_BIT = CLEAR_COLOUR_FLAG;

const int GL_SRC_ALPHA = 0x0302;
const int GL_ONE_MINUS_SRC_ALPHA = 0x0303;
const int GL_ONE = 1;
const int GL_ZERO = 0;
const int GL_DST_ALPHA = 0x0304;
const int GL_SRC_COLOR = 0x0300;
const int GL_DST_COLOR = 0x0306;
const int GL_ONE_MINUS_DST_COLOR = 0x0307;
const int GL_ONE_MINUS_SRC_COLOR = 0x0301;
const int GL_CONSTANT_ALPHA = 0x8003;
const int GL_ONE_MINUS_CONSTANT_ALPHA = 0x8004;

const int GL_GREATER = 0x0204;
const int GL_EQUAL = 0x0202;
const int GL_LEQUAL = 0x0203;
const int GL_GEQUAL = 0x0206;
const int GL_ALWAYS = 0x0207;

const int GL_TEXTURE_MIN_FILTER = 0x2801;
const int GL_TEXTURE_MAG_FILTER = 0x2800;
const int GL_TEXTURE_WRAP_S = 0x2802;
const int GL_TEXTURE_WRAP_T = 0x2803;

const int GL_NEAREST = 0x2600;
const int GL_LINEAR = 0x2601;
const int GL_EXP = 0x0800;
const int GL_NEAREST_MIPMAP_LINEAR = 0x2702;  // TODO - mipmapping bit of this

const int GL_CLAMP = 0x2900;
const int GL_REPEAT = 0x2901;

const int GL_FOG_START = 0x0B63;
const int GL_FOG_END = 0x0B64;
const int GL_FOG_MODE = 0x0B65;
const int GL_FOG_DENSITY = 0x0B62;
const int GL_FOG_COLOR = 0x0B66;

const int GL_POSITION = 0x1203;
const int GL_AMBIENT = 0x1200;
const int GL_DIFFUSE = 0x1201;
const int GL_SPECULAR = 0x1202;

const int GL_LIGHT_MODEL_AMBIENT = 0x0B53;

const int GL_LINES = 0x0001;
const int GL_LINE_STRIP = 0x0003;
const int GL_QUADS = 0x0007;
const int GL_TRIANGLE_FAN = 0x0006;
const int GL_TRIANGLE_STRIP = 0x0005;

// Singleton
extern C4JRender RenderManager;
