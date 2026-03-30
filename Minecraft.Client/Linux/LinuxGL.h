#pragma once

#ifdef __linux__

#include <GL/gl.h>
#include <GL/glu.h>

class FloatBuffer;
class IntBuffer;
class ByteBuffer;

void glGenTextures(IntBuffer*);
int glGenTextures();
void glDeleteTextures(IntBuffer*);
void glLight(int, int, FloatBuffer*);
void glLightModel(int, FloatBuffer*);
void glGetFloat(int a, FloatBuffer* b);
void glTexCoordPointer(int, int, int, int);
void glTexCoordPointer(int, int, FloatBuffer*);
void glNormalPointer(int, int, int);
void glNormalPointer(int, ByteBuffer*);
void glColorPointer(int, bool, int, ByteBuffer*);
void glColorPointer(int, int, int, int);
void glVertexPointer(int, int, int, int);
void glVertexPointer(int, int, FloatBuffer*);
void glNewList(int, int);
void glEndList(int vertexCount = 0);
void glTexImage2D(int, int, int, int, int, int, int, int, ByteBuffer*);
void glCallLists(IntBuffer*);
void glGenQueriesARB(IntBuffer*);
void glBeginQueryARB(int, int);
void glEndQueryARB(int);
void glGetQueryObjectuARB(int, int, IntBuffer*);
void glFog(int, FloatBuffer*);
void glTexGen(int, int, FloatBuffer*);
void glReadPixels(int, int, int, int, int, int, ByteBuffer*);
void glTexGeni(int, int, int);
void glMultiTexCoord2f(int, float, float);
void glClientActiveTexture(int);
void glActiveTexture(int);

#endif
