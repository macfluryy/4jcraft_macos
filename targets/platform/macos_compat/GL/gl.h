#pragma once
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#ifdef USE_METAL
// Metal backend: provide minimal typedefs instead of pulling in Apple's
// OpenGL framework headers (which would conflict with Render.h macros).
#include <stddef.h>
typedef unsigned int   GLenum;
typedef unsigned int   GLuint;
typedef int            GLint;
typedef unsigned char  GLboolean;
typedef int            GLsizei;
typedef float          GLfloat;
typedef double         GLdouble;
typedef unsigned int   GLbitfield;
typedef char           GLchar;
typedef ptrdiff_t      GLsizeiptr;
typedef ptrdiff_t      GLintptr;
#ifndef GL_TRUE
#define GL_TRUE 1
#endif
#ifndef GL_FALSE
#define GL_FALSE 0
#endif

/* Legacy enum values used by game code as arguments to the macro-wrapped
 * glEnable/glDisable/glFog/glLight macros in Render.h. In Metal mode these
 * macros turn them into no-ops, but the enum numeric values must still exist. */
#define GL_VERSION             0x1F02
#define GL_COLOR_MATERIAL      0x0B57
#define GL_NORMALIZE           0x0BA1
#define GL_RESCALE_NORMAL      0x803A
#define GL_TEXTURE_GEN_S       0x0C60
#define GL_TEXTURE_GEN_T       0x0C61
#define GL_TEXTURE_GEN_R       0x0C62
#define GL_TEXTURE_GEN_Q       0x0C63
#define GL_FOG                 0x0B60
#define GL_FOG_MODE            0x0B65
#define GL_FOG_START           0x0B63
#define GL_FOG_END             0x0B64
#define GL_FOG_DENSITY         0x0B62
#define GL_FOG_COLOR           0x0B66
#define GL_LIGHTING            0x0B50
#define GL_LIGHT0              0x4000
#define GL_LIGHT1              0x4001
#define GL_LIGHT_MODEL_AMBIENT 0x0B53
#define GL_POSITION            0x1203
#define GL_AMBIENT             0x1200
#define GL_DIFFUSE             0x1201
#define GL_SPECULAR            0x1202
#define GL_ALPHA_TEST          0x0BC0
#define GL_TEXTURE_2D          0x0DE1
#define GL_BLEND               0x0BE2
#define GL_CULL_FACE           0x0B44
#define GL_DEPTH_TEST          0x0B71
#define GL_TEXTURE0            0x84C0
#define GL_TEXTURE1            0x84C1
#define GL_FLAT                0x1D00
#define GL_SMOOTH              0x1D01
#define GL_LINEAR              0x2601
#define GL_NEAREST             0x2600
#define GL_EXP                 0x0800
#define GL_EXP2                0x0801
#define GL_MODELVIEW           0x1700
#define GL_PROJECTION          0x1701
#define GL_TEXTURE             0x1702
#define GL_MODELVIEW_MATRIX    0x0BA6
#define GL_PROJECTION_MATRIX   0x0BA7
#define GL_TEXTURE_MATRIX      0x0BA8
#define GL_FRONT               0x0404
#define GL_BACK                0x0405
#define GL_FRONT_AND_BACK      0x0408
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#define GL_LEQUAL              0x0203
#define GL_SRC_ALPHA           0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE                 1
#define GL_ZERO                0
#define GL_DEPTH_BUFFER_BIT    0x00000100
#define GL_COLOR_BUFFER_BIT    0x00004000
#define GL_STENCIL_BUFFER_BIT  0x00000400
#define GL_QUADS               0x0007
#define GL_TRIANGLES           0x0004
#define GL_TRIANGLE_STRIP      0x0005
#define GL_TRIANGLE_FAN        0x0006
#define GL_LINES               0x0001
#define GL_LINE_STRIP          0x0003
#define GL_RGBA                0x1908
#define GL_RGB                 0x1907
#define GL_UNSIGNED_BYTE       0x1401
#define GL_FLOAT               0x1406
#define GL_BYTE                0x1400
#define GL_SHORT               0x1402
#define GL_UNSIGNED_SHORT      0x1403
#define GL_INT                 0x1404
#define GL_UNSIGNED_INT        0x1405
#define GL_TEXTURE_MIN_FILTER  0x2801
#define GL_TEXTURE_MAG_FILTER  0x2800
#define GL_TEXTURE_WRAP_S      0x2802
#define GL_TEXTURE_WRAP_T      0x2803
#define GL_REPEAT              0x2901
#define GL_CLAMP_TO_EDGE       0x812F
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR  0x2703
#define GL_TEXTURE_MAX_LEVEL   0x813D
#define GL_ALWAYS              0x0207
#define GL_LESS                0x0201
#define GL_EQUAL               0x0202
#define GL_GREATER             0x0204
#define GL_GEQUAL              0x0206
#define GL_KEEP                0x1E00
#define GL_REPLACE             0x1E01
#define GL_SAMPLES_PASSED      0x8914
#define GL_QUERY_RESULT        0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_CCW                 0x0901
#define GL_CW                  0x0900
#define GL_VIEWPORT            0x0BA2
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_STREAM_DRAW         0x88E0
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_POLYGON_OFFSET_LINE 0x2A02
#define GL_POLYGON_OFFSET_POINT 0x2A01
#define GL_STENCIL_TEST        0x0B90
#define GL_TEXTURE_ENV         0x2300
#define GL_TEXTURE_ENV_MODE    0x2200
#define GL_MODULATE            0x2100
#define GL_DECAL               0x2101
#define GL_REPLACE_EXT         0x8062
#define GL_OBJECT_LINEAR       0x2401
#define GL_EYE_LINEAR          0x2400
#define GL_OBJECT_PLANE        0x2501
#define GL_EYE_PLANE           0x2502
#define GL_SPHERE_MAP          0x2402
#define GL_TEXTURE_GEN_MODE    0x2500
#define GL_S                   0x2000
#define GL_T                   0x2001
#define GL_R                   0x2002
#define GL_Q                   0x2003
#define GL_SCISSOR_TEST        0x0C11
#define GL_BLEND_COLOR         0x8005
#define GL_DST_ALPHA           0x0304
#define GL_SRC_COLOR           0x0300
#define GL_DST_COLOR           0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_CONSTANT_ALPHA      0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_CURRENT_COLOR       0x0B00

/* Inline wrappers for direct GL calls in game code. In Metal mode they
 * forward to RenderManager or are no-ops. These are deliberately minimal —
 * the game code expects these symbols, not Metal calls. */
#ifdef __cplusplus
extern "C" {
#endif
static inline void glBlendFunc(GLenum, GLenum)         {}
static inline void glDepthFunc(GLenum)                  {}
static inline void glDepthMask(GLboolean)               {}
static inline void glTexParameteri(GLenum, GLenum, GLint) {}
static inline void glTexParameterf(GLenum, GLenum, GLfloat) {}
static inline void glBindTexture(GLenum, GLuint)        {}
static inline void glShadeModel(GLenum)                 {}
static inline void glColorMaterial(GLenum, GLenum)      {}
static inline void glNormal3f(GLfloat, GLfloat, GLfloat) {}
static inline void glViewport(GLint, GLint, GLsizei, GLsizei) {}
static inline void glCullFace(GLenum)                   {}
static inline void glFrontFace(GLenum)                  {}
static inline void glPolygonOffset(GLfloat, GLfloat)    {}
static inline void glLineWidth(GLfloat)                 {}
static inline void glClearDepth(GLdouble)               {}
static inline void glClear(GLbitfield)                  {}
static inline void glClearColor(GLfloat, GLfloat, GLfloat, GLfloat) {}
static inline void glColorMask(GLboolean, GLboolean, GLboolean, GLboolean) {}
static inline void glStencilFunc(GLenum, GLint, GLuint) {}
static inline void glStencilMask(GLuint)                {}
static inline void glStencilOp(GLenum, GLenum, GLenum)  {}
static inline void glFlush(void)                        {}
static inline void glFinish(void)                       {}
static inline void glHint(GLenum, GLenum)               {}
static inline void glPixelStorei(GLenum, GLint)         {}
static inline const unsigned char* glGetString(GLenum)  { return (const unsigned char*)""; }
static inline void glGetIntegerv(GLenum, GLint* v)      { if (v) *v = 0; }
static inline void glGetFloatv(GLenum, GLfloat* v)      { if (v) *v = 0.f; }
static inline GLenum glGetError(void)                   { return 0; }
static inline void glTexGeni(GLenum, GLenum, GLint)     {}
static inline void glTexGenfv(GLenum, GLenum, const GLfloat*) {}
static inline void glBlendColor(GLfloat, GLfloat, GLfloat, GLfloat) {}
static inline void glScissor(GLint, GLint, GLsizei, GLsizei) {}
static inline void glReadBuffer(GLenum)                 {}
static inline void glDrawBuffer(GLenum)                 {}
static inline void glPushAttrib(GLbitfield)             {}
static inline void glPopAttrib(void)                    {}
static inline void glPushClientAttrib(GLbitfield)       {}
static inline void glPopClientAttrib(void)              {}
static inline void glMaterialfv(GLenum, GLenum, const GLfloat*) {}
static inline void glMateriali(GLenum, GLenum, GLint)   {}
#ifdef __cplusplus
}
#endif
#else
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#ifndef GL_CLAMP
#define GL_CLAMP 0x2900
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif
#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif
#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif
#ifndef GL_LUMINANCE
#define GL_LUMINANCE 0x1909
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA 0x190A
#endif
#ifndef GL_ALPHA
#define GL_ALPHA 0x1906
#endif
#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8 0x8045
#endif
#ifndef GL_LUMINANCE4_ALPHA4
#define GL_LUMINANCE4_ALPHA4 0x8033
#endif
#ifndef GL_INTENSITY8
#define GL_INTENSITY8 0x8050
#endif
#ifndef GL_INTENSITY4
#define GL_INTENSITY4 0x804F
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8 0x8040
#endif
#ifndef GL_LUMINANCE4
#define GL_LUMINANCE4 0x803F
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

#ifndef USE_METAL
#ifndef PFNGLGENBUFFERSARBPROC
typedef PFNGLGENBUFFERSPROC PFNGLGENBUFFERSARBPROC;
typedef PFNGLDELETEBUFFERSPROC PFNGLDELETEBUFFERSARBPROC;
typedef PFNGLBINDBUFFERPROC PFNGLBINDBUFFERARBPROC;
typedef PFNGLBUFFERDATAPROC PFNGLBUFFERDATAARBPROC;
typedef PFNGLMAPBUFFERPROC PFNGLMAPBUFFERARBPROC;
typedef PFNGLUNMAPBUFFERPROC PFNGLUNMAPBUFFERARBPROC;
typedef PFNGLVERTEXATTRIBPOINTERPROC PFNGLVERTEXATTRIBPOINTERARBPROC;
typedef PFNGLENABLEVERTEXATTRIBARRAYPROC PFNGLENABLEVERTEXATTRIBARRAYARBPROC;
typedef PFNGLDISABLEVERTEXATTRIBARRAYPROC PFNGLDISABLEVERTEXATTRIBARRAYARBPROC;
typedef PFNGLCREATESHADERPROC PFNGLCREATESHADEROBJECTARBPROC;
typedef PFNGLSHADERSOURCEPROC PFNGLSHADERSOURCEARBPROC;
typedef PFNGLCOMPILESHADERPROC PFNGLCOMPILESHADERARBPROC;
typedef PFNGLCREATEPROGRAMPROC PFNGLCREATEPROGRAMOBJECTARBPROC;
typedef PFNGLATTACHSHADERPROC PFNGLATTACHOBJECTARBPROC;
typedef PFNGLLINKPROGRAMPROC PFNGLLINKPROGRAMARBPROC;
typedef PFNGLUSEPROGRAMPROC PFNGLUSEPROGRAMOBJECTARBPROC;
typedef PFNGLGETUNIFORMLOCATIONPROC PFNGLGETUNIFORMLOCATIONARBPROC;
typedef PFNGLUNIFORM1IPROC PFNGLUNIFORM1IARBPROC;
typedef PFNGLUNIFORM1FPROC PFNGLUNIFORM1FARBPROC;
typedef PFNGLUNIFORM2FPROC PFNGLUNIFORM2FARBPROC;
typedef PFNGLUNIFORM4FPROC PFNGLUNIFORM4FARBPROC;
typedef PFNGLUNIFORM4FVPROC PFNGLUNIFORM4FVARBPROC;
typedef PFNGLUNIFORMMATRIX4FVPROC PFNGLUNIFORMMATRIX4FVARBPROC;
typedef PFNGLGENERATEMIPMAPPROC PFNGLGENERATEMIPMAPEXTPROC;
typedef PFNGLBLITFRAMEBUFFERPROC PFNGLBLITFRAMEBUFFEREXTPROC;
typedef PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC;
typedef PFNGLGETSHADERIVPROC PFNGLGETOBJECTPARAMETERIVARBPROC;
typedef PFNGLGETSHADERINFOLOGPROC PFNGLGETINFOLOGARBPROC;
typedef PFNGLDELETESHADERPROC PFNGLDELETEOBJECTARBPROC;
typedef PFNGLGETATTRIBLOCATIONPROC PFNGLGETATTRIBLOCATIONARBPROC;
typedef PFNGLBINDATTRIBLOCATIONPROC PFNGLBINDATTRIBLOCATIONARBPROC;
typedef PFNGLGENVERTEXARRAYSPROC PFNGLGENVERTEXARRAYSARBPROC;
typedef PFNGLBINDVERTEXARRAYPROC PFNGLBINDVERTEXARRAYARBPROC;
typedef PFNGLDELETEVERTEXARRAYSPROC PFNGLDELETEVERTEXARRAYSARBPROC;
typedef PFNGLGENFRAMEBUFFERSPROC PFNGLGENFRAMEBUFFERSEXTPROC;
typedef PFNGLBINDFRAMEBUFFERPROC PFNGLBINDFRAMEBUFFEREXTPROC;
typedef PFNGLFRAMEBUFFERTEXTURE2DPROC PFNGLFRAMEBUFFERTEXTURE2DEXTPROC;
typedef PFNGLCHECKFRAMEBUFFERSTATUSPROC PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC;
typedef PFNGLDELETEFRAMEBUFFERSPROC PFNGLDELETEFRAMEBUFFERSEXTPROC;
typedef PFNGLGENRENDERBUFFERSPROC PFNGLGENRENDERBUFFERSEXTPROC;
typedef PFNGLBINDRENDERBUFFERPROC PFNGLBINDRENDERBUFFEREXTPROC;
typedef PFNGLRENDERBUFFERSTORAGEPROC PFNGLRENDERBUFFERSTORAGEEXTPROC;
typedef PFNGLFRAMEBUFFERRENDERBUFFERPROC PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC;
typedef PFNGLDELETERENDERBUFFERSPROC PFNGLDELETERENDERBUFFERSEXTPROC;
typedef PFNGLACTIVETEXTUREPROC PFNGLACTIVETEXTUREARBPROC;
typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s,
                                            GLfloat t);
/* GL_ARB_occlusion_query */
typedef PFNGLGENQUERIESPROC PFNGLGENQUERIESARBPROC;
typedef PFNGLBEGINQUERYPROC PFNGLBEGINQUERYARBPROC;
typedef PFNGLENDQUERYPROC PFNGLENDQUERYARBPROC;
typedef PFNGLGETQUERYOBJECTUIVPROC PFNGLGETQUERYOBJECTUIVARBPROC;
#endif /* PFNGLGENBUFFERSARBPROC */
#endif /* !USE_METAL */

#ifdef GDRAW_ASSERTS
#define glGenBuffers _sys_glGenBuffers
#define glDeleteBuffers _sys_glDeleteBuffers
#define glBindBuffer _sys_glBindBuffer
#define glBufferData _sys_glBufferData
#define glMapBuffer _sys_glMapBuffer
#define glUnmapBuffer _sys_glUnmapBuffer
#define glVertexAttribPointer _sys_glVertexAttribPointer
#define glEnableVertexAttribArray _sys_glEnableVertexAttribArray
#define glDisableVertexAttribArray _sys_glDisableVertexAttribArray
#define glCreateShader _sys_glCreateShader
#define glDeleteShader _sys_glDeleteShader
#define glShaderSource _sys_glShaderSource
#define glCompileShader _sys_glCompileShader
#define glGetShaderiv _sys_glGetShaderiv
#define glGetShaderInfoLog _sys_glGetShaderInfoLog
#define glCreateProgram _sys_glCreateProgram
#define glDeleteProgram _sys_glDeleteProgram
#define glAttachShader _sys_glAttachShader
#define glLinkProgram _sys_glLinkProgram
#define glGetUniformLocation _sys_glGetUniformLocation
#define glUseProgram _sys_glUseProgram
#define glGetProgramiv _sys_glGetProgramiv
#define glGetProgramInfoLog _sys_glGetProgramInfoLog
#define glUniform1i _sys_glUniform1i
#define glUniform1f _sys_glUniform1f
#define glUniform2f _sys_glUniform2f
#define glUniform4f _sys_glUniform4f
#define glUniform4fv _sys_glUniform4fv
#define glBindAttribLocation _sys_glBindAttribLocation
#define glGetAttribLocation _sys_glGetAttribLocation
#define glGenRenderbuffers _sys_glGenRenderbuffers
#define glDeleteRenderbuffers _sys_glDeleteRenderbuffers
#define glBindRenderbuffer _sys_glBindRenderbuffer
#define glRenderbufferStorage _sys_glRenderbufferStorage
#define glGenFramebuffers _sys_glGenFramebuffers
#define glDeleteFramebuffers _sys_glDeleteFramebuffers
#define glBindFramebuffer _sys_glBindFramebuffer
#define glCheckFramebufferStatus _sys_glCheckFramebufferStatus
#define glFramebufferRenderbuffer _sys_glFramebufferRenderbuffer
#define glFramebufferTexture2D _sys_glFramebufferTexture2D
#define glGenerateMipmap _sys_glGenerateMipmap
#define glBlitFramebuffer _sys_glBlitFramebuffer
#define glRenderbufferStorageMultisample _sys_glRenderbufferStorageMultisample
#define glDrawElements _sys_glDrawElements
#endif /* GDRAW_ASSERTS */