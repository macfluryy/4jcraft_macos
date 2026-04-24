/* macOS compatibility shim: GL/glext.h -> OpenGL/gl3ext.h
 * Also pulls in our gl.h shim for the ARB typedef aliases. */
#pragma once
#include "GL/gl.h"
#ifndef USE_METAL
#include <OpenGL/gl3ext.h>
#endif

/* Additional EXT/ARB enums used by gdraw.c */
#ifndef GL_LUMINANCE8_ALPHA8
#define GL_LUMINANCE8_ALPHA8          0x8045
#endif
#ifndef GL_LUMINANCE4_ALPHA4
#define GL_LUMINANCE4_ALPHA4          0x8033
#endif
#ifndef GL_INTENSITY8
#define GL_INTENSITY8                 0x8050
#endif
#ifndef GL_INTENSITY4
#define GL_INTENSITY4                 0x804F
#endif
#ifndef GL_LUMINANCE8
#define GL_LUMINANCE8                 0x8040
#endif
#ifndef GL_LUMINANCE4
#define GL_LUMINANCE4                 0x803F
#endif
#ifndef GL_RGBA4
#define GL_RGBA4                      0x8056
#endif
#ifndef GL_RGB5_A1
#define GL_RGB5_A1                    0x8057
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