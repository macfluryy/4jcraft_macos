/* macOS compatibility shim: redirect GL/glu.h -> OpenGL/glu.h */
#pragma once
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#ifndef USE_METAL
#include <OpenGL/glu.h>
#endif
