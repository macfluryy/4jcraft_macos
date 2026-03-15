#include "4J_Render.h"
#include <cstring>
#include <cstdlib>  // getenv

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>
#include <cstdio>
#include <cmath>
#include <pthread.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

C4JRender RenderManager;

// Hello SDL!
static SDL_Window* s_window = nullptr;
static SDL_GLContext s_glContext = nullptr;
static bool s_shouldClose = false;
static int s_textureLevels = 1;
static int s_windowWidth = 0;
static int s_windowHeight = 0;

// We set Window size with the monitor's res, so that I can get rid of ugly
// values.
static void SetInitialWindowSize() {
    int w = 0, h = 0;
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_DisplayMode mode;
        if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
            w = (int)(mode.w * 0.4f);
            h = (int)(mode.h * 0.4f);
        }
    }
    if (w > 0 && h > 0) {
        s_windowWidth = w;
        s_windowHeight = h;
    } else {
        s_windowWidth = 1280;
        s_windowHeight = 720;
    }
}
// (can't believe i had to rewrite this, i literally did it TODAY.)
static int s_reqWidth = 0;
static int s_reqHeight = 0;
// When we'll have a settings system in order, we'll set bool to that value,
// right now it's hardcoded.
static bool s_fullscreen = false;

static pthread_key_t s_glCtxKey;
static pthread_once_t s_glCtxKeyOnce = PTHREAD_ONCE_INIT;
static void makeGLCtxKey() { pthread_key_create(&s_glCtxKey, nullptr); }
// Do not touch exactly this number    |
static const int MAX_SHARED_CONTEXTS = 6;  // <- this one, do not touch
static SDL_Window* s_sharedContextWindows[MAX_SHARED_CONTEXTS] = {};
static SDL_GLContext s_sharedContexts[MAX_SHARED_CONTEXTS] = {};
static int s_sharedContextCount = 0;
static int s_nextSharedContext = 0;
static pthread_mutex_t s_sharedCtxMutex = PTHREAD_MUTEX_INITIALIZER;
// Tells thread to do Direct GL calls, just don't touch.
static pthread_mutex_t s_glCallMutex = PTHREAD_MUTEX_INITIALIZER;

// Track which thread is the main (rendering) thread
static pthread_t s_mainThread;
static bool s_mainThreadSet = false;

// viewport go brr
static void onFramebufferResize(int w, int h) {
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    s_windowWidth = w;
    s_windowHeight = h;
    ::glViewport(0, 0, w, h);
}

// V-Sync

// Initialize OpenGL & The SDL window.
void C4JRender::Initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[4J_Render] Failed to initialise SDL: %s\n",
                SDL_GetError());
        return;
    }
    SDL_DisplayMode mode;
    int haveMode = (SDL_GetCurrentDisplayMode(0, &mode) == 0);

    if (s_reqWidth > 0 && s_reqHeight > 0) {
        s_windowWidth = s_reqWidth;
        s_windowHeight = s_reqHeight;
    } else if (haveMode) {
        s_windowWidth = mode.w;
        s_windowHeight = mode.h;
    }
    fprintf(stderr, "[4J_Render] Window %dx%d  fullscreen=%s\n", s_windowWidth,
            s_windowHeight, s_fullscreen ? "yes" : "no");
    fflush(stderr);

    // Setting the sdl_gl ver. Change in future incase we want to use shaders
    // Yes i'm still using fixed functions, get mad at me
    //                                      I don't care.
    //     Im not gonna be rewriting the whole renderer.. AGAIN. ;w;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    Uint32 winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (s_fullscreen) winFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    s_window = SDL_CreateWindow("Minecraft Console Edition",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                s_windowWidth, s_windowHeight, winFlags);
    if (!s_window) {
        fprintf(stderr, "[4J_Render] Failed to create SDL window: %s\n",
                SDL_GetError());
        SDL_Quit();
        return;
    }

    s_glContext = SDL_GL_CreateContext(s_window);
    if (!s_glContext) {
        fprintf(stderr, "[4J_Render] Failed to create GL context: %s\n",
                SDL_GetError());
        SDL_DestroyWindow(s_window);
        s_window = nullptr;
        SDL_Quit();
        return;
    }

// 4JCraft VSync/V-Sync
#ifdef ENABLE_VSYNC
    SDL_GL_SetSwapInterval(1);  // V-Sync On Please.
#else
    SDL_GL_SetSwapInterval(0);  // V-Sync Off Please.
#endif

    int fw, fh;
    SDL_GetWindowSize(s_window, &fw, &fh);
    onFramebufferResize(fw, fh);

    // We initialize the OpenGL states. Touching those values makes some funny
    // artifacts appear.
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
    // Print the renderer's version incase we change it in the future.
    printf("[4J_Render] OpenGL %s  |  %s\n",
           (const char*)::glGetString(GL_VERSION),
           (const char*)::glGetString(GL_RENDERER));
    fflush(stdout);

    // Tag this as the main rendering thread
    pthread_once(&s_glCtxKeyOnce, makeGLCtxKey);
    s_mainThread = pthread_self();
    s_mainThreadSet = true;
    pthread_setspecific(s_glCtxKey, (void*)s_window);

    // Pre-create shared GL contexts for worker threads (chunk builders etc.)
    // Ensure they are invisible so they don't interfere with the window
    // manager.

    // Pre-create shared GL contexts for worker threads (chunk builders & other
    // shit etc.) SDL_GL_SHARE_WITH_CURRENT_CONTEXT my saviour.
    for (int i = 0; i < MAX_SHARED_CONTEXTS; i++) {
        SDL_Window* w = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED,
                                         SDL_WINDOWPOS_UNDEFINED, 1, 1,
                                         SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
        if (!w) break;
        // Ensure sharing
        // I've been stuck on this for a while. Im stupid..
        SDL_GL_MakeCurrent(s_window, s_glContext);
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        SDL_GLContext ctx = SDL_GL_CreateContext(w);
        if (!ctx) {
            SDL_DestroyWindow(w);
            break;
        }
        s_sharedContextWindows[s_sharedContextCount] = w;
        s_sharedContexts[s_sharedContextCount] = ctx;
        s_sharedContextCount++;
    }

    // Ensure main thread still has the context
    SDL_GL_MakeCurrent(s_window, s_glContext);
    fprintf(stderr,
            "[4J_Render] Created %d shared GL contexts for worker threads\n",
            s_sharedContextCount);
    fflush(stderr);
}

void C4JRender::InitialiseContext() {
    if (!s_window) return;
    pthread_once(&s_glCtxKeyOnce, makeGLCtxKey);

    // Main thread reclaiming context (e.g. after startup thread finishes)
    if (s_mainThreadSet && pthread_equal(pthread_self(), s_mainThread)) {
        SDL_GL_MakeCurrent(s_window, s_glContext);
        pthread_setspecific(s_glCtxKey, (void*)s_window);
        return;
    }

    // Worker thread checks if there's a context, we don't want to have multiple
    // contexts.
    void* ctxPtr = pthread_getspecific(s_glCtxKey);
    if (ctxPtr) {
        // ctxPtr -> SDL_GLContext pointer
        SDL_GLContext ctx = (SDL_GLContext)ctxPtr;
        int idx = -1;
        for (int i = 0; i < s_sharedContextCount; ++i)
            if (s_sharedContexts[i] == ctx) {
                idx = i;
                break;
            }
        if (idx >= 0 && s_sharedContextWindows[idx])
            SDL_GL_MakeCurrent(s_sharedContextWindows[idx], ctx);
        return;
    }

    // Grab a pre-created shared context from the pool
    pthread_mutex_lock(&s_sharedCtxMutex);
    SDL_GLContext shared = nullptr;
    if (s_nextSharedContext < s_sharedContextCount) {
        shared = s_sharedContexts[s_nextSharedContext++];
    }
    pthread_mutex_unlock(&s_sharedCtxMutex);

    if (!shared) {
        fprintf(stderr,
                "[4J_Render] ERROR: no shared GL contexts left for worker "
                "thread %lu!\n",
                (unsigned long)pthread_self());
        fflush(stderr);
        return;
    }
    // ewww..... look at line 201-203, we gotta make a function for that....
    int idx = -1;
    for (int i = 0; i < s_sharedContextCount; ++i)
        if (s_sharedContexts[i] == shared) {
            idx = i;
            break;
        }
    if (idx >= 0 && s_sharedContextWindows[idx])
        SDL_GL_MakeCurrent(s_sharedContextWindows[idx], shared);

    // Initialize some basic state for this context to ensure consistent display
    // list recording
    ::glEnable(GL_TEXTURE_2D);
    ::glEnable(GL_DEPTH_TEST);
    ::glDepthFunc(GL_LEQUAL);
    ::glAlphaFunc(GL_GREATER, 0.1f);
    ::glEnable(GL_BLEND);
    ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    ::glShadeModel(GL_SMOOTH);
    ::glEnable(GL_COLOR_MATERIAL);
    ::glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    pthread_setspecific(s_glCtxKey, (void*)shared);
    fprintf(stderr,
            "[4J_Render] Assigned shared GL context %p to worker thread %lu\n",
            (void*)shared, (unsigned long)pthread_self());
    fflush(stderr);
}

void C4JRender::StartFrame() {
    if (!s_window) return;
    int w, h;
    SDL_GetWindowSize(s_window, &w, &h);
    s_windowWidth = w > 0 ? w : 1;
    s_windowHeight = h > 0 ? h : 1;
    ::glViewport(0, 0, s_windowWidth, s_windowHeight);
}

void C4JRender::Present() {
    if (!s_window) return;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT)
            s_shouldClose = true;
        else if (ev.type == SDL_WINDOWEVENT) {
            if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
                s_shouldClose = true;
            else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                onFramebufferResize(ev.window.data1, ev.window.data2);
        }
    }
    // Present the rendered frame after processing input/events to avoid input
    // timing issues
    ::glFlush();
    // debug log to help diagnose mouse issues
    // printf("[4J_Render] Presenting frame (mouse lock=%d)\n", s_mouseLocked);
    // fflush(stdout);
    SDL_GL_SwapWindow(s_window);
}

void C4JRender::SetWindowSize(int w, int h) {
    s_reqWidth = (w > 0) ? w : 0;
    s_reqHeight = (h > 0) ? h : 0;
}

void C4JRender::SetFullscreen(bool fs) { s_fullscreen = fs; }

bool C4JRender::ShouldClose() { return !s_window || s_shouldClose; }

void C4JRender::Shutdown() {
    // Destroy the main window and clean up SDL resources so that
    // destructors running after the game loop don't touch a dead context.
    if (s_window) {
        if (s_glContext) {
            SDL_GL_DeleteContext(s_glContext);
            s_glContext = nullptr;
        }
        SDL_DestroyWindow(s_window);
        s_window = nullptr;
    }

    for (int i = 0; i < s_sharedContextCount; ++i) {
        if (s_sharedContexts[i]) {
            SDL_GL_DeleteContext(s_sharedContexts[i]);
            s_sharedContexts[i] = 0;
        }
        if (s_sharedContextWindows[i]) {
            SDL_DestroyWindow(s_sharedContextWindows[i]);
            s_sharedContextWindows[i] = nullptr;
        }
    }
    s_sharedContextCount = 0;
    SDL_Quit();
}

// rip glfw. you won't be missed. (i hope)
void C4JRender::DoScreenGrabOnNextPresent() {}

void C4JRender::Clear(int flags) { ::glClear(flags); }

void C4JRender::SetClearColour(const float colourRGBA[4]) {
    ::glClearColor(colourRGBA[0], colourRGBA[1], colourRGBA[2], colourRGBA[3]);
}

bool C4JRender::IsWidescreen() { return true; }
bool C4JRender::IsHiDef() { return true; }
void C4JRender::GetFramebufferSize(int& width, int& height) {
    width = s_windowWidth;
    height = s_windowHeight;
}
void C4JRender::CaptureThumbnail(ImageFileBuffer*) {}
void C4JRender::CaptureScreen(ImageFileBuffer*, XSOCIAL_PREVIEWIMAGE*) {}
void C4JRender::BeginConditionalSurvey(int) {}
void C4JRender::EndConditionalSurvey() {}
void C4JRender::BeginConditionalRendering(int) {}
void C4JRender::EndConditionalRendering() {}

void C4JRender::MatrixMode(int type) { ::glMatrixMode(type); }
void C4JRender::MatrixSetIdentity() { ::glLoadIdentity(); }
void C4JRender::MatrixTranslate(float x, float y, float z) {
    ::glTranslatef(x, y, z);
}

void C4JRender::MatrixRotate(float angle, float x, float y, float z) {
    // We use math from the math lib instead of hardcoding it. How Ugly.
    ::glRotatef(angle * (180.0f / static_cast<float>(M_PI)), x, y, z);
}

void C4JRender::MatrixScale(float x, float y, float z) { ::glScalef(x, y, z); }

void C4JRender::MatrixPerspective(float fovy, float aspect, float zNear,
                                  float zFar) {
    ::gluPerspective((double)fovy, (double)aspect, (double)zNear, (double)zFar);
}

void C4JRender::MatrixOrthogonal(float left, float right, float bottom,
                                 float top, float zNear, float zFar) {
    ::glOrtho(left, right, bottom, top, zNear, zFar);
}

void C4JRender::MatrixPop() { ::glPopMatrix(); }
void C4JRender::MatrixPush() { ::glPushMatrix(); }
void C4JRender::MatrixMult(float* mat) { ::glMultMatrixf(mat); }

const float* C4JRender::MatrixGet(int type) {
    static float mat[16];
    ::glGetFloatv(type, mat);
    return mat;
}

void C4JRender::Set_matrixDirty() {}  // immediate-mode

static GLenum mapPrimType(int pt) {
    // Handle GL constants first
    if (pt == GL_QUADS) return GL_QUADS;
    if (pt == GL_TRIANGLES) return GL_TRIANGLES;
    if (pt == GL_LINES) return GL_LINES;
    if (pt == GL_LINE_STRIP) return GL_LINE_STRIP;
    if (pt == GL_TRIANGLE_STRIP) return GL_TRIANGLE_STRIP;
    if (pt == GL_TRIANGLE_FAN) return GL_TRIANGLE_FAN;

    // Map from ePrimitiveType enum
    switch (pt) {
        case C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST:
            return GL_TRIANGLES;
        case C4JRender::PRIMITIVE_TYPE_TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case C4JRender::PRIMITIVE_TYPE_TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        case C4JRender::PRIMITIVE_TYPE_QUAD_LIST:
            return GL_QUADS;
        case C4JRender::PRIMITIVE_TYPE_LINE_LIST:
            return GL_LINES;
        case C4JRender::PRIMITIVE_TYPE_LINE_STRIP:
            return GL_LINE_STRIP;
        default:
            return GL_TRIANGLES;
    }
}

// This is the clientside vertex processing.
void C4JRender::DrawVertices(ePrimitiveType PrimitiveType, int count,
                             void* dataIn, eVertexType vType,
                             C4JRender::ePixelShaderType psType) {
    if (count <= 0 || !dataIn) return;

    // trash trash trash trash
    pthread_mutex_lock(&s_glCallMutex);

    GLenum mode = mapPrimType((int)PrimitiveType);

    if (vType == VERTEX_TYPE_COMPRESSED) {
        int16_t* sdata = (int16_t*)dataIn;
        ::glBegin(mode);
        for (int i = 0; i < count; i++) {
            int16_t* vert = sdata + i * 8;

            float x = vert[0] / 1024.0f;
            float y = vert[1] / 1024.0f;
            float z = vert[2] / 1024.0f;

            // Unpack RGB565 colour (Tesselator stores as packedcol - 32768 to
            // fit in int16)
            unsigned short packedColor = (unsigned short)((int)vert[3] + 32768);
            float r = ((packedColor >> 11) & 0x1f) / 31.0f;
            float g = ((packedColor >> 5) & 0x3f) / 63.0f;
            float b = (packedColor & 0x1f) / 31.0f;

            float fu = vert[4] / 8192.0f;
            float fv = vert[5] / 8192.0f;
            // Tesselator does that. Thanks 4J.
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
        unsigned int* idata = (unsigned int*)dataIn;
        ::glBegin(mode);
        for (int i = 0; i < count; i++) {
            float* fdata = (float*)(idata + i * 8);

            unsigned int colorInt = idata[i * 8 + 5];
            unsigned char cr = (colorInt >> 24) & 0xFF;
            unsigned char cg = (colorInt >> 16) & 0xFF;
            unsigned char cb = (colorInt >> 8) & 0xFF;
            unsigned char ca = colorInt & 0xFF;

            unsigned int normalInt = idata[i * 8 + 6];
            int8_t nx = (int8_t)(normalInt & 0xFF);
            int8_t ny = (int8_t)((normalInt >> 8) & 0xFF);
            int8_t nz = (int8_t)((normalInt >> 16) & 0xFF);

            unsigned int tex2Int = idata[i * 8 + 7];

            if (nx != 0 || ny != 0 || nz != 0) {
                ::glNormal3f(nx / 127.0f, ny / 127.0f, nz / 127.0f);
            }

            // This breaks particle colors.. i think. fixme!
            if (colorInt != 0) {
                ::glColor4ub(cr, cg, cb, ca);
            }

            ::glTexCoord2f(fdata[3], fdata[4]);

            // Unit 1 (lightmap) UVs - 0xfe00fe00 is sentinel for "no Unit 1
            // UVs" Ugly hack, replace soon.
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

    pthread_mutex_unlock(&s_glCallMutex);
}

void C4JRender::CBuffLockStaticCreations() {}

int C4JRender::CBuffCreate(int count) {
    int id = (int)::glGenLists(count);
    ::glFlush();
    return id;
}

void C4JRender::CBuffDelete(int first, int count) {
    if (first > 0 && count > 0) {
        ::glDeleteLists(first, count);
        ::glFlush();
    }
}

void C4JRender::CBuffStart(int index, bool /*full*/) {
    if (index > 0) {
        ::glNewList(index, GL_COMPILE);
        ::glFlush();
    }
}

void C4JRender::CBuffClear(int index) {
    if (index > 0) {
        ::glNewList(index, GL_COMPILE);
        ::glEndList();
        ::glFlush();
    }
}

int C4JRender::CBuffSize(int /*index*/) { return 0; }

void C4JRender::CBuffEnd() {
    ::glEndList();
    ::glFlush();
}

bool C4JRender::CBuffCall(int index, bool /*full*/) {
    if (index <= 0) return false;
    if (::glIsList(index)) {
        ::glCallList(index);
        return true;
    }
    return false;
}

void C4JRender::CBuffTick() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}

int C4JRender::TextureCreate() {
    GLuint id = 0;
    ::glGenTextures(1, &id);
    return (int)id;
}

void C4JRender::TextureFree(int idx) {
    GLuint id = (GLuint)idx;
    ::glDeleteTextures(1, &id);
}

void C4JRender::TextureBind(int idx) {
    if (idx < 0) {
        ::glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        ::glBindTexture(GL_TEXTURE_2D, (GLuint)idx);
    }
}

void C4JRender::TextureBindVertex(int idx, bool scaleLight) {
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

        // 4jcraft: jank workaround for entities
        // referenced from the disabled code in GameRenderer::turnOnLightLayer
        if (scaleLight) {
            ::glMatrixMode(GL_TEXTURE);
            ::glLoadIdentity();
            float s = 1 / 16.0f / 15.0f * 15 / 16;
            ::glScalef(s, s, s);
            ::glTranslatef(8.0f, 8.0f, 8.0f);
            ::glMatrixMode(GL_MODELVIEW);
        }
    }

    ::glActiveTexture(GL_TEXTURE0);
    ::glFlush();
}

void C4JRender::TextureSetTextureLevels(int levels) {
    // base level is always 0, no mipmaps sadly. I'll add them later.
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,
                      levels > 0 ? levels - 1 : 0);
    s_textureLevels = levels;
}
int C4JRender::TextureGetTextureLevels() { return s_textureLevels; }

void C4JRender::TextureData(int width, int height, void* data, int level,
                            eTextureFormat /*format*/) {
    // TODO: Check if correct format.
    ::glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA,
                   GL_UNSIGNED_BYTE, data);

    ::glFlush();

    if (level == 0) {
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

void C4JRender::TextureDataUpdate(int xoffset, int yoffset, int width,
                                  int height, void* data, int level) {
    ::glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height,
                      GL_RGBA, GL_UNSIGNED_BYTE, data);
    ::glFlush();
}

void C4JRender::TextureSetParam(int param, int value) {
    ::glTexParameteri(GL_TEXTURE_2D, param, value);
}

void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd() {}

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short) {}

// Converts RGBA data to the format expected by the texture loader.
static HRESULT LoadFromSTB(unsigned char* data, int width, int height,
                           D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut) {
    int pixelCount = width * height;
    int* pixels = new int[pixelCount];

    for (int i = 0; i < pixelCount; i++) {
        unsigned char r = data[i * 4 + 0];
        unsigned char g = data[i * 4 + 1];
        unsigned char b = data[i * 4 + 2];
        unsigned char a = data[i * 4 + 3];

        // pixels[i] = (a << 24) | (b << 16) | (g << 8) | r;
        pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    if (pSrcInfo) {
        pSrcInfo->Width = width;
        pSrcInfo->Height = height;
    }

    *ppDataOut = pixels;
    return S_OK;
}

HRESULT C4JRender::LoadTextureData(const char* szFilename,
                                   D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut) {
    int width, height, channels;

    unsigned char* data = stbi_load(szFilename, &width, &height, &channels, 4);
    if (!data) return E_FAIL;

    HRESULT hr = LoadFromSTB(data, width, height, pSrcInfo, ppDataOut);

    stbi_image_free(data);
    return hr;
}

HRESULT C4JRender::LoadTextureData(std::uint8_t* pbData,
                                   std::uint32_t byteCount,
                                   D3DXIMAGE_INFO* pSrcInfo, int** ppDataOut) {
    int width, height, channels;

    unsigned char* data =
        stbi_load_from_memory(pbData, byteCount, &width, &height, &channels, 4);
    if (!data) return E_FAIL;

    HRESULT hr = LoadFromSTB(data, width, height, pSrcInfo, ppDataOut);

    stbi_image_free(data);
    return hr;
}

HRESULT C4JRender::SaveTextureData(const char* szFilename,
                                   D3DXIMAGE_INFO* pSrcInfo, int* ppDataOut) {
    return S_OK;
}
HRESULT C4JRender::SaveTextureDataToMemory(void* pOutput, int outputCapacity,
                                           int* outputLength, int width,
                                           int height, int* ppDataIn) {
    return S_OK;
}
void C4JRender::TextureGetStats() {}
void* C4JRender::TextureGetTexture(int idx) { return nullptr; }

void C4JRender::StateSetColour(float r, float g, float b, float a) {
    ::glColor4f(r, g, b, a);
}

void C4JRender::StateSetDepthMask(bool enable) {
    ::glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

void C4JRender::StateSetBlendEnable(bool enable) {
    if (enable)
        ::glEnable(GL_BLEND);
    else
        ::glDisable(GL_BLEND);
}

void C4JRender::StateSetBlendFunc(int src, int dst) { ::glBlendFunc(src, dst); }

void C4JRender::StateSetBlendFactor(unsigned int colour) {
    // colour is 0xAARRGGBB packed
    float a = ((colour >> 24) & 0xFF) / 255.0f;
    float r = ((colour >> 16) & 0xFF) / 255.0f;
    float g = ((colour >> 8) & 0xFF) / 255.0f;
    float b = (colour & 0xFF) / 255.0f;
    ::glBlendColor(r, g, b, a);
}

void C4JRender::StateSetAlphaFunc(int func, float param) {
    ::glAlphaFunc(func, param);
}

void C4JRender::StateSetDepthFunc(int func) { ::glDepthFunc(func); }

void C4JRender::StateSetFaceCull(bool enable) {
    if (enable)
        ::glEnable(GL_CULL_FACE);
    else
        ::glDisable(GL_CULL_FACE);
}

void C4JRender::StateSetFaceCullCW(bool enable) {
    ::glFrontFace(enable ? GL_CW : GL_CCW);
}

void C4JRender::StateSetLineWidth(float width) { ::glLineWidth(width); }

void C4JRender::StateSetWriteEnable(bool red, bool green, bool blue,
                                    bool alpha) {
    ::glColorMask(red, green, blue, alpha);
}

void C4JRender::StateSetDepthTestEnable(bool enable) {
    if (enable)
        ::glEnable(GL_DEPTH_TEST);
    else
        ::glDisable(GL_DEPTH_TEST);
}

void C4JRender::StateSetAlphaTestEnable(bool enable) {
    if (enable)
        ::glEnable(GL_ALPHA_TEST);
    else
        ::glDisable(GL_ALPHA_TEST);
}

void C4JRender::StateSetDepthSlopeAndBias(float slope, float bias) {
    if (slope != 0.0f || bias != 0.0f) {
        ::glEnable(GL_POLYGON_OFFSET_FILL);
        ::glPolygonOffset(slope, bias);
    } else {
        ::glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void C4JRender::StateSetFogEnable(bool enable) {
    if (enable)
        ::glEnable(GL_FOG);
    else
        ::glDisable(GL_FOG);
}

void C4JRender::StateSetFogMode(int mode) { ::glFogi(GL_FOG_MODE, mode); }

void C4JRender::StateSetFogNearDistance(float dist) {
    ::glFogf(GL_FOG_START, dist);
}

void C4JRender::StateSetFogFarDistance(float dist) {
    ::glFogf(GL_FOG_END, dist);
}

void C4JRender::StateSetFogDensity(float density) {
    ::glFogf(GL_FOG_DENSITY, density);
}

void C4JRender::StateSetFogColour(float red, float green, float blue) {
    float c[4] = {red, green, blue, 1.0f};
    ::glFogfv(GL_FOG_COLOR, c);
}

void C4JRender::StateSetLightingEnable(bool enable) {
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

void C4JRender::StateSetVertexTextureUV(float u, float v) {
    ::glMultiTexCoord2f(GL_TEXTURE1, u, v);
}

void C4JRender::StateSetLightColour(int light, float red, float green,
                                    float blue) {
    float diffuse[4] = {red, green, blue, 1.0f};
    ::glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, diffuse);
}

void C4JRender::StateSetLightAmbientColour(float red, float green, float blue) {
    float ambient[4] = {red, green, blue, 1.0f};
    float model[4] = {red, green, blue, 1.0f};
    ::glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model);
    ::glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
}

void C4JRender::StateSetLightDirection(int light, float x, float y, float z) {
    float dir[4] = {x, y, z,
                    0.0f};  // TODO: Java seems to do the reverse, gotta check.
    ::glLightfv(GL_LIGHT0 + light, GL_POSITION, dir);
}

void C4JRender::StateSetLightEnable(int light, bool enable) {
    GLenum l = GL_LIGHT0 + light;
    if (enable)
        ::glEnable(l);
    else
        ::glDisable(l);
}

void C4JRender::StateSetViewport(eViewportType viewportType) {
    // Use the full framebuffer for all viewport types
    ::glViewport(0, 0, s_windowWidth, s_windowHeight);
}

void C4JRender::StateSetEnableViewportClipPlanes(bool enable) {
    // Clip planes not commonly used in the legacy path
    if (enable)
        ::glEnable(GL_CLIP_PLANE0);
    else
        ::glDisable(GL_CLIP_PLANE0);
}

void C4JRender::StateSetTexGenCol(int col, float x, float y, float z, float w,
                                  bool eyeSpace) {
    GLenum coord;
    switch (col) {
        case 0:
            coord = GL_S;
            break;
        case 1:
            coord = GL_T;
            break;
        case 2:
            coord = GL_R;
            break;
        case 3:
            coord = GL_Q;
            break;
        default:
            coord = GL_S;
            break;
    }
    float plane[4] = {x, y, z, w};
    GLenum planeMode = eyeSpace ? GL_EYE_PLANE : GL_OBJECT_PLANE;
    ::glTexGeni(coord, GL_TEXTURE_GEN_MODE,
                eyeSpace ? GL_EYE_LINEAR : GL_OBJECT_LINEAR);
    ::glTexGenfv(coord, planeMode, plane);
}

void C4JRender::StateSetStencil(int Function, uint8_t stencil_ref,
                                uint8_t stencil_func_mask,
                                uint8_t stencil_write_mask) {
    ::glEnable(GL_STENCIL_TEST);
    ::glStencilFunc(Function, stencil_ref, stencil_func_mask);
    ::glStencilMask(stencil_write_mask);
}

void C4JRender::StateSetForceLOD(int LOD) {}  // No LOD bias in legacy GL path

void C4JRender::BeginEvent(LPCWSTR eventName) {}
void C4JRender::EndEvent() {}
void C4JRender::Suspend() {}
bool C4JRender::Suspended() { return false; }
void C4JRender::Resume() {}
