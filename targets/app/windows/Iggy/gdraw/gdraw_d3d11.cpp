#include "minecraft/stdafx.h"  































#define GDRAW_ASSERTS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#pragma warning(push)
#pragma warning(disable \
                : 4201)  

#include <d3d11.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#include "../include/gdraw.h"
#include "../include/iggy.h"
#include "gdraw_d3d11.h"

#pragma warning(pop)



#define D3D1X_(id) D3D11_##id
#define ID3D1X(id) ID3D11##id
#define gdraw_D3D1X_(id) gdraw_D3D11_##id
#define GDRAW_D3D1X_(id) GDRAW_D3D11_##id

typedef ID3D11Device ID3D1XDevice;
typedef ID3D11DeviceContext ID3D1XContext;
typedef F32 ViewCoord;
typedef gdraw_d3d11_resourcetype gdraw_resourcetype;

static void report_d3d_error(HRESULT hr, char* call, char* context);

static void* map_buffer(ID3D1XContext* ctx, ID3D11Buffer* buf, bool discard) {
    D3D11_MAPPED_SUBRESOURCE msr;
    HRESULT hr = ctx->Map(
        buf, 0,
        discard ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0,
        &msr);
    if (FAILED(hr)) {
        report_d3d_error(hr, "Map", "of buffer");
        return NULL;
    } else
        return msr.pData;
}

static void unmap_buffer(ID3D1XContext* ctx, ID3D11Buffer* buf) {
    ctx->Unmap(buf, 0);
}

static RADINLINE void set_pixel_shader(ID3D11DeviceContext* ctx,
                                       ID3D11PixelShader* shader) {
    ctx->PSSetShader(shader, NULL, 0);
}

static RADINLINE void set_vertex_shader(ID3D11DeviceContext* ctx,
                                        ID3D11VertexShader* shader) {
    ctx->VSSetShader(shader, NULL, 0);
}

static ID3D11BlendState* create_blend_state(ID3D11Device* dev, BOOL blend,
                                            D3D11_BLEND src, D3D11_BLEND dst) {
    D3D11_BLEND_DESC desc = {};
    desc.RenderTarget[0].BlendEnable = blend;
    desc.RenderTarget[0].SrcBlend = src;
    desc.RenderTarget[0].DestBlend = dst;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha =
        (src == D3D11_BLEND_DEST_COLOR) ? D3D11_BLEND_DEST_ALPHA : src;
    desc.RenderTarget[0].DestBlendAlpha = dst;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* res;
    HRESULT hr = dev->CreateBlendState(&desc, &res);
    if (FAILED(hr)) {
        report_d3d_error(hr, "CreateBlendState", "");
        res = NULL;
    }

    return res;
}

#define GDRAW_SHADER_FILE "gdraw_d3d10_shaders.inl"
#include "gdraw_d3d1x_shared.inl"

static void create_pixel_shader(ProgramWithCachedVariableLocations* p,
                                ProgramWithCachedVariableLocations* src) {
    *p = *src;
    if (p->bytecode) {
        HRESULT hr = gdraw->d3d_device->CreatePixelShader(p->bytecode, p->size,
                                                          NULL, &p->pshader);
        if (FAILED(hr)) {
            report_d3d_error(hr, "CreatePixelShader", "");
            p->pshader = NULL;
            return;
        }
    }
}

static void create_vertex_shader(ProgramWithCachedVariableLocations* p,
                                 ProgramWithCachedVariableLocations* src) {
    *p = *src;
    if (p->bytecode) {
        HRESULT hr = gdraw->d3d_device->CreateVertexShader(p->bytecode, p->size,
                                                           NULL, &p->vshader);
        if (FAILED(hr)) {
            report_d3d_error(hr, "CreateVertexShader", "");
            p->vshader = NULL;
            return;
        }
    }
}

GDrawFunctions* gdraw_D3D11_CreateContext(ID3D11Device* dev,
                                          ID3D11DeviceContext* ctx, S32 w,
                                          S32 h) {
    return create_context(dev, ctx, w, h);
}



void gdraw_D3D11_setViewport_4J() { set_viewport(); }
