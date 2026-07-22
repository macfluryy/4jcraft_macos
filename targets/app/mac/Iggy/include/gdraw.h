




#ifndef __RAD_INCLUDE_GDRAW_H__
#define __RAD_INCLUDE_GDRAW_H__

#include "rrCore.h"

#define IDOC

RADDEFSTART



#ifndef IGGY_GDRAW_SHARED_TYPEDEF

#define IGGY_GDRAW_SHARED_TYPEDEF
typedef struct GDrawFunctions GDrawFunctions;

typedef struct GDrawTexture GDrawTexture;

#endif  

IDOC typedef struct GDrawVertexBuffer GDrawVertexBuffer;




IDOC typedef struct gswf_recti {
    S32 x0, y0;  
    S32 x1, y1;  
} gswf_recti;



IDOC typedef struct gswf_rectf {
    F32 x0, y0;  
    F32 x1, y1;  
} gswf_rectf;



IDOC typedef struct gswf_matrix {
    union {
        F32 m[2][2];  
        struct {
            F32 m00;  
            F32 m01;  
            F32 m10;  
            F32 m11;  
        };
    };
    F32 trans[2];  
} gswf_matrix;


#define GDRAW_STATS_batches 1
#define GDRAW_STATS_blits 2
#define GDRAW_STATS_alloc_tex 4
#define GDRAW_STATS_frees 8
#define GDRAW_STATS_defrag 16
#define GDRAW_STATS_rendtarg 32
#define GDRAW_STATS_clears 64
IDOC typedef struct GDrawStats {
    S16 nonzero_flags;  

    U16 num_batches;  
    U16 num_blits;    
                      
    U16 freed_objects;         
    U16 defrag_objects;        
    U16 alloc_tex;             
    U16 rendertarget_changes;  
    U16 num_clears;
    

    U32 drawn_indices;  
    U32 drawn_vertices;   
    U32 num_blit_pixels;  
    U32 alloc_tex_bytes;  
    U32 freed_bytes;      
    U32 defrag_bytes;     
    U32 cleared_pixels;   
    U32 reserved;
    
} GDrawStats;









IDOC typedef enum gdraw_bformat {
    GDRAW_BFORMAT_vbib,                
    GDRAW_BFORMAT_wii_dlist,           
    GDRAW_BFORMAT_vbib_single_format,  
                                       
                                       

    GDRAW_BFORMAT__count,
} gdraw_bformat;







IDOC typedef struct GDrawInfo {
    S32 num_stencil_bits;  
    U32 max_id;  
                 
    U32 max_texture_size;  
                           
    U32 buffer_format;     
    rrbool shared_depth_stencil;  
                                  
    rrbool always_mipmap;  
                           
    rrbool conditional_nonpow2;  
                                 
    rrbool has_rendertargets;    
                                 
    rrbool no_nonpow2;           
} GDrawInfo;                     




IDOC typedef void RADLINK gdraw_get_info(GDrawInfo* d);









IDOC typedef enum gdraw_blend {
    GDRAW_BLEND_none,      
    GDRAW_BLEND_alpha,     
                           
    GDRAW_BLEND_multiply,  
    GDRAW_BLEND_add,       

    GDRAW_BLEND_filter,   
                          
    GDRAW_BLEND_special,  
                          

    GDRAW_BLEND__count,
} gdraw_blend;


IDOC typedef enum gdraw_blendspecial {
    GDRAW_BLENDSPECIAL_layer,       
    GDRAW_BLENDSPECIAL_multiply,    
    GDRAW_BLENDSPECIAL_screen,      
    GDRAW_BLENDSPECIAL_lighten,     
    GDRAW_BLENDSPECIAL_darken,      
    GDRAW_BLENDSPECIAL_add,         
    GDRAW_BLENDSPECIAL_subtract,    
    GDRAW_BLENDSPECIAL_difference,  
    GDRAW_BLENDSPECIAL_invert,      
    GDRAW_BLENDSPECIAL_overlay,     
                                    
    GDRAW_BLENDSPECIAL_hardlight,   
                                    

    
    GDRAW_BLENDSPECIAL_erase,          
    GDRAW_BLENDSPECIAL_alpha_special,  

    GDRAW_BLENDSPECIAL__count,
} gdraw_blendspecial;





















IDOC typedef enum gdraw_filter {
    GDRAW_FILTER_blur,         
    GDRAW_FILTER_colormatrix,  
    GDRAW_FILTER_bevel,        
    GDRAW_FILTER_dropshadow,   

    GDRAW_FILTER__count,
} gdraw_filter;








IDOC typedef enum gdraw_texture {
    GDRAW_TEXTURE_none,            
    GDRAW_TEXTURE_normal,          
    GDRAW_TEXTURE_alpha,           
    GDRAW_TEXTURE_radial,          
    GDRAW_TEXTURE_focal_gradient,  
    GDRAW_TEXTURE_alpha_test,  
                               

    GDRAW_TEXTURE__count,
} gdraw_texture;


IDOC typedef enum gdraw_wrap {
    GDRAW_WRAP_clamp,   
    GDRAW_WRAP_repeat,  
    GDRAW_WRAP_mirror,  
    GDRAW_WRAP_clamp_to_border,  

    GDRAW_WRAP__count,
} gdraw_wrap;


typedef struct GDrawRenderState {
    S32 id;           
    U32 test_id : 1;  
    U32 set_id : 1;   
    U32 use_world_space
        : 1;  
    U32 scissor : 1;          
                              
    U32 identical_state : 1;  
                              
    U32 unused : 27;
    

    U8 texgen0_enabled;  
    U8 tex0_mode;        
    U8 wrap0;            
    U8 nearest0;         

    U8 blend_mode;     
    U8 special_blend;  
                       
                       
    U8 filter;         
                       
                       
    U8 filter_mode;    
                       
                       
                       
    U8 stencil_test;   
    U8 stencil_set;    
                     
                     

    U8 reserved[2];   
                      
    S32 blur_passes;  
                      
                      

    S16* cxf_add;  

    GDrawTexture* tex[3];  
                           
                           
    F32* edge_matrix;  
    gswf_matrix* o2w;  

    

    
    F32 color[4];  

    
    gswf_recti scissor_rect;  
                              
                              
    
    

    F32 s0_texgen[4];     
    F32 t0_texgen[4];     
                          
    F32 focal_point[4];   
                          
                          
    F32 blur_x, blur_y;   
                          
                          
    F32 shader_data[20];  
                          
} GDrawRenderState;



IDOC typedef void RADLINK gdraw_set_view_size_and_world_scale(
    S32 w, S32 h, F32 x_world_to_pixel, F32 y_world_to_pixel);










typedef void RADLINK gdraw_set_3d_transform(F32* mat); 

IDOC typedef void RADLINK gdraw_render_tile_begin(S32 tx0, S32 ty0, S32 tx1,
                                                  S32 ty1, S32 pad,
                                                  GDrawStats* stats);


IDOC typedef void RADLINK gdraw_render_tile_end(GDrawStats* stats);


IDOC typedef void RADLINK gdraw_rendering_begin(void);


IDOC typedef void RADLINK gdraw_rendering_end(void);








IDOC typedef void RADLINK gdraw_clear_stencil_bits(U32 bits);



IDOC typedef void RADLINK gdraw_clear_id(void);



IDOC typedef void RADLINK gdraw_filter_quad(GDrawRenderState* r, S32 x0, S32 y0,
                                            S32 x1, S32 y1, GDrawStats* stats);








IDOC typedef struct GDrawPrimitive {
    F32* vertices;  
                    
    U16* indices;   
                    

    S32 num_vertices;  
    S32 num_indices;   

    S32 vertex_format;  
                        

    U32 uniform_count;
    F32* uniforms;

    U8 drawprim_mode;
} GDrawPrimitive;



IDOC typedef void RADLINK gdraw_draw_indexed_triangles(GDrawRenderState* r,
                                                       GDrawPrimitive* prim,
                                                       GDrawVertexBuffer* buf,
                                                       GDrawStats* stats);















IDOC typedef void RADLINK gdraw_set_antialias_texture(S32 width, U8* rgba);











IDOC typedef enum gdraw_texture_format {
    
    GDRAW_TEXTURE_FORMAT_rgba32,  
                                  
                                  
                                  
    GDRAW_TEXTURE_FORMAT_font,    
    
    

    
    GDRAW_TEXTURE_FORMAT__platform = 16,

    
    
} gdraw_texture_format;


IDOC typedef enum gdraw_texture_type {
    GDRAW_TEXTURE_TYPE_rgba,  
                              
    GDRAW_TEXTURE_TYPE_bgra,  
                              
    GDRAW_TEXTURE_TYPE_argb,  
                              

    GDRAW_TEXTURE_TYPE__count,
} gdraw_texture_type;


IDOC typedef struct GDraw_MakeTexture_ProcessingInfo {
    U8* texture_data;     
    S32 num_rows;         
    S32 stride_in_bytes;  
                          
    S32 texture_type;     

    U32 temp_buffer_bytes;  
    U8* temp_buffer;  
                      

    void *p0, *p1, *p2, *p3, *p4, *p5, *p6,
        *p7;  
              
    U32 i0, i1, i2, i3, i4, i5, i6,
        i7;  
             
} GDraw_MakeTexture_ProcessingInfo;


IDOC typedef struct GDraw_Texture_Description {
    S32 width;          
    S32 height;         
    U32 size_in_bytes;  
} GDraw_Texture_Description;


IDOC typedef U32 gdraw_maketexture_flags;
#define GDRAW_MAKETEXTURE_FLAGS_mipmap \
    1 IDOC  
#define GDRAW_MAKETEXTURE_FLAGS_updatable \
    2 IDOC  
            
#define GDRAW_MAKETEXTURE_FLAGS_never_flush \
    4 IDOC  
            



IDOC typedef void RADLINK gdraw_set_texture_unique_id(GDrawTexture* tex,
                                                      void* old_unique_id,
                                                      void* new_unique_id);


IDOC typedef rrbool RADLINK gdraw_make_texture_begin(
    void* unique_id, S32 width, S32 height, gdraw_texture_format format,
    gdraw_maketexture_flags flags,
    GDraw_MakeTexture_ProcessingInfo* output_info, GDrawStats* stats);







IDOC typedef rrbool RADLINK
gdraw_make_texture_more(GDraw_MakeTexture_ProcessingInfo* info);







IDOC typedef GDrawTexture* RADLINK gdraw_make_texture_end(
    GDraw_MakeTexture_ProcessingInfo* info, GDrawStats* stats);






IDOC typedef rrbool RADLINK gdraw_update_texture_begin(GDrawTexture* tex,
                                                       void* unique_id,
                                                       GDrawStats* stats);







IDOC typedef void RADLINK gdraw_update_texture_rect(
    GDrawTexture* tex, void* unique_id, S32 x, S32 y, S32 stride, S32 w, S32 h,
    U8* data, gdraw_texture_format format);






IDOC typedef void RADLINK gdraw_update_texture_end(GDrawTexture* tex,
                                                   void* unique_id,
                                                   GDrawStats* stats);






IDOC typedef void RADLINK
gdraw_describe_texture(GDrawTexture* tex, GDraw_Texture_Description* desc);


IDOC typedef GDrawTexture* RADLINK gdraw_make_texture_from_resource(
    U8* resource_file, S32 file_len, void* texture);


IDOC typedef void RADLINK gdraw_free_texture_from_resource(GDrawTexture* tex);


IDOC typedef struct gswf_vertex_xy {
    F32 x, y;  
} gswf_vertex_xy;


IDOC typedef struct gswf_vertex_xyoffs {
    F32 x, y;  

    S16 aa;      
    S16 dx, dy;  
                 
    S16 unused;
} gswf_vertex_xyoffs;



IDOC typedef struct gswf_vertex_xyst {
    F32 x, y;  
    F32 s, t;  
} gswf_vertex_xyst;


typedef int gdraw_verify_size_xy[sizeof(gswf_vertex_xy) == 8 ? 1 : -1];
typedef int gdraw_verify_size_xyoffs[sizeof(gswf_vertex_xyoffs) == 16 ? 1 : -1];
typedef int gdraw_verify_size_xyst[sizeof(gswf_vertex_xyst) == 16 ? 1 : -1];

IDOC typedef enum gdraw_vformat {
    GDRAW_vformat_v2,     
                          
    GDRAW_vformat_v2aa,   
                          
    GDRAW_vformat_v2tc2,  
                          

    GDRAW_vformat__basic_count,
    GDRAW_vformat_ihud1 =
        GDRAW_vformat__basic_count,  
                                     

    GDRAW_vformat__count,
    GDRAW_vformat_mixed,  
                          
} gdraw_vformat;


IDOC typedef struct GDraw_MakeVertexBuffer_ProcessingInfo {
    U8* vertex_data;  
    U8* index_data;   

    S32 vertex_data_length;  
    S32 index_data_length;   

    void *p0, *p1, *p2, *p3, *p4, *p5, *p6,
        *p7;  
              
    U32 i0, i1, i2, i3, i4, i5, i6,
        i7;  
             
} GDraw_MakeVertexBuffer_ProcessingInfo;



IDOC typedef struct GDraw_VertexBuffer_Description {
    S32 size_in_bytes;  
} GDraw_VertexBuffer_Description;



IDOC typedef rrbool RADLINK gdraw_make_vertex_buffer_begin(
    void* unique_id, gdraw_vformat vformat, S32 vdata_len_in_bytes,
    S32 idata_len_in_bytes, GDraw_MakeVertexBuffer_ProcessingInfo* info,
    GDrawStats* stats);









IDOC typedef rrbool RADLINK
gdraw_make_vertex_buffer_more(GDraw_MakeVertexBuffer_ProcessingInfo* info);







IDOC typedef GDrawVertexBuffer* RADLINK gdraw_make_vertex_buffer_end(
    GDraw_MakeVertexBuffer_ProcessingInfo* info, GDrawStats* stats);






IDOC typedef void RADLINK gdraw_describe_vertex_buffer(
    GDrawVertexBuffer* buffer, GDraw_VertexBuffer_Description* desc);


IDOC typedef rrbool RADLINK gdraw_try_to_lock_texture(GDrawTexture* tex,
                                                      void* unique_id,
                                                      GDrawStats* stats);






IDOC typedef rrbool RADLINK gdraw_try_to_lock_vertex_buffer(
    GDrawVertexBuffer* vb, void* unique_id, GDrawStats* stats);






IDOC typedef void RADLINK gdraw_unlock_handles(GDrawStats* stats);










IDOC typedef void RADLINK gdraw_free_vertex_buffer(GDrawVertexBuffer* vb,
                                                   void* unique_id,
                                                   GDrawStats* stats);






IDOC typedef void RADLINK gdraw_free_texture(GDrawTexture* t, void* unique_id,
                                             GDrawStats* stats);













IDOC typedef U32 gdraw_texturedrawbuffer_flags;
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color \
    1 IDOC  
            
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha \
    2 IDOC  
            
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_stencil \
    4 IDOC  
            
#define GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_id \
    8 IDOC  
            



IDOC typedef rrbool RADLINK gdraw_texture_draw_buffer_begin(
    gswf_recti* region, gdraw_texture_format format,
    gdraw_texturedrawbuffer_flags flags, void* unique_id, GDrawStats* stats);






IDOC typedef GDrawTexture* RADLINK
gdraw_texture_draw_buffer_end(GDrawStats* stats);











IDOC typedef void RADLINK gdraw_draw_mask_begin(gswf_recti* region,
                                                S32 mask_bit,
                                                GDrawStats* stats);






IDOC typedef void RADLINK gdraw_draw_mask_end(gswf_recti* region, S32 mask_bit,
                                              GDrawStats* stats);












IDOC struct GDrawFunctions {
    
    gdraw_get_info* GetInfo;

    
    gdraw_set_view_size_and_world_scale* SetViewSizeAndWorldScale;
    gdraw_render_tile_begin* RenderTileBegin;
    gdraw_render_tile_end* RenderTileEnd;
    gdraw_set_antialias_texture* SetAntialiasTexture;

    
    gdraw_clear_stencil_bits* ClearStencilBits;
    gdraw_clear_id* ClearID;
    gdraw_filter_quad* FilterQuad;
    gdraw_draw_indexed_triangles* DrawIndexedTriangles;
    gdraw_make_texture_begin* MakeTextureBegin;
    gdraw_make_texture_more* MakeTextureMore;
    gdraw_make_texture_end* MakeTextureEnd;
    gdraw_make_vertex_buffer_begin* MakeVertexBufferBegin;
    gdraw_make_vertex_buffer_more* MakeVertexBufferMore;
    gdraw_make_vertex_buffer_end* MakeVertexBufferEnd;
    gdraw_try_to_lock_texture* TryToLockTexture;
    gdraw_try_to_lock_vertex_buffer* TryToLockVertexBuffer;
    gdraw_unlock_handles* UnlockHandles;
    gdraw_free_texture* FreeTexture;
    gdraw_free_vertex_buffer* FreeVertexBuffer;
    gdraw_update_texture_begin* UpdateTextureBegin;
    gdraw_update_texture_rect* UpdateTextureRect;
    gdraw_update_texture_end* UpdateTextureEnd;

    
    gdraw_texture_draw_buffer_begin* TextureDrawBufferBegin;
    gdraw_texture_draw_buffer_end* TextureDrawBufferEnd;

    gdraw_describe_texture* DescribeTexture;
    gdraw_describe_vertex_buffer* DescribeVertexBuffer;

    
    gdraw_set_texture_unique_id* SetTextureUniqueID;

    gdraw_draw_mask_begin* DrawMaskBegin;
    gdraw_draw_mask_end* DrawMaskEnd;

    gdraw_rendering_begin* RenderingBegin;
    gdraw_rendering_end* RenderingEnd;

    gdraw_make_texture_from_resource* MakeTextureFromResource;
    gdraw_free_texture_from_resource* FreeTextureFromResource;

    gdraw_set_3d_transform* Set3DTransform;
};












RADDEFEND

#endif
