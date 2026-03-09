
// TODO: ADD BETTER COMMENTS.
#include "4J_Render.h"
#include <cstring>
#include <cstdlib>  // getenv

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

C4JRender RenderManager;

static GLFWwindow *s_window = nullptr;
static int s_textureLevels = 1;
static int s_windowWidth  = 1280;  // updated to actual framebuffer size each frame
static int s_windowHeight = 720;
static int s_reqWidth     = 0;     // 0 = auto-detect from primary monitor
static int s_reqHeight    = 0;
static bool s_fullscreen  = false;

// Thread-local storage for per-thread shared GL contexts.
// The main thread uses s_window directly; worker threads get invisible
// windows that share objects (textures, display lists) with s_window.
static pthread_key_t s_glCtxKey;
static pthread_once_t s_glCtxKeyOnce = PTHREAD_ONCE_INIT;
static void makeGLCtxKey() { pthread_key_create(&s_glCtxKey, nullptr); }

// Pre-created pool of shared contexts for worker threads

// AMD drivers (especially on Linux/Mesa) can be very sensitive to the number of shared contexts
// and concurrent display list compilation. 8 was original, 4 was an attempt to fix it.
// 6 covers the 5 concurrent worker threads (update + 3x rebuild + main thread).
static const int MAX_SHARED_CONTEXTS = 6;
static GLFWwindow *s_sharedContexts[MAX_SHARED_CONTEXTS] = {};
static int s_sharedContextCount = 0;
static int s_nextSharedContext = 0;
static pthread_mutex_t s_sharedCtxMutex = PTHREAD_MUTEX_INITIALIZER;

// Track which thread is the main (rendering) thread
static pthread_t s_mainThread;
static bool s_mainThreadSet = false;

// viewport go brr
static void onFramebufferResize(GLFWwindow * /*win*/, int w, int h)
{
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    s_windowWidth  = w;
    s_windowHeight = h;
    ::glViewport(0, 0, w, h);
}

void C4JRender::Initialise()
{
    if (!glfwInit()) {
        fprintf(stderr, "[4J_Render] Failed to initialise GLFW\n");
        return;
    }
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = primaryMonitor ? glfwGetVideoMode(primaryMonitor) : nullptr;

    if (s_reqWidth > 0 && s_reqHeight > 0) {
        s_windowWidth  = s_reqWidth;
        s_windowHeight = s_reqHeight;
    } else if (mode) {
        s_windowWidth  = mode->width;
        s_windowHeight = mode->height;
    }
    fprintf(stderr, "[4J_Render] Window %dx%d  fullscreen=%s\n",
            s_windowWidth, s_windowHeight, s_fullscreen ? "yes" : "no");
    fflush(stderr);

    // opengl 2.1!!!
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    GLFWmonitor *fsMonitor = s_fullscreen ? primaryMonitor : nullptr;
    s_window = glfwCreateWindow(s_windowWidth, s_windowHeight,
                                "Minecraft Console Edition", fsMonitor, nullptr);
    if (!s_window) {
        fprintf(stderr, "[4J_Render] Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(s_window);
    glfwSwapInterval(0);  // vsync

    // Keep viewport in sync with OS-driven window resizes.
    glfwSetFramebufferSizeCallback(s_window, onFramebufferResize);

    // init opengl
    ::glEnable(GL_TEXTURE_2D);
    ::glEnable(GL_DEPTH_TEST);
    ::glDepthFunc(GL_LEQUAL);
    ::glClearDepth(1.0);
    ::glEnable(GL_ALPHA_TEST);
    ::glAlphaFunc(GL_GREATER, 0.1f);
    ::glEnable(GL_BLEND);
    ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ::glEnable(GL_CULL_FACE);
    ::glCullFace(GL_BACK);
    ::glShadeModel(GL_SMOOTH);
    ::glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    ::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    ::glViewport(0, 0, s_windowWidth, s_windowHeight);
    ::glEnable(GL_COLOR_MATERIAL);
    ::glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    printf("[4J_Render] OpenGL %s  |  %s\n",
           (const char*)::glGetString(GL_VERSION),
           (const char*)::glGetString(GL_RENDERER));
    fflush(stdout);

    // Tag this as the main rendering thread
    pthread_once(&s_glCtxKeyOnce, makeGLCtxKey);
    s_mainThread = pthread_self();
    s_mainThreadSet = true;
    pthread_setspecific(s_glCtxKey, s_window);

    // Pre-create shared GL contexts for worker threads (chunk builders etc.)
    // Must be done on the main thread because GLFW requires it.
    // Ensure they are invisible so they don't interfere with the window manager.
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    for (int i = 0; i < MAX_SHARED_CONTEXTS; i++) {
        s_sharedContexts[i] = glfwCreateWindow(1, 1, "", nullptr, s_window);
        if (s_sharedContexts[i]) {
            s_sharedContextCount++;
        } else {
            fprintf(stderr, "[4J_Render] WARN: only created %d/%d shared contexts\n", i, MAX_SHARED_CONTEXTS);
            break;
        }
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    // Ensure main thread still has the context
    glfwMakeContextCurrent(s_window);
    fprintf(stderr, "[4J_Render] Created %d shared GL contexts for worker threads\n", s_sharedContextCount);
    fflush(stderr);
}

void C4JRender::InitialiseContext()
{
    if (!s_window) return;
    pthread_once(&s_glCtxKeyOnce, makeGLCtxKey);

    // Main thread reclaiming context (e.g. after startup thread finishes)
    if (s_mainThreadSet && pthread_equal(pthread_self(), s_mainThread)) {
        glfwMakeContextCurrent(s_window);
        pthread_setspecific(s_glCtxKey, s_window);
        return;
    }

    // Worker thread: check if it already has a shared context
    GLFWwindow *ctx = (GLFWwindow*)pthread_getspecific(s_glCtxKey);
    if (ctx) {
        glfwMakeContextCurrent(ctx);
        return;
    }

    // Grab a pre-created shared context from the pool
    pthread_mutex_lock(&s_sharedCtxMutex);
    GLFWwindow *shared = nullptr;
    if (s_nextSharedContext < s_sharedContextCount) {
        shared = s_sharedContexts[s_nextSharedContext++];
    }
    pthread_mutex_unlock(&s_sharedCtxMutex);

    if (!shared) {
        fprintf(stderr, "[4J_Render] ERROR: no shared GL contexts left for worker thread %lu!\n", (unsigned long)pthread_self());
        fflush(stderr);
        return;
    }
    glfwMakeContextCurrent(shared);

    // Initialize some basic state for this context to ensure consistent display list recording
    ::glEnable(GL_TEXTURE_2D);
    ::glEnable(GL_DEPTH_TEST);
    ::glDepthFunc(GL_LEQUAL);
    ::glAlphaFunc(GL_GREATER, 0.1f);
    ::glEnable(GL_BLEND);
    ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ::glShadeModel(GL_SMOOTH);
    ::glEnable(GL_COLOR_MATERIAL);
    ::glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    pthread_setspecific(s_glCtxKey, shared);
    fprintf(stderr, "[4J_Render] Assigned shared GL context %p to worker thread %lu\n", (void*)shared, (unsigned long)pthread_self());
    fflush(stderr);
}

void C4JRender::StartFrame()
{
    if (!s_window) return;
    glfwGetFramebufferSize(s_window, &s_windowWidth, &s_windowHeight);
    if (s_windowWidth < 1) s_windowWidth = 1;
    if (s_windowHeight < 1) s_windowHeight = 1;
    ::glViewport(0, 0, s_windowWidth, s_windowHeight);
}

void C4JRender::Present()
{
    if (!s_window) return;
    ::glFlush();
    glfwSwapBuffers(s_window);
    glfwPollEvents();
}

void C4JRender::SetWindowSize(int w, int h)
{
    s_reqWidth  = (w > 0) ? w : 0;
    s_reqHeight = (h > 0) ? h : 0;
}

void C4JRender::SetFullscreen(bool fs)
{
    s_fullscreen = fs;
}

bool C4JRender::ShouldClose()
{
    return !s_window || glfwWindowShouldClose(s_window);
}

void C4JRender::Shutdown()
{
    // Destroy the main window and terminate GLFW cleanly so that
    // destructors running after the game loop don't touch a dead context.
    if (s_window)
    {
        glfwDestroyWindow(s_window);
        s_window = nullptr;
    }
    glfwTerminate();
}

void C4JRender::DoScreenGrabOnNextPresent() {}

void C4JRender::Clear(int flags)
{
    ::glClear(flags);
}

void C4JRender::SetClearColour(const float colourRGBA[4])
{
    ::glClearColor(colourRGBA[0], colourRGBA[1], colourRGBA[2], colourRGBA[3]);
}

bool C4JRender::IsWidescreen() { return true; }
bool C4JRender::IsHiDef()      { return true; }
void C4JRender::GetFramebufferSize(int &width, int &height) { width = s_windowWidth; height = s_windowHeight; }
void C4JRender::CaptureThumbnail(ImageFileBuffer *)  {}
void C4JRender::CaptureScreen(ImageFileBuffer *, XSOCIAL_PREVIEWIMAGE *) {}
void C4JRender::BeginConditionalSurvey(int)   {}
void C4JRender::EndConditionalSurvey()        {}
void C4JRender::BeginConditionalRendering(int){}
void C4JRender::EndConditionalRendering()     {}

void C4JRender::MatrixMode(int type)     { ::glMatrixMode(type); }
void C4JRender::MatrixSetIdentity()      { ::glLoadIdentity(); }
void C4JRender::MatrixTranslate(float x, float y, float z) { ::glTranslatef(x, y, z); }

void C4JRender::MatrixRotate(float angle, float x, float y, float z)
{
    // can't fucking afford pi
    ::glRotatef(angle * (180.0f / 3.14159265358979f), x, y, z);
}

void C4JRender::MatrixScale(float x, float y, float z) { ::glScalef(x, y, z); }

void C4JRender::MatrixPerspective(float fovy, float aspect, float zNear, float zFar)
{
    ::gluPerspective((double)fovy, (double)aspect, (double)zNear, (double)zFar);
}

void C4JRender::MatrixOrthogonal(float left, float right, float bottom, float top,
                                  float zNear, float zFar)
{
    ::glOrtho(left, right, bottom, top, zNear, zFar);
}

void C4JRender::MatrixPop()              { ::glPopMatrix(); }
void C4JRender::MatrixPush()             { ::glPushMatrix(); }
void C4JRender::MatrixMult(float *mat)   { ::glMultMatrixf(mat); }

const float* C4JRender::MatrixGet(int type)
{
    static float mat[16];
    ::glGetFloatv(type, mat);
    return mat;
}

void C4JRender::Set_matrixDirty() {} // immediate-mode

static GLenum mapPrimType(int pt)
{
    // Handle GL constants first
    if (pt == GL_QUADS) return GL_QUADS;
    if (pt == GL_TRIANGLES) return GL_TRIANGLES;
    if (pt == GL_LINES) return GL_LINES;
    if (pt == GL_LINE_STRIP) return GL_LINE_STRIP;
    if (pt == GL_TRIANGLE_STRIP) return GL_TRIANGLE_STRIP;
    if (pt == GL_TRIANGLE_FAN) return GL_TRIANGLE_FAN;

    // Map from ePrimitiveType enum
    switch (pt) {
    case C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST:  return GL_TRIANGLES;
    case C4JRender::PRIMITIVE_TYPE_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case C4JRender::PRIMITIVE_TYPE_TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
    case C4JRender::PRIMITIVE_TYPE_QUAD_LIST:      return GL_QUADS;
    case C4JRender::PRIMITIVE_TYPE_LINE_LIST:      return GL_LINES;
    case C4JRender::PRIMITIVE_TYPE_LINE_STRIP:     return GL_LINE_STRIP;
    default:                                       return GL_TRIANGLES;
    }
}

// clientside awawawa
void C4JRender::DrawVertices(ePrimitiveType PrimitiveType, int count,
                             void *dataIn, eVertexType vType,
                             C4JRender::ePixelShaderType psType)
{
    if (count <= 0 || !dataIn) return;

    GLenum mode = mapPrimType((int)PrimitiveType);

    if (vType == VERTEX_TYPE_COMPRESSED) {
        // NO NEED TO REWRITE IT ALL YAY
        int16_t *sdata = (int16_t *)dataIn;
        ::glBegin(mode);
        for (int i = 0; i < count; i++) {
            int16_t *vert = sdata + i * 8;

            float x = vert[0] / 1024.0f;
            float y = vert[1] / 1024.0f;
            float z = vert[2] / 1024.0f;

            // Unpack RGB565 colour (Tesselator stores as packedcol - 32768 to fit in int16)
            unsigned short packedColor = (unsigned short)((int)vert[3] + 32768);
            float r = ((packedColor >> 11) & 0x1f) / 31.0f;
            float g = ((packedColor >>  5) & 0x3f) / 63.0f;
            float b = ( packedColor        & 0x1f) / 31.0f;

            float fu = vert[4] / 8192.0f;
            float fv = vert[5] / 8192.0f;
            // Strip mipmap-disable flag: Tesselator adds +1.0 (= +8192) to u when mipmaps off
            if (fu >= 1.0f) fu -= 1.0f;

            // Unit 1 (lightmap) UVs
            float fu2 = (float)vert[6] / 256.0f;
            float fv2 = (float)vert[7] / 256.0f;

            ::glColor3f(r, g, b);
            ::glTexCoord2f(fu, fv);
            ::glMultiTexCoord2f(GL_TEXTURE1, fu2, fv2);
            ::glVertex3f(x, y, z);
        }
        ::glEnd();
    } else {
        unsigned int *idata = (unsigned int *)dataIn;
        ::glBegin(mode);
        for (int i = 0; i < count; i++) {
            float *fdata = (float *)(idata + i * 8);

            unsigned int colorInt = idata[i * 8 + 5];
            unsigned char cr = (colorInt >> 24) & 0xFF;
            unsigned char cg = (colorInt >> 16) & 0xFF;
            unsigned char cb = (colorInt >>  8) & 0xFF;
            unsigned char ca =  colorInt         & 0xFF;

            unsigned int normalInt = idata[i * 8 + 6];
            int8_t nx = (int8_t)( normalInt        & 0xFF);
            int8_t ny = (int8_t)((normalInt >>  8) & 0xFF);
            int8_t nz = (int8_t)((normalInt >> 16) & 0xFF);

            unsigned int tex2Int = idata[i * 8 + 7];

            if (nx != 0 || ny != 0 || nz != 0) {
                ::glNormal3f(nx / 127.0f, ny / 127.0f, nz / 127.0f);
            }

            // Only override current GL color when the vertex actually carries one.
            // colorInt == 0 is the Tesselator sentinel for "no colour set"
            if (colorInt != 0) {
                ::glColor4ub(cr, cg, cb, ca);
            }

            ::glTexCoord2f(fdata[3], fdata[4]);

            // Unit 1 (lightmap) UVs - 0xfe00fe00 is sentinel for "no Unit 1 UVs"
            if (tex2Int != 0xfe00fe00) {
                float u2 = (float)(short)(tex2Int & 0xFFFF) / 256.0f;
                float v2 = (float)(short)((tex2Int >> 16) & 0xFFFF) / 256.0f;
                ::glMultiTexCoord2f(GL_TEXTURE1, u2, v2);
            }

            ::glVertex3f(fdata[0], fdata[1], fdata[2]);
        }
        ::glEnd();
    }
    ::glFlush();
}


void C4JRender::CBuffLockStaticCreations() {}

int C4JRender::CBuffCreate(int count)
{
    int id = (int)::glGenLists(count);
    ::glFlush();
    return id;
}

void C4JRender::CBuffDelete(int first, int count)
{
    if (first > 0 && count > 0) {
        ::glDeleteLists(first, count);
        ::glFlush();
    }
}

void C4JRender::CBuffStart(int index, bool /*full*/)
{
    if (index > 0) {
        ::glNewList(index, GL_COMPILE);
        ::glFlush();
    }
}

void C4JRender::CBuffClear(int index)
{
    if (index > 0) { 
        ::glNewList(index, GL_COMPILE); 
        ::glEndList(); 
        ::glFlush();
    }
}

int  C4JRender::CBuffSize(int /*index*/) { return 0; }

void C4JRender::CBuffEnd()
{
    ::glEndList();
    ::glFlush();
}

bool C4JRender::CBuffCall(int index, bool /*full*/)
{
    if (index <= 0) return false;
    if (::glIsList(index)) { ::glCallList(index); return true; }
    return false;
}

void C4JRender::CBuffTick()              {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd()   {}


int C4JRender::TextureCreate()
{
    GLuint id = 0;
    ::glGenTextures(1, &id);
    return (int)id;
}

void C4JRender::TextureFree(int idx)
{
    GLuint id = (GLuint)idx;
    ::glDeleteTextures(1, &id);
}

void C4JRender::TextureBind(int idx)
{
    if (idx < 0) {
        ::glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        ::glBindTexture(GL_TEXTURE_2D, (GLuint)idx);
    }
}

void C4JRender::TextureBindVertex(int idx)
{
    // Unit 1 used for lightmapping in fixed-function or standard shaders
    ::glActiveTexture(GL_TEXTURE1);
    if (idx < 0) {
        ::glBindTexture(GL_TEXTURE_2D, 0);
        ::glDisable(GL_TEXTURE_2D);
    } else {
        ::glEnable(GL_TEXTURE_2D);
        ::glBindTexture(GL_TEXTURE_2D, (GLuint)idx);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    ::glActiveTexture(GL_TEXTURE0);
    ::glFlush();
}

void C4JRender::TextureSetTextureLevels(int levels)
{
    // Set GL_TEXTURE_MAX_LEVEL so OpenGL knows how many mip levels this texture has.
    // Without this, the default is 1000, and any texture that doesn't upload all 1000
    // levels is considered "incomplete" and renders as white.
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, levels > 0 ? levels - 1 : 0);
    s_textureLevels = levels;
}
int  C4JRender::TextureGetTextureLevels()            { return s_textureLevels; }

void C4JRender::TextureData(int width, int height, void *data, int level,
                            eTextureFormat /*format*/)
{
    // Game produces [r,g,b,a] bytes via the non-Xbox transferFromImage/loadTexture paths.
    // Use GL_RGBA so OpenGL interprets them correctly. GL_BGRA would swap R and B channels.
    ::glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA,
                   width, height, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, data);

    ::glFlush();

    if (level == 0) {
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

void C4JRender::TextureDataUpdate(int xoffset, int yoffset,
                                  int width, int height,
                                  void *data, int level)
{
    ::glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset,
                      width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    ::glFlush();
}

void C4JRender::TextureSetParam(int param, int value)
{
    ::glTexParameteri(GL_TEXTURE_2D, param, value);
}

void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd()   {}

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short) {}

// This sucks, but at least better than libpng
static HRESULT LoadFromSTB(unsigned char* data, int width, int height, D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut)
{
    int pixelCount = width * height;
    int* pixels = new int[pixelCount];

    for (int i = 0; i < pixelCount; i++)
    {
        unsigned char r = data[i * 4 + 0];
        unsigned char g = data[i * 4 + 1];
        unsigned char b = data[i * 4 + 2];
        unsigned char a = data[i * 4 + 3];

        //pixels[i] = (a << 24) | (b << 16) | (g << 8) | r;
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    if (pSrcInfo)
    {
        pSrcInfo->Width = width;
        pSrcInfo->Height = height;
    }

    *ppDataOut = pixels;
    return S_OK;
}

HRESULT C4JRender::LoadTextureData(const char* szFilename, D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut)
{
    int width, height, channels;

    unsigned char* data = stbi_load(szFilename, &width, &height, &channels, 4);
    if (!data)
        return E_FAIL;

    HRESULT hr = LoadFromSTB(data, width, height, pSrcInfo, ppDataOut);

    stbi_image_free(data);
    return hr;
}

HRESULT C4JRender::LoadTextureData(BYTE* pbData, DWORD dwBytes, D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut)
{
    int width, height, channels;

    unsigned char* data = stbi_load_from_memory(pbData, dwBytes, &width, &height, &channels, 4);
    if (!data)
        return E_FAIL;

    HRESULT hr = LoadFromSTB(data, width, height, pSrcInfo, ppDataOut);

    stbi_image_free(data);
    return hr;
}

HRESULT C4JRender::SaveTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int *ppDataOut) { return S_OK; }
HRESULT C4JRender::SaveTextureDataToMemory(void *pOutput, int outputCapacity, int *outputLength, int width, int height, int *ppDataIn) { return S_OK; }
void C4JRender::TextureGetStats() {}
void* C4JRender::TextureGetTexture(int idx) { return nullptr; }

// we handle opengl calls cuz multiplatform is painful!!
void C4JRender::StateSetColour(float r, float g, float b, float a)
{
    ::glColor4f(r, g, b, a);
}

void C4JRender::StateSetDepthMask(bool enable)
{
    ::glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

void C4JRender::StateSetBlendEnable(bool enable)
{
    if (enable) ::glEnable(GL_BLEND); else ::glDisable(GL_BLEND);
}

void C4JRender::StateSetBlendFunc(int src, int dst)
{
    ::glBlendFunc(src, dst);
}

void C4JRender::StateSetBlendFactor(unsigned int colour)
{
    // colour is 0xAARRGGBB packed
    float a = ((colour >> 24) & 0xFF) / 255.0f;
    float r = ((colour >> 16) & 0xFF) / 255.0f;
    float g = ((colour >>  8) & 0xFF) / 255.0f;
    float b = ( colour        & 0xFF) / 255.0f;
    ::glBlendColor(r, g, b, a);
}

void C4JRender::StateSetAlphaFunc(int func, float param)
{
    ::glAlphaFunc(func, param);
}

void C4JRender::StateSetDepthFunc(int func)
{
    ::glDepthFunc(func);
}

void C4JRender::StateSetFaceCull(bool enable)
{
    if (enable) ::glEnable(GL_CULL_FACE); else ::glDisable(GL_CULL_FACE);
}

void C4JRender::StateSetFaceCullCW(bool enable)
{
    ::glFrontFace(enable ? GL_CW : GL_CCW);
}

void C4JRender::StateSetLineWidth(float width)
{
    ::glLineWidth(width);
}

void C4JRender::StateSetWriteEnable(bool red, bool green, bool blue, bool alpha)
{
    ::glColorMask(red, green, blue, alpha);
}

void C4JRender::StateSetDepthTestEnable(bool enable)
{
    if (enable) ::glEnable(GL_DEPTH_TEST); else ::glDisable(GL_DEPTH_TEST);
}

void C4JRender::StateSetAlphaTestEnable(bool enable)
{
    if (enable) ::glEnable(GL_ALPHA_TEST); else ::glDisable(GL_ALPHA_TEST);
}

void C4JRender::StateSetDepthSlopeAndBias(float slope, float bias)
{
    if (slope != 0.0f || bias != 0.0f) {
        ::glEnable(GL_POLYGON_OFFSET_FILL);
        ::glPolygonOffset(slope, bias);
    } else {
        ::glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void C4JRender::StateSetFogEnable(bool enable)
{
    if (enable) ::glEnable(GL_FOG); else ::glDisable(GL_FOG);
}

void C4JRender::StateSetFogMode(int mode)
{
    ::glFogi(GL_FOG_MODE, mode);
}

void C4JRender::StateSetFogNearDistance(float dist)
{
    ::glFogf(GL_FOG_START, dist);
}

void C4JRender::StateSetFogFarDistance(float dist)
{
    ::glFogf(GL_FOG_END, dist);
}

void C4JRender::StateSetFogDensity(float density)
{
    ::glFogf(GL_FOG_DENSITY, density);
}

void C4JRender::StateSetFogColour(float red, float green, float blue)
{
    float c[4] = {red, green, blue, 1.0f};
    ::glFogfv(GL_FOG_COLOR, c);
}

void C4JRender::StateSetLightingEnable(bool enable)
{
    if (enable) {
        ::glEnable(GL_LIGHTING);
        // Enable color material so glColor calls set material ambient+diffuse
        ::glEnable(GL_COLOR_MATERIAL);
        ::glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    } else {
        ::glDisable(GL_LIGHTING);
        ::glDisable(GL_COLOR_MATERIAL);
    }
}

void C4JRender::StateSetVertexTextureUV(float u, float v)
{
    ::glMultiTexCoord2f(GL_TEXTURE1, u, v);
}

void C4JRender::StateSetLightColour(int light, float red, float green, float blue)
{
    float diffuse[4] = {red, green, blue, 1.0f};
    ::glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, diffuse);
}

void C4JRender::StateSetLightAmbientColour(float red, float green, float blue)
{
    float ambient[4] = {red, green, blue, 1.0f};
    float model[4] = {red, green, blue, 1.0f};
    ::glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model);
    // Also set on light 0 as a fallback incase
    ::glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
}

void C4JRender::StateSetLightDirection(int light, float x, float y, float z)
{
    float dir[4] = {x, y, z, 0.0f};  // w=0 → directional light
    ::glLightfv(GL_LIGHT0 + light, GL_POSITION, dir);
}

void C4JRender::StateSetLightEnable(int light, bool enable)
{
    GLenum l = GL_LIGHT0 + light;
    if (enable) ::glEnable(l); else ::glDisable(l);
}

void C4JRender::StateSetViewport(eViewportType viewportType)
{
    // Use the full framebuffer for all viewport types
    ::glViewport(0, 0, s_windowWidth, s_windowHeight);
}

void C4JRender::StateSetEnableViewportClipPlanes(bool enable)
{
    // Clip planes not commonly used in the legacy path
    if (enable) ::glEnable(GL_CLIP_PLANE0); else ::glDisable(GL_CLIP_PLANE0);
}

void C4JRender::StateSetTexGenCol(int col, float x, float y, float z, float w, bool eyeSpace)
{
    GLenum coord;
    switch (col) {
    case 0: coord = GL_S; break;
    case 1: coord = GL_T; break;
    case 2: coord = GL_R; break;
    case 3: coord = GL_Q; break;
    default: coord = GL_S; break;
    }
    float plane[4] = {x, y, z, w};
    GLenum planeMode = eyeSpace ? GL_EYE_PLANE : GL_OBJECT_PLANE;
    ::glTexGeni(coord, GL_TEXTURE_GEN_MODE,
                eyeSpace ? GL_EYE_LINEAR : GL_OBJECT_LINEAR);
    ::glTexGenfv(coord, planeMode, plane);
}

void C4JRender::StateSetStencil(int Function, uint8_t stencil_ref,
                                uint8_t stencil_func_mask,
                                uint8_t stencil_write_mask)
{
    ::glEnable(GL_STENCIL_TEST);
    ::glStencilFunc(Function, stencil_ref, stencil_func_mask);
    ::glStencilMask(stencil_write_mask);
}

void C4JRender::StateSetForceLOD(int LOD) {} // No LOD bias in legacy GL path

void C4JRender::BeginEvent(LPCWSTR eventName) {}
void C4JRender::EndEvent() {}
void C4JRender::Suspend() {}
bool C4JRender::Suspended() { return false; }
void C4JRender::Resume() {}
