#ifdef __linux__

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <dlfcn.h>

#include "../../Minecraft.World/IO/Streams/IntBuffer.h"
#include "../../Minecraft.World/IO/Streams/FloatBuffer.h"
#include "../../Minecraft.World/IO/Streams/ByteBuffer.h"


int glGenTextures()
{
	GLuint id = 0;
	::glGenTextures(1, &id);
	return (int)id;
}

void glGenTextures(IntBuffer *buf)
{
	GLuint id = 0;
	::glGenTextures(1, &id);
	buf->put((int)id);
	buf->flip();
}

void glDeleteTextures(int id)
{
	GLuint uid = (GLuint)id;
	::glDeleteTextures(1, &uid);
}

void glDeleteTextures(IntBuffer *buf)
{
	int id = buf->get(0);
	GLuint uid = (GLuint)id;
	::glDeleteTextures(1, &uid);
}

void glLight(int light, int pname, FloatBuffer *params)
{
	::glLightfv((GLenum)light, (GLenum)pname, params->_getDataPointer());
}

void glLightModel(int pname, FloatBuffer *params)
{
	::glLightModelfv((GLenum)pname, params->_getDataPointer());
}

void glGetFloat(int pname, FloatBuffer *params)
{
	::glGetFloatv((GLenum)pname, params->_getDataPointer());
}

void glTexGen(int coord, int pname, FloatBuffer *params)
{
	::glTexGenfv((GLenum)coord, (GLenum)pname, params->_getDataPointer());
}

void glFog(int pname, FloatBuffer *params)
{
	::glFogfv((GLenum)pname, params->_getDataPointer());
}

void glTexCoordPointer(int size, int type, FloatBuffer *pointer)
{
	::glTexCoordPointer(size, (GLenum)type, 0, pointer->_getDataPointer());
}

void glNormalPointer(int type, ByteBuffer *pointer)
{
	::glNormalPointer((GLenum)type, 0, pointer->getBuffer());
}

void glColorPointer(int size, bool normalized, int stride, ByteBuffer *pointer)
{
	(void)normalized;
	::glColorPointer(size, GL_UNSIGNED_BYTE, stride, pointer->getBuffer());
}

void glVertexPointer(int size, int type, FloatBuffer *pointer)
{
	::glVertexPointer(size, (GLenum)type, 0, pointer->_getDataPointer());
}

void glEndList(int)
{
	::glEndList();
}

void glTexImage2D(int target, int level, int internalformat, int width, int height, int border, int format, int type, ByteBuffer *pixels)
{
	void *data = pixels ? pixels->getBuffer() : nullptr;
	::glTexImage2D((GLenum)target, level, internalformat, width, height, border, (GLenum)format, (GLenum)type, data);
}

void glCallLists(IntBuffer *lists)
{
	int count = lists->limit() - lists->position();
	::glCallLists(count, GL_INT, lists->getBuffer());
}

static PFNGLGENQUERIESARBPROC _glGenQueriesARB = nullptr;
static PFNGLBEGINQUERYARBPROC _glBeginQueryARB = nullptr;
static PFNGLENDQUERYARBPROC _glEndQueryARB = nullptr;
static PFNGLGETQUERYOBJECTUIVARBPROC _glGetQueryObjectuivARB = nullptr;
static bool _queriesInitialized = false;

static void initQueryFuncs()
{
	if (_queriesInitialized) return;
	_queriesInitialized = true;
	_glGenQueriesARB = (PFNGLGENQUERIESARBPROC)dlsym(RTLD_DEFAULT, "glGenQueriesARB");
	_glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)dlsym(RTLD_DEFAULT, "glBeginQueryARB");
	_glEndQueryARB = (PFNGLENDQUERYARBPROC)dlsym(RTLD_DEFAULT, "glEndQueryARB");
	_glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)dlsym(RTLD_DEFAULT, "glGetQueryObjectuivARB");
}

void glGenQueriesARB(IntBuffer *buf)
{
	initQueryFuncs();
	if (_glGenQueriesARB)
	{
		GLuint id = 0;
		_glGenQueriesARB(1, &id);
		buf->put((int)id);
		buf->flip();
	}
}

void glBeginQueryARB(int target, int id)
{
	initQueryFuncs();
	if (_glBeginQueryARB) _glBeginQueryARB((GLenum)target, (GLuint)id);
}

void glEndQueryARB(int target)
{
	initQueryFuncs();
	if (_glEndQueryARB) _glEndQueryARB((GLenum)target);
}

void glGetQueryObjectuARB(int id, int pname, IntBuffer *params)
{
	initQueryFuncs();
	if (_glGetQueryObjectuivARB)
	{
		GLuint val = 0;
		_glGetQueryObjectuivARB((GLuint)id, (GLenum)pname, &val);
		params->put((int)val);
		params->flip();
	}
}

void glReadPixels(int x, int y, int width, int height, int format, int type, ByteBuffer *pixels)
{
	::glReadPixels(x, y, width, height, (GLenum)format, (GLenum)type, pixels->getBuffer());
}

#endif
