// gdraw_glfw.c - Linux/GLFW port of gdraw_wgl.c
// Original: copyright 2011-2012 RAD Game Tools
// Port: Implements the Iggy graphics driver layer for GL on Linux via GLFW.

#define GDRAW_ASSERTS

#include "../../../Windows64/Iggy/include/iggy.h"
#include "../../../Windows64/Iggy/include/gdraw.h"
#include "gdraw_glfw.h"

#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#define true 1
#define false 0

// Iggy GDraw support functions - normally in the Iggy library, stubbed here
void * IggyGDrawMallocAnnotated(SINTa size, const char *file, int line) {
   (void)file; (void)line;
   return malloc((size_t)size);
}

void IggyGDrawFree(void *ptr) {
   free(ptr);
}

void IggyGDrawSendWarning(Iggy *f, char const *message, ...) {
   (void)f;
   va_list args;
   va_start(args, message);
   fprintf(stderr, "[Iggy GDraw Warning] ");
   vfprintf(stderr, message, args);
   fprintf(stderr, "\n");
   va_end(args);
}

void IggyDiscardVertexBufferCallback(void *owner, void *buf) {
   (void)owner; (void)buf;
}

///////////////////////////////////////////////////////////////////////////////
//
//  Extensions (we map to GL 2.0 function names for a uniform interface
//  across platforms)
//
//  NOTE: glActiveTexture and glCompressedTexImage2D are omitted here because
//  on Linux they are core GL 1.3+ functions already declared in <GL/gl.h>.
//  The shared code calls them by name and the real functions are used directly.
//

#define GDRAW_GL_EXTENSION_LIST \
   /*  identifier                      import                              procname */ \
   /* GL_ARB_vertex_buffer_object */ \
   GLE(GenBuffers,                     "GenBuffersARB",                    GENBUFFERSARB) \
   GLE(DeleteBuffers,                  "DeleteBuffersARB",                 DELETEBUFFERSARB) \
   GLE(BindBuffer,                     "BindBufferARB",                    BINDBUFFERARB) \
   GLE(BufferData,                     "BufferDataARB",                    BUFFERDATAARB) \
   GLE(MapBuffer,                      "MapBufferARB",                     MAPBUFFERARB) \
   GLE(UnmapBuffer,                    "UnmapBufferARB",                   UNMAPBUFFERARB) \
   GLE(VertexAttribPointer,            "VertexAttribPointerARB",           VERTEXATTRIBPOINTERARB) \
   GLE(EnableVertexAttribArray,        "EnableVertexAttribArrayARB",       ENABLEVERTEXATTRIBARRAYARB) \
   GLE(DisableVertexAttribArray,       "DisableVertexAttribArrayARB",      DISABLEVERTEXATTRIBARRAYARB) \
   /* GL_ARB_shader_objects */ \
   GLE(CreateShader,                   "CreateShaderObjectARB",            CREATESHADEROBJECTARB) \
   GLE(DeleteShader,                   "DeleteObjectARB",                  DELETEOBJECTARB) \
   GLE(ShaderSource,                   "ShaderSourceARB",                  SHADERSOURCEARB) \
   GLE(CompileShader,                  "CompileShaderARB",                 COMPILESHADERARB) \
   GLE(GetShaderiv,                    "GetObjectParameterivARB",          GETOBJECTPARAMETERIVARB) \
   GLE(GetShaderInfoLog,               "GetInfoLogARB",                    GETINFOLOGARB) \
   GLE(CreateProgram,                  "CreateProgramObjectARB",           CREATEPROGRAMOBJECTARB) \
   GLE(DeleteProgram,                  "DeleteObjectARB",                  DELETEOBJECTARB) \
   GLE(AttachShader,                   "AttachObjectARB",                  ATTACHOBJECTARB) \
   GLE(LinkProgram,                    "LinkProgramARB",                   LINKPROGRAMARB) \
   GLE(GetUniformLocation,             "GetUniformLocationARB",            GETUNIFORMLOCATIONARB) \
   GLE(UseProgram,                     "UseProgramObjectARB",              USEPROGRAMOBJECTARB) \
   GLE(GetProgramiv,                   "GetObjectParameterivARB",          GETOBJECTPARAMETERIVARB) \
   GLE(GetProgramInfoLog,              "GetInfoLogARB",                    GETINFOLOGARB) \
   GLE(Uniform1i,                      "Uniform1iARB",                     UNIFORM1IARB) \
   GLE(Uniform4f,                      "Uniform4fARB",                     UNIFORM4FARB) \
   GLE(Uniform4fv,                     "Uniform4fvARB",                    UNIFORM4FVARB) \
   /* GL_ARB_vertex_shader */ \
   GLE(BindAttribLocation,             "BindAttribLocationARB",            BINDATTRIBLOCATIONARB) \
   /* Missing from WGL but needed by shared code */ \
   GLE(Uniform1f,                      "Uniform1fARB",                     UNIFORM1FARB) \
   /* GL_EXT_framebuffer_object */ \
   GLE(GenRenderbuffers,               "GenRenderbuffersEXT",              GENRENDERBUFFERSEXT) \
   GLE(DeleteRenderbuffers,            "DeleteRenderbuffersEXT",           DELETERENDERBUFFERSEXT) \
   GLE(BindRenderbuffer,               "BindRenderbufferEXT",              BINDRENDERBUFFEREXT) \
   GLE(RenderbufferStorage,            "RenderbufferStorageEXT",           RENDERBUFFERSTORAGEEXT) \
   GLE(GenFramebuffers,                "GenFramebuffersEXT",               GENFRAMEBUFFERSEXT) \
   GLE(DeleteFramebuffers,             "DeleteFramebuffersEXT",            DELETEFRAMEBUFFERSEXT) \
   GLE(BindFramebuffer,                "BindFramebufferEXT",               BINDFRAMEBUFFEREXT) \
   GLE(CheckFramebufferStatus,         "CheckFramebufferStatusEXT",        CHECKFRAMEBUFFERSTATUSEXT) \
   GLE(FramebufferRenderbuffer,        "FramebufferRenderbufferEXT",       FRAMEBUFFERRENDERBUFFEREXT) \
   GLE(FramebufferTexture2D,           "FramebufferTexture2DEXT",          FRAMEBUFFERTEXTURE2DEXT) \
   GLE(GenerateMipmap,                 "GenerateMipmapEXT",                GENERATEMIPMAPEXT) \
   /* GL_EXT_framebuffer_blit */ \
   GLE(BlitFramebuffer,                "BlitFramebufferEXT",               BLITFRAMEBUFFEREXT) \
   /* GL_EXT_framebuffer_multisample */ \
   GLE(RenderbufferStorageMultisample, "RenderbufferStorageMultisampleEXT",RENDERBUFFERSTORAGEMULTISAMPLEEXT) \
   /* <end> */

#define gdraw_GLx_(id)     gdraw_GL_##id
#define GDRAW_GLx_(id)     GDRAW_GL_##id
#define GDRAW_SHADERS      "gdraw_gl_shaders.inl"

// On Linux, GLhandleARB is void* (not GLuint) but the shader functions
// actually return/take GLuint values. Use GLuint as our handle type,
// matching the Mac pattern from gdraw_gl_shared.inl.
#define GDrawGLProgram GLuint
typedef GLuint GLhandle;
typedef gdraw_gl_resourcetype gdraw_resourcetype;

// Extensions
#define GLE(id, import, procname) static PFNGL##procname##PROC gl##id;
GDRAW_GL_EXTENSION_LIST
#undef GLE

static void load_extensions(void)
{
#define GLE(id, import, procname) gl##id = (PFNGL##procname##PROC) glfwGetProcAddress("gl" import);
   GDRAW_GL_EXTENSION_LIST
#undef GLE
}

static void clear_renderstate_platform_specific(void)
{
   glDisable(GL_ALPHA_TEST);
}

static void error_msg_platform_specific(const char *msg)
{
   fprintf(stderr, "[GDraw GL] %s\n", msg);
}

///////////////////////////////////////////////////////////////////////////////
//
//  Shared code
//

#define GDRAW_MULTISAMPLING

// Override RR_BREAK() to avoid SIGTRAP from GL debug checks on Linux
#ifdef RR_BREAK
#undef RR_BREAK
#endif
#define RR_BREAK() do { fprintf(stderr, "[GDraw] RR_BREAK suppressed (GL error)\n"); } while(0)

#include "../../../Windows64/Iggy/gdraw/gdraw_gl_shared.inl"

///////////////////////////////////////////////////////////////////////////////
//
//  Initialization and platform-specific functionality
//

GDrawFunctions *gdraw_GL_CreateContext(S32 w, S32 h, S32 msaa_samples)
{
   static const TextureFormatDesc tex_formats[] = {
      { IFT_FORMAT_rgba_8888,    1, 1,  4,   GL_RGBA,                            GL_RGBA,               GL_UNSIGNED_BYTE },
      { IFT_FORMAT_rgba_4444_LE, 1, 1,  2,   GL_RGBA4,                           GL_RGBA,               GL_UNSIGNED_SHORT_4_4_4_4 },
      { IFT_FORMAT_rgba_5551_LE, 1, 1,  2,   GL_RGB5_A1,                         GL_RGBA,               GL_UNSIGNED_SHORT_5_5_5_1 },
      { IFT_FORMAT_la_88,        1, 1,  2,   GL_LUMINANCE8_ALPHA8,               GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE },
      { IFT_FORMAT_la_44,        1, 1,  1,   GL_LUMINANCE4_ALPHA4,               GL_LUMINANCE_ALPHA,    GL_UNSIGNED_BYTE },
      { IFT_FORMAT_i_8,          1, 1,  1,   GL_INTENSITY8,                      GL_ALPHA,              GL_UNSIGNED_BYTE },
      { IFT_FORMAT_i_4,          1, 1,  1,   GL_INTENSITY4,                      GL_ALPHA,              GL_UNSIGNED_BYTE },
      { IFT_FORMAT_l_8,          1, 1,  1,   GL_LUMINANCE8,                      GL_LUMINANCE,          GL_UNSIGNED_BYTE },
      { IFT_FORMAT_l_4,          1, 1,  1,   GL_LUMINANCE4,                      GL_LUMINANCE,          GL_UNSIGNED_BYTE },
      { IFT_FORMAT_DXT1,         4, 4,  8,   GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,   0,                     GL_UNSIGNED_BYTE },
      { IFT_FORMAT_DXT3,         4, 4, 16,   GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,   0,                     GL_UNSIGNED_BYTE },
      { IFT_FORMAT_DXT5,         4, 4, 16,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,   0,                     GL_UNSIGNED_BYTE },
      { 0,                       0, 0,  0,   0,                                  0,                     0 },
   };

   GDrawFunctions *funcs;
   const char *s;
   GLint n;

   // check for the extensions we need
   s = (const char *) glGetString(GL_EXTENSIONS);
   if (s == NULL) {
      fprintf(stderr, "[GDraw GL] glGetString(GL_EXTENSIONS) returned NULL - GL context not current?\n");
      assert(s != NULL);
      return NULL;
   }

   // check for the extensions we won't work without
   if (!hasext(s, "GL_ARB_multitexture") ||
       !hasext(s, "GL_ARB_texture_compression") ||
       !hasext(s, "GL_ARB_texture_mirrored_repeat") ||
       !hasext(s, "GL_ARB_texture_non_power_of_two") ||
       !hasext(s, "GL_ARB_vertex_buffer_object") ||
       !hasext(s, "GL_EXT_framebuffer_object") ||
       !hasext(s, "GL_ARB_shader_objects") ||
       !hasext(s, "GL_ARB_vertex_shader") ||
       !hasext(s, "GL_ARB_fragment_shader"))
   {
      fprintf(stderr, "[GDraw GL] Required GL extensions not available\n");
      return NULL;
   }

   // if user requests multisampling and HW doesn't support it, bail
   if (!hasext(s, "GL_EXT_framebuffer_multisample") && msaa_samples > 1)
      return NULL;

   load_extensions();
   funcs = create_context(w, h);
   if (!funcs)
      return NULL;

   gdraw->tex_formats = tex_formats;

   // check for optional extensions
   gdraw->has_mapbuffer = true; // part of core VBO extension on regular GL
   gdraw->has_depth24 = true;   // we just assume.
   gdraw->has_texture_max_level = true; // core on regular GL

   if (hasext(s, "GL_EXT_packed_depth_stencil"))      gdraw->has_packed_depth_stencil = true;

   glGetIntegerv(GL_MAX_TEXTURE_SIZE, &n);
   gdraw->has_conditional_non_power_of_two = n < 8192;

   // clamp number of multisampling levels to max supported
   if (msaa_samples > 1) {
      glGetIntegerv(GL_MAX_SAMPLES, &n);
      gdraw->multisampling = RR_MIN(msaa_samples, n);
   }

   opengl_check();

   fprintf(stderr, "[GDraw GL] Context created successfully (%dx%d, msaa=%d)\n", w, h, msaa_samples);
   return funcs;
}

///////////////////////////////////////////////////////////////////////////////
//
//  4J-specific custom draw functions
//

void gdraw_GL_BeginCustomDraw_4J(IggyCustomDrawCallbackRegion *region, F32 *matrix)
{
   // Same as BeginCustomDraw but uses different depth param
   clear_renderstate();
   gdraw_GetObjectSpaceMatrix(matrix, region->o2w, gdraw->projection, depth_from_id(0), 1);
}

void gdraw_GL_CalculateCustomDraw_4J(IggyCustomDrawCallbackRegion *region, F32 *matrix)
{
   gdraw_GetObjectSpaceMatrix(matrix, region->o2w, gdraw->projection, 0.0f, 0);
}
