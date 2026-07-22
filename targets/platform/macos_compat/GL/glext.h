#pragma once

#include "GL/gl.h"
#include <OpenGL/gl3ext.h>


#ifndef GL_COMBINE
#define GL_COMBINE                     0x8570
#endif
#ifndef GL_COMBINE_RGB
#define GL_COMBINE_RGB                 0x8571
#endif
#ifndef GL_COMBINE_ALPHA
#define GL_COMBINE_ALPHA               0x8572
#endif
#ifndef GL_SOURCE0_RGB
#define GL_SOURCE0_RGB                 0x8580
#endif
#ifndef GL_SOURCE1_RGB
#define GL_SOURCE1_RGB                 0x8581
#endif
#ifndef GL_SOURCE0_ALPHA
#define GL_SOURCE0_ALPHA               0x8588
#endif
#ifndef GL_SOURCE1_ALPHA
#define GL_SOURCE1_ALPHA               0x8589
#endif
#ifndef GL_OPERAND0_RGB
#define GL_OPERAND0_RGB                0x8590
#endif
#ifndef GL_OPERAND1_RGB
#define GL_OPERAND1_RGB                0x8591
#endif
#ifndef GL_OPERAND0_ALPHA
#define GL_OPERAND0_ALPHA              0x8598
#endif
#ifndef GL_OPERAND1_ALPHA
#define GL_OPERAND1_ALPHA              0x8599
#endif
#ifndef GL_RGB_SCALE
#define GL_RGB_SCALE                   0x8573
#endif
#ifndef GL_ADD_SIGNED
#define GL_ADD_SIGNED                  0x8574
#endif
#ifndef GL_INTERPOLATE
#define GL_INTERPOLATE                 0x8575
#endif
#ifndef GL_SUBTRACT
#define GL_SUBTRACT                    0x84E7
#endif
#ifndef GL_CONSTANT
#define GL_CONSTANT                    0x8576
#endif
#ifndef GL_PRIMARY_COLOR
#define GL_PRIMARY_COLOR               0x8577
#endif
#ifndef GL_PREVIOUS
#define GL_PREVIOUS                    0x8578
#endif


#ifndef GL_MAX_TEXTURE_LOD_BIAS
#define GL_MAX_TEXTURE_LOD_BIAS        0x84FD
#endif
#ifndef GL_TEXTURE_FILTER_CONTROL
#define GL_TEXTURE_FILTER_CONTROL      0x8500
#endif
#ifndef GL_TEXTURE_LOD_BIAS
#define GL_TEXTURE_LOD_BIAS            0x8501
#endif


#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER             0x812D
#endif


#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT  0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif


#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL               0x84F9
#endif
#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8           0x84FA
#endif
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8            0x88F0
#endif
#ifndef GL_TEXTURE_STENCIL_SIZE
#define GL_TEXTURE_STENCIL_SIZE        0x88F1
#endif


#ifndef GL_DEPTH_COMPONENT16
#define GL_DEPTH_COMPONENT16           0x81A5
#endif
#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24           0x81A6
#endif
#ifndef GL_DEPTH_COMPONENT32
#define GL_DEPTH_COMPONENT32           0x81A7
#endif


#ifndef GL_TEXTURE_COMPARE_MODE
#define GL_TEXTURE_COMPARE_MODE        0x884C
#endif
#ifndef GL_TEXTURE_COMPARE_FUNC
#define GL_TEXTURE_COMPARE_FUNC        0x884D
#endif
#ifndef GL_COMPARE_R_TO_TEXTURE
#define GL_COMPARE_R_TO_TEXTURE        0x884E
#endif


#ifndef GL_RGBA32F
#define GL_RGBA32F                     0x8814
#endif
#ifndef GL_RGB32F
#define GL_RGB32F                      0x8815
#endif
#ifndef GL_RGBA16F
#define GL_RGBA16F                     0x881A
#endif
#ifndef GL_RGB16F
#define GL_RGB16F                      0x881B
#endif


#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT                  0x140B
#endif


#ifndef GL_POINT_SPRITE
#define GL_POINT_SPRITE                0x8861
#endif
#ifndef GL_COORD_REPLACE
#define GL_COORD_REPLACE               0x8862
#endif


#ifndef GL_COLOR_SUM
#define GL_COLOR_SUM                   0x8458
#endif


#ifndef GL_MIN
#define GL_MIN                         0x8007
#endif
#ifndef GL_MAX
#define GL_MAX                         0x8008
#endif
#ifndef GL_FUNC_ADD
#define GL_FUNC_ADD                    0x8006
#endif
#ifndef GL_FUNC_SUBTRACT
#define GL_FUNC_SUBTRACT               0x800A
#endif
#ifndef GL_FUNC_REVERSE_SUBTRACT
#define GL_FUNC_REVERSE_SUBTRACT       0x800B
#endif


#ifndef GL_CONSTANT_COLOR
#define GL_CONSTANT_COLOR              0x8001
#endif
#ifndef GL_ONE_MINUS_CONSTANT_COLOR
#define GL_ONE_MINUS_CONSTANT_COLOR    0x8002
#endif
#ifndef GL_CONSTANT_ALPHA
#define GL_CONSTANT_ALPHA              0x8003
#endif
#ifndef GL_ONE_MINUS_CONSTANT_ALPHA
#define GL_ONE_MINUS_CONSTANT_ALPHA    0x8004
#endif


#ifndef GL_SAMPLES_PASSED
#define GL_SAMPLES_PASSED              0x8914
#endif
#ifndef GL_QUERY_RESULT
#define GL_QUERY_RESULT                0x8866
#endif
#ifndef GL_QUERY_RESULT_AVAILABLE
#define GL_QUERY_RESULT_AVAILABLE      0x8867
#endif


#ifndef GL_PIXEL_PACK_BUFFER
#define GL_PIXEL_PACK_BUFFER           0x88EB
#endif
#ifndef GL_PIXEL_UNPACK_BUFFER
#define GL_PIXEL_UNPACK_BUFFER         0x88EC
#endif
#ifndef GL_PIXEL_PACK_BUFFER_BINDING
#define GL_PIXEL_PACK_BUFFER_BINDING   0x88ED
#endif
#ifndef GL_PIXEL_UNPACK_BUFFER_BINDING
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#endif


#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW                 0x88E0
#endif
#ifndef GL_STREAM_READ
#define GL_STREAM_READ                 0x88E1
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW                 0x88E4
#endif
#ifndef GL_DYNAMIC_DRAW
#define GL_DYNAMIC_DRAW                0x88E8
#endif


#ifndef GL_READ_ONLY
#define GL_READ_ONLY                   0x88B8
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY                  0x88B9
#endif
#ifndef GL_READ_WRITE
#define GL_READ_WRITE                  0x88BA
#endif


#ifndef GL_BLEND_DST_RGB
#define GL_BLEND_DST_RGB               0x80C8
#endif
#ifndef GL_BLEND_SRC_RGB
#define GL_BLEND_SRC_RGB               0x80C9
#endif
#ifndef GL_BLEND_DST_ALPHA
#define GL_BLEND_DST_ALPHA             0x80CA
#endif
#ifndef GL_BLEND_SRC_ALPHA
#define GL_BLEND_SRC_ALPHA             0x80CB
#endif


#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE        0x8CD5
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#endif
#ifndef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#endif
#ifndef GL_FRAMEBUFFER_UNSUPPORTED
#define GL_FRAMEBUFFER_UNSUPPORTED     0x8CDD
#endif
#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0           0x8CE0
#endif
#ifndef GL_DEPTH_ATTACHMENT
#define GL_DEPTH_ATTACHMENT            0x8D00
#endif
#ifndef GL_STENCIL_ATTACHMENT
#define GL_STENCIL_ATTACHMENT          0x8D20
#endif
#ifndef GL_RENDERBUFFER
#define GL_RENDERBUFFER                0x8D41
#endif
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER                 0x8D40
#endif


#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER            0x8CA8
#endif
#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER            0x8CA9
#endif