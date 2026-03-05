#include "4J_Render.h"
#include <cstring>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cmath>
#include <pthread.h>

C4JRender RenderManager;

static GLFWwindow *s_window = nullptr;
static int s_textureLevels = 1;
static int s_windowWidth = 1920;
static int s_windowHeight = 1080;

// Thread-local storage for per-thread shared GL contexts.
// The main thread uses s_window directly; worker threads get invisible
// windows that share objects (textures, display lists) with s_window.
static pthread_key_t s_glCtxKey;
static pthread_once_t s_glCtxKeyOnce = PTHREAD_ONCE_INIT;
static void makeGLCtxKey() { pthread_key_create(&s_glCtxKey, nullptr); }

// Pre-created pool of shared contexts for worker threads
static const int MAX_SHARED_CONTEXTS = 8;
static GLFWwindow *s_sharedContexts[MAX_SHARED_CONTEXTS] = {};
static int s_sharedContextCount = 0;
static int s_nextSharedContext = 0;
static pthread_mutex_t s_sharedCtxMutex = PTHREAD_MUTEX_INITIALIZER;

// Track which thread is the main (rendering) thread
static pthread_t s_mainThread;
static bool s_mainThreadSet = false;

void C4JRender::Initialise()
{
    if (!glfwInit()) {
        fprintf(stderr, "[4J_Render] Failed to initialise GLFW\n");
        return;
    }

    // opengl 2.1!!!
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    s_window = glfwCreateWindow(s_windowWidth, s_windowHeight,
                                "Minecraft Console Edition", nullptr, nullptr);
    if (!s_window) {
        fprintf(stderr, "[4J_Render] Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(s_window);
    glfwSwapInterval(1);  // vsync

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
    for (int i = 0; i < MAX_SHARED_CONTEXTS; i++) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        s_sharedContexts[i] = glfwCreateWindow(1, 1, "", nullptr, s_window);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        if (s_sharedContexts[i]) {
            s_sharedContextCount++;
        } else {
            fprintf(stderr, "[4J_Render] WARN: only created %d/%d shared contexts\n", i, MAX_SHARED_CONTEXTS);
            break;
        }
    }
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
        fprintf(stderr, "[4J_Render] ERROR: no shared GL contexts left for worker thread!\n");
        return;
    }
    glfwMakeContextCurrent(shared);
    pthread_setspecific(s_glCtxKey, shared);
    fprintf(stderr, "[4J_Render] Assigned shared GL context %p to worker thread\n", (void*)shared);
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

bool C4JRender::ShouldClose()
{
    return !s_window || glfwWindowShouldClose(s_window);
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
    switch (pt) {
    case 0:  return GL_TRIANGLES;      // C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST
    case GL_LINES:          return GL_LINES;
    case GL_LINE_STRIP:     return GL_LINE_STRIP;
    case GL_TRIANGLES:      return GL_TRIANGLES;
    case GL_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case GL_TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
    case GL_QUADS:          return GL_QUADS;
    default:                return GL_TRIANGLES;
    }
}

// clientside awawawa
static bool isCompilingDisplayList()
{
    GLint listMode = 0;
    ::glGetIntegerv(GL_LIST_MODE, &listMode);
    return (listMode == GL_COMPILE || listMode == GL_COMPILE_AND_EXECUTE);
}

void C4JRender::DrawVertices(ePrimitiveType PrimitiveType, int count,
                             void *dataIn, eVertexType vType,
                             C4JRender::ePixelShaderType psType)
{
    if (count <= 0 || !dataIn) return;

    GLenum mode = mapPrimType((int)PrimitiveType);
    unsigned char *data = (unsigned char *)dataIn;

    // Vertex layout: 3 floats pos, 2 floats tex, 4 bytes color, 4 bytes normal, 4 bytes padding = 32 bytes
    const int stride = 32;

    // Color byte-order fix for little-endian (x86/x64):
    // Console code (Xbox 360 / PS3, big-endian) stores color as int col = (r<<24)|(g<<16)|(b<<8)|a
    // Big-endian memory:    [r, g, b, a] — correct for glColor4ub(col[0], col[1], col[2], col[3])
    // Little-endian memory: [a, b, g, r] — bytes are reversed!
    // Fix: read bytes in reverse order col[3]=r, col[2]=g, col[1]=b, col[0]=a

    if (isCompilingDisplayList()) {
        ::glBegin(mode);
        for (int i = 0; i < count; i++) {
            unsigned char *v = data + i * stride;
            float *pos = (float *)(v);
            float *tex = (float *)(v + 12);
            unsigned char *col = v + 20;
            signed char *nrm = (signed char *)(v + 24);

            ::glNormal3f(nrm[0] / 127.0f, nrm[1] / 127.0f, nrm[2] / 127.0f);
            ::glColor4ub(col[3], col[2], col[1], col[0]); // LE fix: r,g,b,a from reversed bytes
            ::glTexCoord2f(tex[0], tex[1]);
            ::glVertex3f(pos[0], pos[1], pos[2]);
        }
        ::glEnd();
    } else {
        // For vertex array path, swap color bytes in-place to RGBA order
        for (int i = 0; i < count; i++) {
            unsigned char *col = data + i * stride + 20;
            unsigned char tmp;
            tmp = col[0]; col[0] = col[3]; col[3] = tmp; // swap a<->r
            tmp = col[1]; col[1] = col[2]; col[2] = tmp; // swap b<->g
        }

        ::glEnableClientState(GL_VERTEX_ARRAY);
        ::glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        ::glEnableClientState(GL_COLOR_ARRAY);
        ::glEnableClientState(GL_NORMAL_ARRAY);

        ::glVertexPointer(3, GL_FLOAT, stride, data);
        ::glTexCoordPointer(2, GL_FLOAT, stride, data + 12);
        ::glColorPointer(4, GL_UNSIGNED_BYTE, stride, data + 20);
        ::glNormalPointer(GL_BYTE, stride, data + 24);

        ::glDrawArrays(mode, 0, count);

        // Swap back to preserve original data
        for (int i = 0; i < count; i++) {
            unsigned char *col = data + i * stride + 20;
            unsigned char tmp;
            tmp = col[0]; col[0] = col[3]; col[3] = tmp;
            tmp = col[1]; col[1] = col[2]; col[2] = tmp;
        }
    }
}


void C4JRender::CBuffLockStaticCreations() {}

int C4JRender::CBuffCreate(int count)
{
    return (int)::glGenLists(count);
}

void C4JRender::CBuffDelete(int first, int count)
{
    if (first > 0 && count > 0) ::glDeleteLists(first, count);
}

void C4JRender::CBuffStart(int index, bool /*full*/)
{
    if (index > 0) ::glNewList(index, GL_COMPILE);
}

void C4JRender::CBuffClear(int index)
{
    if (index > 0) { ::glNewList(index, GL_COMPILE); ::glEndList(); }
}

int  C4JRender::CBuffSize(int /*index*/) { return 0; }

void C4JRender::CBuffEnd()
{
    ::glEndList();
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
    ::glActiveTexture(GL_TEXTURE1);
    if (idx < 0) {
        ::glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        ::glBindTexture(GL_TEXTURE_2D, (GLuint)idx);
    }
    ::glActiveTexture(GL_TEXTURE0);
}

void C4JRender::TextureSetTextureLevels(int levels) { s_textureLevels = levels; }
int  C4JRender::TextureGetTextureLevels()            { return s_textureLevels; }

void C4JRender::TextureData(int width, int height, void *data, int level,
                            eTextureFormat /*format*/)
{
    ::glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8,
                   width, height, 0,
                   GL_BGRA, GL_UNSIGNED_BYTE, data);

    if (level == 0) {
        if (s_textureLevels > 1) {
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                              GL_NEAREST_MIPMAP_LINEAR);
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                              s_textureLevels - 1);
        } else {
            ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
}

void C4JRender::TextureDataUpdate(int xoffset, int yoffset,
                                  int width, int height,
                                  void *data, int level)
{
    ::glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset,
                      width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);
}

void C4JRender::TextureSetParam(int param, int value)
{
    ::glTexParameteri(GL_TEXTURE_2D, param, value);
}

void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd()   {}

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short) {}
// really don't know if this is nessesary but didn't find any other functions to load images properly as a png..
// im sorry.
#ifdef __linux__
#include <png.h>
#include <stdio.h>
#include <string.h>

static HRESULT LoadPNGFromRows(png_structp png, png_infop info, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    unsigned char *buf = new unsigned char[width * height * 4];
    png_bytep *rows = new png_bytep[height];
    for (int y = 0; y < height; y++)
        rows[y] = buf + y * width * 4;
    png_read_image(png, rows);
    delete[] rows;
    // considering i worked on previous projects with raw pngs,,,,, 
    int *pixels = new int[width * height];
    for (int i = 0; i < width * height; i++)
    {
        unsigned char r = buf[i * 4 + 0];
        unsigned char g = buf[i * 4 + 1];
        unsigned char b = buf[i * 4 + 2];
        unsigned char a = buf[i * 4 + 3];
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    delete[] buf;

    pSrcInfo->Width = width;
    pSrcInfo->Height = height;
    *ppDataOut = pixels;
    return S_OK;
}

HRESULT C4JRender::LoadTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    FILE *fp = fopen(szFilename, "rb");
    if (!fp) return E_FAIL;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) { fclose(fp); return E_FAIL; }
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, NULL, NULL); fclose(fp); return E_FAIL; }
    if (setjmp(png_jmpbuf(png))) { png_destroy_read_struct(&png, &info, NULL); fclose(fp); return E_FAIL; }

    png_init_io(png, fp);
    png_read_info(png, info);

    HRESULT hr = LoadPNGFromRows(png, info, pSrcInfo, ppDataOut);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(fp);
    return hr;
}

struct PNGMemReader { const unsigned char *data; png_size_t pos; png_size_t size; };

static void png_mem_read(png_structp png, png_bytep out, png_size_t len)
{
    PNGMemReader *r = (PNGMemReader *)png_get_io_ptr(png);
    if (r->pos + len > r->size) len = r->size - r->pos;
    memcpy(out, r->data + r->pos, len);
    r->pos += len;
}

HRESULT C4JRender::LoadTextureData(BYTE *pbData, DWORD dwBytes, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) return E_FAIL;
    png_infop info = png_create_info_struct(png);
    if (!info) { png_destroy_read_struct(&png, NULL, NULL); return E_FAIL; }
    if (setjmp(png_jmpbuf(png))) { png_destroy_read_struct(&png, &info, NULL); return E_FAIL; }

    PNGMemReader reader = { pbData, 0, dwBytes };
    png_set_read_fn(png, &reader, png_mem_read);
    png_read_info(png, info);

    HRESULT hr = LoadPNGFromRows(png, info, pSrcInfo, ppDataOut);
    png_destroy_read_struct(&png, &info, NULL);
    return hr;
}

#else
HRESULT C4JRender::LoadTextureData(const char *szFilename, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut) { return S_OK; }
HRESULT C4JRender::LoadTextureData(BYTE *pbData, DWORD dwBytes, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut) { return S_OK; }
#endif
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
    if (enable) ::glEnable(GL_LIGHTING); else ::glDisable(GL_LIGHTING);
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
