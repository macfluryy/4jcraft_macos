#pragma once

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>


#ifndef GL_CLAMP
#define GL_CLAMP                       0x2900
#endif


#ifndef GL_BGR
#define GL_BGR                         0x80E0
#endif
#ifndef GL_BGRA
#define GL_BGRA                        0x80E1
#endif
#ifndef GL_ALPHA
#define GL_ALPHA                       0x1906
#endif
#ifndef GL_LUMINANCE
#define GL_LUMINANCE                   0x1909
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA             0x190A
#endif
#ifndef GL_INTENSITY
#define GL_INTENSITY                   0x8049
#endif


#ifndef GL_RGBA8
#define GL_RGBA8                       0x8058
#endif
#ifndef GL_RGBA4
#define GL_RGBA4                       0x8056
#endif
#ifndef GL_RGB5_A1
#define GL_RGB5_A1                     0x8057
#endif
#ifndef GL_RGB8
#define GL_RGB8                        0x8051
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8                  0x8040
#endif
#ifndef GL_LUMINANCE4
#define GL_LUMINANCE4                  0x803F
#endif
#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8           0x8045
#endif
#ifndef GL_LUMINANCE4_ALPHA4
#define GL_LUMINANCE4_ALPHA4           0x8033
#endif
#ifndef GL_INTENSITY8
#define GL_INTENSITY8                  0x8050
#endif
#ifndef GL_INTENSITY4
#define GL_INTENSITY4                  0x804F
#endif


#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#endif


#ifndef GL_LOGIC_OP
#define GL_LOGIC_OP                    0x0BF1
#endif


#ifndef GL_POLYGON_SMOOTH
#define GL_POLYGON_SMOOTH              0x0B41
#endif
#ifndef GL_LINE_SMOOTH
#define GL_LINE_SMOOTH                 0x0B20
#endif
#ifndef PFNGLGENBUFFERSARBPROC


typedef PFNGLGENBUFFERSPROC                        PFNGLGENBUFFERSARBPROC;
typedef PFNGLDELETEBUFFERSPROC                     PFNGLDELETEBUFFERSARBPROC;
typedef PFNGLBINDBUFFERPROC                        PFNGLBINDBUFFERARBPROC;
typedef PFNGLBUFFERDATAPROC                        PFNGLBUFFERDATAARBPROC;
typedef PFNGLMAPBUFFERPROC                         PFNGLMAPBUFFERARBPROC;
typedef PFNGLUNMAPBUFFERPROC                       PFNGLUNMAPBUFFERARBPROC;


typedef PFNGLVERTEXATTRIBPOINTERPROC               PFNGLVERTEXATTRIBPOINTERARBPROC;
typedef PFNGLENABLEVERTEXATTRIBARRAYPROC            PFNGLENABLEVERTEXATTRIBARRAYARBPROC;
typedef PFNGLDISABLEVERTEXATTRIBARRAYPROC           PFNGLDISABLEVERTEXATTRIBARRAYARBPROC;
typedef PFNGLBINDATTRIBLOCATIONPROC                PFNGLBINDATTRIBLOCATIONARBPROC;
typedef PFNGLGETATTRIBLOCATIONPROC                 PFNGLGETATTRIBLOCATIONARBPROC;


typedef PFNGLCREATESHADERPROC                      PFNGLCREATESHADEROBJECTARBPROC;
typedef PFNGLSHADERSOURCEPROC                      PFNGLSHADERSOURCEARBPROC;
typedef PFNGLCOMPILESHADERPROC                     PFNGLCOMPILESHADERARBPROC;
typedef PFNGLGETSHADERIVPROC                       PFNGLGETOBJECTPARAMETERIVARBPROC;
typedef PFNGLGETSHADERINFOLOGPROC                  PFNGLGETINFOLOGARBPROC;
typedef PFNGLDELETESHADERPROC                      PFNGLDELETEOBJECTARBPROC;
typedef PFNGLCREATEPROGRAMPROC                     PFNGLCREATEPROGRAMOBJECTARBPROC;
typedef PFNGLATTACHSHADERPROC                      PFNGLATTACHOBJECTARBPROC;
typedef PFNGLLINKPROGRAMPROC                       PFNGLLINKPROGRAMARBPROC;
typedef PFNGLUSEPROGRAMPROC                        PFNGLUSEPROGRAMOBJECTARBPROC;
typedef PFNGLGETPROGRAMIVPROC                      PFNGLGETPROGRAMIVARBPROC;
typedef PFNGLGETPROGRAMINFOLOGPROC                 PFNGLGETPROGRAMINFOLOGARBPROC;
typedef PFNGLGETUNIFORMLOCATIONPROC                PFNGLGETUNIFORMLOCATIONARBPROC;


typedef PFNGLUNIFORM1IPROC                         PFNGLUNIFORM1IARBPROC;
typedef PFNGLUNIFORM1FPROC                         PFNGLUNIFORM1FARBPROC;
typedef PFNGLUNIFORM2FPROC                         PFNGLUNIFORM2FARBPROC;
typedef PFNGLUNIFORM3FPROC                         PFNGLUNIFORM3FARBPROC;
typedef PFNGLUNIFORM4FPROC                         PFNGLUNIFORM4FARBPROC;
typedef PFNGLUNIFORM4FVPROC                        PFNGLUNIFORM4FVARBPROC;
typedef PFNGLUNIFORMMATRIX4FVPROC                  PFNGLUNIFORMMATRIX4FVARBPROC;


typedef PFNGLGENVERTEXARRAYSPROC                   PFNGLGENVERTEXARRAYSARBPROC;
typedef PFNGLBINDVERTEXARRAYPROC                   PFNGLBINDVERTEXARRAYARBPROC;
typedef PFNGLDELETEVERTEXARRAYSPROC                PFNGLDELETEVERTEXARRAYSARBPROC;


typedef PFNGLGENFRAMEBUFFERSPROC                   PFNGLGENFRAMEBUFFERSEXTPROC;
typedef PFNGLBINDFRAMEBUFFERPROC                   PFNGLBINDFRAMEBUFFEREXTPROC;
typedef PFNGLDELETEFRAMEBUFFERSPROC                PFNGLDELETEFRAMEBUFFERSEXTPROC;
typedef PFNGLCHECKFRAMEBUFFERSTATUSPROC            PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC;
typedef PFNGLFRAMEBUFFERTEXTURE2DPROC              PFNGLFRAMEBUFFERTEXTURE2DEXTPROC;
typedef PFNGLFRAMEBUFFERRENDERBUFFERPROC           PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC;


typedef PFNGLGENRENDERBUFFERSPROC                  PFNGLGENRENDERBUFFERSEXTPROC;
typedef PFNGLBINDRENDERBUFFERPROC                  PFNGLBINDRENDERBUFFEREXTPROC;
typedef PFNGLDELETERENDERBUFFERSPROC               PFNGLDELETERENDERBUFFERSEXTPROC;
typedef PFNGLRENDERBUFFERSTORAGEPROC               PFNGLRENDERBUFFERSTORAGEEXTPROC;
typedef PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC;


typedef PFNGLBLITFRAMEBUFFERPROC                   PFNGLBLITFRAMEBUFFEREXTPROC;
typedef PFNGLGENERATEMIPMAPPROC                    PFNGLGENERATEMIPMAPEXTPROC;


typedef PFNGLACTIVETEXTUREPROC                     PFNGLACTIVETEXTUREARBPROC;

typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);


typedef PFNGLGENQUERIESPROC                        PFNGLGENQUERIESARBPROC;
typedef PFNGLDELETEQUERIESPROC                     PFNGLDELETEQUERIESARBPROC;
typedef PFNGLBEGINQUERYPROC                        PFNGLBEGINQUERYARBPROC;
typedef PFNGLENDQUERYPROC                          PFNGLENDQUERYARBPROC;
typedef PFNGLGETQUERYOBJECTUIVPROC                 PFNGLGETQUERYOBJECTUIVARBPROC;
typedef PFNGLGETQUERYOBJECTIVPROC                  PFNGLGETQUERYOBJECTIVARBPROC;


typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGIPROC_)(GLenum name, GLuint index);

#endif 
#ifdef GDRAW_ASSERTS


#define glGenBuffers                      _sys_glGenBuffers
#define glDeleteBuffers                   _sys_glDeleteBuffers
#define glBindBuffer                      _sys_glBindBuffer
#define glBufferData                      _sys_glBufferData
#define glMapBuffer                       _sys_glMapBuffer
#define glUnmapBuffer                     _sys_glUnmapBuffer


#define glVertexAttribPointer             _sys_glVertexAttribPointer
#define glEnableVertexAttribArray         _sys_glEnableVertexAttribArray
#define glDisableVertexAttribArray        _sys_glDisableVertexAttribArray
#define glBindAttribLocation              _sys_glBindAttribLocation
#define glGetAttribLocation               _sys_glGetAttribLocation


#define glCreateShader                    _sys_glCreateShader
#define glDeleteShader                    _sys_glDeleteShader
#define glShaderSource                    _sys_glShaderSource
#define glCompileShader                   _sys_glCompileShader
#define glGetShaderiv                     _sys_glGetShaderiv
#define glGetShaderInfoLog                _sys_glGetShaderInfoLog


#define glCreateProgram                   _sys_glCreateProgram
#define glDeleteProgram                   _sys_glDeleteProgram
#define glAttachShader                    _sys_glAttachShader
#define glLinkProgram                     _sys_glLinkProgram
#define glUseProgram                      _sys_glUseProgram
#define glGetProgramiv                    _sys_glGetProgramiv
#define glGetProgramInfoLog               _sys_glGetProgramInfoLog
#define glGetUniformLocation              _sys_glGetUniformLocation


#define glUniform1i                       _sys_glUniform1i
#define glUniform1f                       _sys_glUniform1f
#define glUniform2f                       _sys_glUniform2f
#define glUniform3f                       _sys_glUniform3f
#define glUniform4f                       _sys_glUniform4f
#define glUniform4fv                      _sys_glUniform4fv
#define glUniformMatrix4fv                _sys_glUniformMatrix4fv


#define glGenVertexArrays                 _sys_glGenVertexArrays
#define glBindVertexArray                 _sys_glBindVertexArray
#define glDeleteVertexArrays              _sys_glDeleteVertexArrays


#define glGenFramebuffers                 _sys_glGenFramebuffers
#define glDeleteFramebuffers              _sys_glDeleteFramebuffers
#define glBindFramebuffer                 _sys_glBindFramebuffer
#define glCheckFramebufferStatus          _sys_glCheckFramebufferStatus
#define glFramebufferTexture2D            _sys_glFramebufferTexture2D
#define glFramebufferRenderbuffer         _sys_glFramebufferRenderbuffer


#define glGenRenderbuffers                _sys_glGenRenderbuffers
#define glDeleteRenderbuffers             _sys_glDeleteRenderbuffers
#define glBindRenderbuffer                _sys_glBindRenderbuffer
#define glRenderbufferStorage             _sys_glRenderbufferStorage
#define glRenderbufferStorageMultisample  _sys_glRenderbufferStorageMultisample


#define glGenerateMipmap                  _sys_glGenerateMipmap
#define glBlitFramebuffer                 _sys_glBlitFramebuffer
#define glDrawElements                    _sys_glDrawElements


#define glGenQueries                      _sys_glGenQueries
#define glDeleteQueries                   _sys_glDeleteQueries
#define glBeginQuery                      _sys_glBeginQuery
#define glEndQuery                        _sys_glEndQuery
#define glGetQueryObjectuiv               _sys_glGetQueryObjectuiv
#define glGetQueryObjectiv                _sys_glGetQueryObjectiv

#endif 