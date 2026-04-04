#ifdef __linux__

#include "platform/sdl2/Render.h"
#include "app/include/stubs.h"

#include "java/ByteBuffer.h"
#include "java/FloatBuffer.h"
#include "java/IntBuffer.h"

extern C4JRender RenderManager;

#ifdef GLES
extern "C" {
extern void glClearDepthf(float depth);
void glClearDepth(double depth) { glClearDepthf((float)depth); }
void glTexGeni(unsigned int, unsigned int, int) {}
void glTexGenfv(unsigned int, unsigned int, const float*) {}
void glTexCoordPointer(int, unsigned int, int, const void*) {}
void glNormalPointer(unsigned int, int, const void*) {}
void glColorPointer(int, unsigned int, int, const void*) {}
void glVertexPointer(int, unsigned int, int, const void*) {}
void glEndList(void) {}
void glCallLists(int, unsigned int, const void*) {}
}
#endif

inline int* getIntPtr(IntBuffer* buf) {
    return buf ? (int*)buf->getBuffer() + buf->position() : nullptr;
}
inline void* getBytePtr(ByteBuffer* buf) {
    return buf ? (char*)buf->getBuffer() + buf->position() : nullptr;
}

void glGenTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int n = buf->limit() - buf->position();
    int* dst = getIntPtr(buf);
    for (int i = 0; i < n; i++) dst[i] = RenderManager.TextureCreate();
}

void glDeleteTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int n = buf->limit() - buf->position();
    int* src = getIntPtr(buf);
    for (int i = 0; i < n; i++) RenderManager.TextureFree(src[i]);
}

void glTexImage2D_4J(int target, int level, int internalformat, int width,
                     int height, int border, int format, int type,
                     ByteBuffer* pixels) {
    (void)target;
    (void)internalformat;
    (void)border;
    (void)format;
    (void)type;
    RenderManager.TextureData(width, height, getBytePtr(pixels), level,
                              C4JRender::TEXTURE_FORMAT_RxGyBzAw);
}

void glLight_4J(int light, int pname, FloatBuffer* params) {
    const float* p = params->_getDataPointer();
    int idx = (light == 0x4001) ? 1 : 0;
    if (pname == 0x1203)
        RenderManager.StateSetLightDirection(idx, p[0], p[1], p[2]);
    else if (pname == 0x1201)
        RenderManager.StateSetLightColour(idx, p[0], p[1], p[2]);
    else if (pname == 0x1200)
        RenderManager.StateSetLightAmbientColour(p[0], p[1], p[2]);
}

void glLightModel_4J(int pname, FloatBuffer* params) {
    if (pname == 0x0B53) {
        const float* p = params->_getDataPointer();
        RenderManager.StateSetLightAmbientColour(p[0], p[1], p[2]);
    }
}

void glFog_4J(int pname, FloatBuffer* params) {
    const float* p = params->_getDataPointer();
    if (pname == 0x0B66) RenderManager.StateSetFogColour(p[0], p[1], p[2]);
}

void glGetFloat_4J(int pname, FloatBuffer* params) {
    const float* m = RenderManager.MatrixGet(pname);
    if (m) memcpy(params->_getDataPointer(), m, 16 * sizeof(float));
}

void glCallLists_4J(IntBuffer* lists) {
    if (!lists) return;
    int count = lists->limit() - lists->position();
    int* ids = getIntPtr(lists);
    for (int i = 0; i < count; i++) RenderManager.CBuffCall(ids[i], false);
}

void glReadPixels_4J(int x, int y, int w, int h, int f, int t, ByteBuffer* p) {
    (void)f;
    (void)t;
    RenderManager.ReadPixels(x, y, w, h, getBytePtr(p));
}

// dead stubs
void glTexCoordPointer_4J(int, int, FloatBuffer*) {}
void glNormalPointer_4J(int, ByteBuffer*) {}
void glColorPointer_4J(int, bool, int, ByteBuffer*) {}
void glVertexPointer_4J(int, int, FloatBuffer*) {}
void glEndList_4J(int) {}
void glTexGen_4J(int, int, FloatBuffer*) {}

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static PFNGLGENQUERIESARBPROC _glGenQueriesARB = nullptr;
static PFNGLBEGINQUERYARBPROC _glBeginQueryARB = nullptr;
static PFNGLENDQUERYARBPROC _glEndQueryARB = nullptr;
static PFNGLGETQUERYOBJECTUIVARBPROC _glGetQueryObjectuivARB = nullptr;
static bool _queriesInitialized = false;

static void initQueryFuncs() {
    if (_queriesInitialized) return;
    _queriesInitialized = true;
    _glGenQueriesARB =
        (PFNGLGENQUERIESARBPROC)dlsym(RTLD_DEFAULT, "glGenQueriesARB");
    _glBeginQueryARB =
        (PFNGLBEGINQUERYARBPROC)dlsym(RTLD_DEFAULT, "glBeginQueryARB");
    _glEndQueryARB = (PFNGLENDQUERYARBPROC)dlsym(RTLD_DEFAULT, "glEndQueryARB");
    _glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)dlsym(
        RTLD_DEFAULT, "glGetQueryObjectuivARB");
}

void glGenQueriesARB_4J(IntBuffer* buf) {
    initQueryFuncs();
    if (_glGenQueriesARB && buf) {
        int n = buf->limit() - buf->position();
        if (n > 0) _glGenQueriesARB(n, (GLuint*)getIntPtr(buf));
    }
}

void glBeginQueryARB_4J(int target, int id) {
    initQueryFuncs();
    if (_glBeginQueryARB) _glBeginQueryARB((GLenum)target, (GLuint)id);
}

void glEndQueryARB_4J(int target) {
    initQueryFuncs();
    if (_glEndQueryARB) _glEndQueryARB((GLenum)target);
}

void glGetQueryObjectuARB_4J(int id, int pname, IntBuffer* params) {
    initQueryFuncs();
    if (_glGetQueryObjectuivARB && params)
        // LWJGL does not change limits/positions during these calls, it
        // reads/writes exactly at pointer!!
        _glGetQueryObjectuivARB((GLuint)id, (GLenum)pname,
                                (GLuint*)getIntPtr(params));
}
void glGetFloat(int pname, FloatBuffer* params) {
    glGetFloat_4J(pname, params);
}
void LinuxGLLogLightmapState(const char* stage, int textureId,
                             bool scaleLight) {
    static int logCount = 0;
    if (logCount >= 16) return;
    ++logCount;
    fprintf(stderr, "[linux-lightmap] %s tex=%d scale=%d\n", stage, textureId,
            scaleLight ? 1 : 0);
}
#endif