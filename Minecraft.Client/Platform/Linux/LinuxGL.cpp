#ifdef __linux__

#include "../stdafx.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <dlfcn.h>
#include <vector>

#include "../../Minecraft.World/IO/Streams/IntBuffer.h"
#include "../../Minecraft.World/IO/Streams/FloatBuffer.h"
#include "../../Minecraft.World/IO/Streams/ByteBuffer.h"

#undef glGenTextures
#undef glDeleteTextures
#undef glLight
#undef glLightModel
#undef glGetFloat
#undef glTexGen
#undef glFog
#undef glTexCoordPointer
#undef glNormalPointer
#undef glColorPointer
#undef glVertexPointer
#undef glEndList
#undef glTexImage2D
#undef glCallLists
#undef glReadPixels
#undef glActiveTexture

// Helper functions & stuff
inline GLuint* getIntPtr(IntBuffer* buf) {
    return buf ? (GLuint*)((int*)buf->getBuffer() + buf->position()) : nullptr;
}
inline GLvoid* getBytePtr(ByteBuffer* buf) {
    return buf ? (GLvoid*)((char*)buf->getBuffer() + buf->position()) : nullptr;
}

// _4j suffix shit
int glGenTextures() {
    GLuint id = 0;
    ::glGenTextures(1, &id);
    return (int)id;
}

void glGenTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int n = buf->limit() - buf->position();
    if (n > 0) {
        ::glGenTextures(n, getIntPtr(buf));
    }
}

void glDeleteTextures(int id) {
    GLuint uid = (GLuint)id;
    ::glDeleteTextures(1, &uid);
}

void glDeleteTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int n = buf->limit() - buf->position();
    if (n > 0) {
        ::glDeleteTextures(n, getIntPtr(buf));
    }
}

void glLight_4J(int light, int pname, FloatBuffer* params) {
    ::glLightfv((GLenum)light, (GLenum)pname, params->_getDataPointer());
}

void glLightModel_4J(int pname, FloatBuffer* params) {
    ::glLightModelfv((GLenum)pname, params->_getDataPointer());
}

void glGetFloat_4J(int pname, FloatBuffer* params) {
    ::glGetFloatv((GLenum)pname, params->_getDataPointer());
}

void glTexGen_4J(int coord, int pname, FloatBuffer* params) {
    ::glTexGenfv((GLenum)coord, (GLenum)pname, params->_getDataPointer());
}

void glFog_4J(int pname, FloatBuffer* params) {
    ::glFogfv((GLenum)pname, params->_getDataPointer());
}

void glTexCoordPointer_4J(int size, int type, FloatBuffer* pointer) {
    ::glTexCoordPointer(size, (GLenum)type, 0, pointer->_getDataPointer());
}

void glNormalPointer_4J(int type, ByteBuffer* pointer) {
    ::glNormalPointer((GLenum)type, 0, getBytePtr(pointer));
}

void glColorPointer_4J(int size, bool normalized, int stride,
                       ByteBuffer* pointer) {
    (void)normalized;
    ::glColorPointer(size, GL_UNSIGNED_BYTE, stride, getBytePtr(pointer));
}

void glVertexPointer_4J(int size, int type, FloatBuffer* pointer) {
    ::glVertexPointer(size, (GLenum)type, 0, pointer->_getDataPointer());
}

void glEndList_4J(int dummy) {
    (void)dummy;
    ::glEndList();
}

void glTexImage2D_4J(int target, int level, int internalformat, int width,
                     int height, int border, int format, int type,
                     ByteBuffer* pixels) {
    ::glTexImage2D((GLenum)target, level, internalformat, width, height, border,
                   (GLenum)format, (GLenum)type, getBytePtr(pixels));
}

void glCallLists_4J(IntBuffer* lists) {
    if (!lists) return;
    int count = lists->limit() - lists->position();
    if (count > 0) {
        ::glCallLists(count, GL_INT, getIntPtr(lists));
    }
}

void glReadPixels_4J(int x, int y, int width, int height, int format, int type,
                     ByteBuffer* pixels) {
    ::glReadPixels(x, y, width, height, (GLenum)format, (GLenum)type,
                   getBytePtr(pixels));
}

void glGetFloat(int pname, FloatBuffer* params) {
    glGetFloat_4J(pname, params);
}
void glGenTextures(IntBuffer* buf) { glGenTextures_4J(buf); }
void glDeleteTextures(IntBuffer* buf) { glDeleteTextures_4J(buf); }
void glLight(int light, int pname, FloatBuffer* params) {
    glLight_4J(light, pname, params);
}
void glLightModel(int pname, FloatBuffer* params) {
    glLightModel_4J(pname, params);
}
void glTexGen(int coord, int pname, FloatBuffer* params) {
    glTexGen_4J(coord, pname, params);
}
void glFog(int pname, FloatBuffer* params) { glFog_4J(pname, params); }
void glTexCoordPointer(int size, int type, FloatBuffer* pointer) {
    glTexCoordPointer_4J(size, type, pointer);
}
void glNormalPointer(int type, ByteBuffer* pointer) {
    glNormalPointer_4J(type, pointer);
}
void glColorPointer(int size, bool normalized, int stride,
                    ByteBuffer* pointer) {
    glColorPointer_4J(size, normalized, stride, pointer);
}
void glVertexPointer(int size, int type, FloatBuffer* pointer) {
    glVertexPointer_4J(size, type, pointer);
}
void glTexImage2D(int t, int l, int i, int w, int h, int b, int f, int ty,
                  ByteBuffer* p) {
    glTexImage2D_4J(t, l, i, w, h, b, f, ty, p);
}
void glCallLists(IntBuffer* lists) { glCallLists_4J(lists); }
void glReadPixels(int x, int y, int w, int h, int f, int t, ByteBuffer* p) {
    glReadPixels_4J(x, y, w, h, f, t, p);
}

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
        if (n > 0) {
            _glGenQueriesARB(n, getIntPtr(buf));
        }
    }
}
void glGenQueriesARB(IntBuffer* buf) { glGenQueriesARB_4J(buf); }

void glBeginQueryARB_4J(int target, int id) {
    initQueryFuncs();
    if (_glBeginQueryARB) _glBeginQueryARB((GLenum)target, (GLuint)id);
}
void glBeginQueryARB(int target, int id) { glBeginQueryARB_4J(target, id); }

void glEndQueryARB_4J(int target) {
    initQueryFuncs();
    if (_glEndQueryARB) _glEndQueryARB((GLenum)target);
}
void glEndQueryARB(int target) { glEndQueryARB_4J(target); }

void glGetQueryObjectuARB_4J(int id, int pname, IntBuffer* params) {
    initQueryFuncs();
    if (_glGetQueryObjectuivARB && params) {
        // LWJGL does not change limits/positions during these calls, it
        // reads/writes exactly at pointer!!
        _glGetQueryObjectuivARB((GLuint)id, (GLenum)pname, getIntPtr(params));
    }
}
void glGetQueryObjectuARB(int id, int pname, IntBuffer* params) {
    glGetQueryObjectuARB_4J(id, pname, params);
}

void LinuxGLLogLightmapState(const char* stage, int textureId,
                             bool scaleLight) {
    static int logCount = 0;
    if (logCount >= 16) return;
    ++logCount;

    GLint activeTexture = 0;
    ::glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    const GLint restoreTexture = activeTexture;
    ::glActiveTexture(GL_TEXTURE1);
    GLint unit1Binding = 0;
    ::glGetIntegerv(GL_TEXTURE_BINDING_2D, &unit1Binding);
    ::glActiveTexture(restoreTexture);

    app.DebugPrintf(
        "[linux-lightmap] %s tex=%d scale=%d active=%#x unit1Bound=%d\n", stage,
        textureId, scaleLight ? 1 : 0, activeTexture, unit1Binding);
}

#endif