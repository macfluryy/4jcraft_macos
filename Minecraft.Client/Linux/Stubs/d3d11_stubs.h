#ifndef D3D11STUBS_H
#define D3D11STUBS_H

#pragma once

#include "winapi_stubs.h"
#include "DirectXMath/DirectXMath.h"

using namespace DirectX;

typedef struct _RECT
{
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
// typedef DWORD (*PTHREAD_START_ROUTINE)(	LPVOID lpThreadParameter);
// typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

// Used only by windows/durango gdraw and UIController. Will be unnecessary once we have our own
// UIController stubs.
typedef void ID3D11ShaderResourceView;
typedef void ID3D11Resource;
typedef void ID3D11Texture2D;
typedef void D3D11_TEXTURE2D_DESC;

#endif // D3D11STUBS_H