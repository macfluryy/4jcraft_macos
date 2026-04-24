//   - LinuxGLLogLightmapState renamed to MacGLLogLightmapState.

#if defined(__APPLE__)

#ifndef USE_METAL
#include <OpenGL/gl.h>
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "platform/sdl2/Render.h"
#include "app/include/stubs.h"
#include "java/ByteBuffer.h"
#include "java/FloatBuffer.h"
#include "java/IntBuffer.h"

#include <stdio.h>
#include <string.h>

extern C4JRender RenderManager;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

inline int* getIntPtr(IntBuffer* buf) {
    return buf ? (int*)buf->getBuffer() + buf->position() : nullptr;
}

inline void* getBytePtr(ByteBuffer* buf) {
    return buf ? (char*)buf->getBuffer() + buf->position() : nullptr;
}

// ---------------------------------------------------------------------------
// Texture helpers
// ---------------------------------------------------------------------------

void glGenTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int  n   = buf->limit() - buf->position();
    int* dst = getIntPtr(buf);
    for (int i = 0; i < n; i++) dst[i] = RenderManager.TextureCreate();
}

void glDeleteTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int  n   = buf->limit() - buf->position();
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

// ---------------------------------------------------------------------------
// Lighting / fog / matrix helpers
// ---------------------------------------------------------------------------

void glLight_4J(int light, int pname, FloatBuffer* params) {
    const float* p   = params->_getDataPointer();
    int          idx = (light == 0x4001) ? 1 : 0;
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

// ---------------------------------------------------------------------------
// Display-list / misc helpers
// ---------------------------------------------------------------------------

void glCallLists_4J(IntBuffer* lists) {
    if (!lists) return;
    int  count = lists->limit() - lists->position();
    int* ids   = getIntPtr(lists);
    for (int i = 0; i < count; i++) RenderManager.CBuffCall(ids[i], false);
}

void glReadPixels_4J(int x, int y, int w, int h, int f, int t, ByteBuffer* p) {
    (void)f;
    (void)t;
    RenderManager.ReadPixels(x, y, w, h, getBytePtr(p));
}

void glTexCoordPointer_4J(int, int, FloatBuffer*)  {}
void glNormalPointer_4J(int, ByteBuffer*)           {}
void glColorPointer_4J(int, bool, int, ByteBuffer*) {}
void glVertexPointer_4J(int, int, FloatBuffer*)     {}
void glEndList_4J(int)                              {}
void glTexGen_4J(int, int, FloatBuffer*)            {}

void glGenQueriesARB_4J(IntBuffer* buf) {
#ifndef USE_METAL
    if (!buf) return;
    int n = buf->limit() - buf->position();
    if (n > 0) ::glGenQueries(n, (GLuint*)getIntPtr(buf));
#else
    (void)buf;
#endif
}

void glBeginQueryARB_4J(int target, int id) {
#ifndef USE_METAL
    ::glBeginQuery((GLenum)target, (GLuint)id);
#else
    (void)target; (void)id;
#endif
}

void glEndQueryARB_4J(int target) {
#ifndef USE_METAL
    ::glEndQuery((GLenum)target);
#else
    (void)target;
#endif
}

void glGetQueryObjectuARB_4J(int id, int pname, IntBuffer* params) {
#ifndef USE_METAL
    if (params)
        ::glGetQueryObjectuiv((GLuint)id, (GLenum)pname,
                              (GLuint*)getIntPtr(params));
#else
    (void)id; (void)pname; (void)params;
#endif
}

// ---------------------------------------------------------------------------
// Public wrapper (matches header declaration)
// ---------------------------------------------------------------------------

void glGetFloat(int pname, FloatBuffer* params) {
    glGetFloat_4J(pname, params);
}

// ---------------------------------------------------------------------------
// Debug helper (renamed from Linux* to Mac*)
// ---------------------------------------------------------------------------

void MacGLLogLightmapState(const char* stage, int textureId, bool scaleLight) {
    static int logCount = 0;
    if (logCount >= 16) return;
    ++logCount;
    fprintf(stderr, "[macos-lightmap] %s tex=%d scale=%d\n", stage, textureId,
            scaleLight ? 1 : 0);
}

#pragma clang diagnostic pop

#endif /* __APPLE__ */