/* macOS compatibility shim: redirect GL/gl.h -> OpenGL/gl3.h
 * gl3.h provides the full OpenGL 3.x core profile without legacy cruft.
 * GL_SILENCE_DEPRECATION suppresses Apple's deprecation warnings. */
#pragma once
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>

/* -----------------------------------------------------------------------
 * Legacy OpenGL 1.x/2.x constants removed from Core Profile.
 * The renderer uses these as integer tokens; the enum values are correct.
 * ----------------------------------------------------------------------- */
#ifndef GL_CLAMP
#define GL_CLAMP           0x2900
#endif
#ifndef GL_BGRA
#define GL_BGRA            0x80E1
#endif
#ifndef GL_BGR
#define GL_BGR             0x80E0
#endif
#ifndef GL_RGBA8
#define GL_RGBA8           0x8058
#endif
#ifndef GL_LUMINANCE
#define GL_LUMINANCE       0x1909
#endif
#ifndef GL_LUMINANCE_ALPHA
#define GL_LUMINANCE_ALPHA 0x190A
#endif
#ifndef GL_ALPHA
#define GL_ALPHA           0x1906
#endif
/* Sized/compressed legacy texture formats used by gdraw.c's format table */
#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8   0x8045
#endif
#ifndef GL_LUMINANCE4_ALPHA4
#define GL_LUMINANCE4_ALPHA4   0x8033
#endif
#ifndef GL_INTENSITY8
#define GL_INTENSITY8          0x8050
#endif
#ifndef GL_INTENSITY4
#define GL_INTENSITY4          0x804F
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8          0x8040
#endif
#ifndef GL_LUMINANCE4
#define GL_LUMINANCE4          0x803F
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

/* -----------------------------------------------------------------------
 * ARB/EXT function-pointer type aliases.
 * gdraw.c was written against GL ARB extension names; on macOS Core Profile
 * the types exist under their core (non-ARB) names only. Alias them.
 * ----------------------------------------------------------------------- */
#ifndef PFNGLGENBUFFERSARBPROC
typedef PFNGLGENBUFFERSPROC               PFNGLGENBUFFERSARBPROC;
typedef PFNGLDELETEBUFFERSPROC            PFNGLDELETEBUFFERSARBPROC;
typedef PFNGLBINDBUFFERPROC               PFNGLBINDBUFFERARBPROC;
typedef PFNGLBUFFERDATAPROC               PFNGLBUFFERDATAARBPROC;
typedef PFNGLMAPBUFFERPROC                PFNGLMAPBUFFERARBPROC;
typedef PFNGLUNMAPBUFFERPROC              PFNGLUNMAPBUFFERARBPROC;
typedef PFNGLVERTEXATTRIBPOINTERPROC      PFNGLVERTEXATTRIBPOINTERARBPROC;
typedef PFNGLENABLEVERTEXATTRIBARRAYPROC  PFNGLENABLEVERTEXATTRIBARRAYARBPROC;
typedef PFNGLDISABLEVERTEXATTRIBARRAYPROC PFNGLDISABLEVERTEXATTRIBARRAYARBPROC;
typedef PFNGLCREATESHADERPROC             PFNGLCREATESHADEROBJECTARBPROC;
typedef PFNGLSHADERSOURCEPROC             PFNGLSHADERSOURCEARBPROC;
typedef PFNGLCOMPILESHADERPROC            PFNGLCOMPILESHADERARBPROC;
typedef PFNGLCREATEPROGRAMPROC            PFNGLCREATEPROGRAMOBJECTARBPROC;
typedef PFNGLATTACHSHADERPROC             PFNGLATTACHOBJECTARBPROC;
typedef PFNGLLINKPROGRAMPROC              PFNGLLINKPROGRAMARBPROC;
typedef PFNGLUSEPROGRAMPROC               PFNGLUSEPROGRAMOBJECTARBPROC;
typedef PFNGLGETUNIFORMLOCATIONPROC       PFNGLGETUNIFORMLOCATIONARBPROC;
typedef PFNGLUNIFORM1IPROC                PFNGLUNIFORM1IARBPROC;
typedef PFNGLUNIFORM1FPROC                PFNGLUNIFORM1FARBPROC;
typedef PFNGLUNIFORM2FPROC                PFNGLUNIFORM2FARBPROC;
typedef PFNGLUNIFORM4FPROC                PFNGLUNIFORM4FARBPROC;
typedef PFNGLUNIFORM4FVPROC               PFNGLUNIFORM4FVARBPROC;
typedef PFNGLUNIFORMMATRIX4FVPROC         PFNGLUNIFORMMATRIX4FVARBPROC;
typedef PFNGLGENERATEMIPMAPPROC           PFNGLGENERATEMIPMAPEXTPROC;
typedef PFNGLBLITFRAMEBUFFERPROC          PFNGLBLITFRAMEBUFFEREXTPROC;
typedef PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC;
typedef PFNGLGETSHADERIVPROC              PFNGLGETOBJECTPARAMETERIVARBPROC;
typedef PFNGLGETSHADERINFOLOGPROC         PFNGLGETINFOLOGARBPROC;
typedef PFNGLDELETESHADERPROC             PFNGLDELETEOBJECTARBPROC;
typedef PFNGLGETATTRIBLOCATIONPROC        PFNGLGETATTRIBLOCATIONARBPROC;
typedef PFNGLBINDATTRIBLOCATIONPROC       PFNGLBINDATTRIBLOCATIONARBPROC;
typedef PFNGLGENVERTEXARRAYSPROC          PFNGLGENVERTEXARRAYSARBPROC;
typedef PFNGLBINDVERTEXARRAYPROC          PFNGLBINDVERTEXARRAYARBPROC;
typedef PFNGLDELETEVERTEXARRAYSPROC       PFNGLDELETEVERTEXARRAYSARBPROC;
typedef PFNGLGENFRAMEBUFFERSPROC          PFNGLGENFRAMEBUFFERSEXTPROC;
typedef PFNGLBINDFRAMEBUFFERPROC          PFNGLBINDFRAMEBUFFEREXTPROC;
typedef PFNGLFRAMEBUFFERTEXTURE2DPROC     PFNGLFRAMEBUFFERTEXTURE2DEXTPROC;
typedef PFNGLCHECKFRAMEBUFFERSTATUSPROC   PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC;
typedef PFNGLDELETEFRAMEBUFFERSPROC       PFNGLDELETEFRAMEBUFFERSEXTPROC;
typedef PFNGLGENRENDERBUFFERSPROC         PFNGLGENRENDERBUFFERSEXTPROC;
typedef PFNGLBINDRENDERBUFFERPROC         PFNGLBINDRENDERBUFFEREXTPROC;
typedef PFNGLRENDERBUFFERSTORAGEPROC      PFNGLRENDERBUFFERSTORAGEEXTPROC;
typedef PFNGLFRAMEBUFFERRENDERBUFFERPROC  PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC;
typedef PFNGLDELETERENDERBUFFERSPROC      PFNGLDELETERENDERBUFFERSEXTPROC;
typedef PFNGLACTIVETEXTUREPROC            PFNGLACTIVETEXTUREARBPROC;
/* glMultiTexCoord2f removed from Core Profile – provide pointer type only */
typedef void (*PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);
/* GL_ARB_occlusion_query */
typedef PFNGLGENQUERIESPROC               PFNGLGENQUERIESARBPROC;
typedef PFNGLBEGINQUERYPROC               PFNGLBEGINQUERYARBPROC;
typedef PFNGLENDQUERYPROC                 PFNGLENDQUERYARBPROC;
typedef PFNGLGETQUERYOBJECTUIVPROC        PFNGLGETQUERYOBJECTUIVARBPROC;
#endif /* PFNGLGENBUFFERSARBPROC */

/* -----------------------------------------------------------------------
 * gdraw.c re-declares gl* functions as static function-pointer variables
 * (loaded at runtime via SDL_GL_GetProcAddress). On macOS, OpenGL/gl3.h
 * already declares them as real extern functions, causing "redefinition as
 * a different kind of symbol". We rename the built-in decls out of the way
 * with #define so gdraw.c's static declarations compile without conflict.
 * The static pointers are populated via SDL_GL_GetProcAddress at runtime
 * so functionality is identical.
 *
 * NOTE: This block applies ONLY when compiling gdraw.c — the guard macro
 * GDRAW_ASSERTS is defined at the top of that file and nowhere else.
 * ----------------------------------------------------------------------- */
#ifdef GDRAW_ASSERTS
#define glGenBuffers                     _sys_glGenBuffers
#define glDeleteBuffers                  _sys_glDeleteBuffers
#define glBindBuffer                     _sys_glBindBuffer
#define glBufferData                     _sys_glBufferData
#define glMapBuffer                      _sys_glMapBuffer
#define glUnmapBuffer                    _sys_glUnmapBuffer
#define glVertexAttribPointer            _sys_glVertexAttribPointer
#define glEnableVertexAttribArray        _sys_glEnableVertexAttribArray
#define glDisableVertexAttribArray       _sys_glDisableVertexAttribArray
#define glCreateShader                   _sys_glCreateShader
#define glDeleteShader                   _sys_glDeleteShader
#define glShaderSource                   _sys_glShaderSource
#define glCompileShader                  _sys_glCompileShader
#define glGetShaderiv                    _sys_glGetShaderiv
#define glGetShaderInfoLog               _sys_glGetShaderInfoLog
#define glCreateProgram                  _sys_glCreateProgram
#define glDeleteProgram                  _sys_glDeleteProgram
#define glAttachShader                   _sys_glAttachShader
#define glLinkProgram                    _sys_glLinkProgram
#define glGetUniformLocation             _sys_glGetUniformLocation
#define glUseProgram                     _sys_glUseProgram
#define glGetProgramiv                   _sys_glGetProgramiv
#define glGetProgramInfoLog              _sys_glGetProgramInfoLog
#define glUniform1i                      _sys_glUniform1i
#define glUniform1f                      _sys_glUniform1f
#define glUniform2f                      _sys_glUniform2f
#define glUniform4f                      _sys_glUniform4f
#define glUniform4fv                     _sys_glUniform4fv
#define glBindAttribLocation             _sys_glBindAttribLocation
#define glGetAttribLocation              _sys_glGetAttribLocation
#define glGenRenderbuffers               _sys_glGenRenderbuffers
#define glDeleteRenderbuffers            _sys_glDeleteRenderbuffers
#define glBindRenderbuffer               _sys_glBindRenderbuffer
#define glRenderbufferStorage            _sys_glRenderbufferStorage
#define glGenFramebuffers                _sys_glGenFramebuffers
#define glDeleteFramebuffers             _sys_glDeleteFramebuffers
#define glBindFramebuffer                _sys_glBindFramebuffer
#define glCheckFramebufferStatus         _sys_glCheckFramebufferStatus
#define glFramebufferRenderbuffer        _sys_glFramebufferRenderbuffer
#define glFramebufferTexture2D           _sys_glFramebufferTexture2D
#define glGenerateMipmap                 _sys_glGenerateMipmap
#define glBlitFramebuffer                _sys_glBlitFramebuffer
#define glRenderbufferStorageMultisample _sys_glRenderbufferStorageMultisample
#define glDrawElements                   _sys_glDrawElements
#endif /* GDRAW_ASSERTS */
