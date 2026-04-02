#ifndef D3D11STUBS_H
#define D3D11STUBS_H

#pragma once

#include "app/linux/Stubs/DirectXMath/DirectXMath.h"
#include "winapi_stubs.h"

using namespace DirectX;

typedef struct _RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT;

// stole- i mean borrowed from OrbisStubs.h
typedef void ID3D11Device;
typedef void ID3D11DeviceContext;
typedef void IDXGISwapChain;
typedef RECT D3D11_RECT;
typedef void ID3D11RenderTargetView;
typedef void ID3D11DepthStencilView;
typedef void ID3D11Buffer;
// typedef DWORD (*PTHREAD_START_ROUTINE)(	void* lpThreadParameter);
// typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

// Used only by windows/durango gdraw and UIController. Will be unnecessary once
// we have our own UIController stubs.
typedef void ID3D11ShaderResourceView;
typedef void ID3D11Resource;
typedef void ID3D11Texture2D;
typedef void D3D11_TEXTURE2D_DESC;

enum D3D11_BLEND {
    D3D11_BLEND_ZERO = 1,
    D3D11_BLEND_ONE = 2,
    D3D11_BLEND_SRC_COLOR = 3,
    D3D11_BLEND_INV_SRC_COLOR = 4,
    D3D11_BLEND_SRC_ALPHA = 5,
    D3D11_BLEND_INV_SRC_ALPHA = 6,
    D3D11_BLEND_DEST_ALPHA = 7,
    D3D11_BLEND_INV_DEST_ALPHA = 8,
    D3D11_BLEND_DEST_COLOR = 9,
    D3D11_BLEND_INV_DEST_COLOR = 10,
    D3D11_BLEND_SRC_ALPHA_SAT = 11,
    D3D11_BLEND_BLEND_FACTOR = 14,
    D3D11_BLEND_INV_BLEND_FACTOR = 15,
    D3D11_BLEND_SRC1_COLOR = 16,
    D3D11_BLEND_INV_SRC1_COLOR = 17,
    D3D11_BLEND_SRC1_ALPHA = 18,
    D3D11_BLEND_INV_SRC1_ALPHA = 19
};

enum D3D11_COMPARISON_FUNC {
    D3D11_COMPARISON_NEVER = 1,
    D3D11_COMPARISON_LESS = 2,
    D3D11_COMPARISON_EQUAL = 3,
    D3D11_COMPARISON_LESS_EQUAL = 4,
    D3D11_COMPARISON_GREATER = 5,
    D3D11_COMPARISON_NOT_EQUAL = 6,
    D3D11_COMPARISON_GREATER_EQUAL = 7,
    D3D11_COMPARISON_ALWAYS = 8
};

#endif  // D3D11STUBS_H
