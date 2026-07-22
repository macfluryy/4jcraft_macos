#pragma once  





#define IDOC


typedef enum gdraw_d3d11_resourcetype {
    GDRAW_D3D11_RESOURCE_rendertarget,
    GDRAW_D3D11_RESOURCE_texture,
    GDRAW_D3D11_RESOURCE_vertexbuffer,
    GDRAW_D3D11_RESOURCE_dynbuffer,  
                                     

    GDRAW_D3D11_RESOURCE__count,
} gdraw_d3d11_resourcetype;

IDOC extern int gdraw_D3D11_SetResourceLimits(gdraw_d3d11_resourcetype type,
                                              S32 num_handles, S32 num_bytes);












IDOC extern GDrawFunctions* gdraw_D3D11_CreateContext(ID3D11Device* dev,
                                                      ID3D11DeviceContext* ctx,
                                                      S32 w, S32 h);


















IDOC extern void gdraw_D3D11_DestroyContext(void);


IDOC extern void gdraw_D3D11_SetErrorHandler(
    void(__cdecl* error_handler)(HRESULT hr));






IDOC extern void gdraw_D3D11_SetRendertargetSize(S32 w, S32 h);






IDOC extern void gdraw_D3D11_SetTileOrigin(
    ID3D11RenderTargetView* main_rt, ID3D11DepthStencilView* main_ds,
    ID3D11ShaderResourceView* non_msaa_rt, S32 x, S32 y);













IDOC extern void gdraw_D3D11_NoMoreGDrawThisFrame(void);






IDOC extern void gdraw_D3D11_PreReset(void);



IDOC extern void gdraw_D3D11_PostReset(void);


IDOC extern void RADLINK gdraw_D3D11_BeginCustomDraw_4J(
    IggyCustomDrawCallbackRegion* Region, F32 mat[16]);
IDOC extern void RADLINK gdraw_D3D11_CalculateCustomDraw_4J(
    IggyCustomDrawCallbackRegion* Region, F32 mat[16]);
IDOC extern void RADLINK gdraw_D3D11_BeginCustomDraw(
    IggyCustomDrawCallbackRegion* Region, F32 mat[4][4]);




IDOC extern void RADLINK
gdraw_D3D11_EndCustomDraw(IggyCustomDrawCallbackRegion* Region);



IDOC extern void RADLINK gdraw_D3D11_GetResourceUsageStats(
    gdraw_d3d11_resourcetype type, S32* handles_used, S32* bytes_used);
















IDOC extern GDrawTexture* gdraw_D3D11_WrappedTextureCreate(
    ID3D11ShaderResourceView* tex_view);







IDOC extern void gdraw_D3D11_WrappedTextureChange(
    GDrawTexture* tex, ID3D11ShaderResourceView* tex_view);





IDOC extern void gdraw_D3D11_WrappedTextureDestroy(GDrawTexture* tex);




GDrawTexture* RADLINK gdraw_D3D11_MakeTextureFromResource(
    U8* resource_file, S32 length, IggyFileTextureRaw* texture);
void RADLINK gdraw_D3D11_DestroyTextureFromResource(GDrawTexture* tex);


extern void RADLINK gdraw_D3D11_setViewport_4J();