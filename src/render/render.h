/*  ----------------------------------- RENDER
	This header file contains functions related to rendering with D3D11.
	
*/

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#ifndef _DX11H_
#define _DX11H_

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

// structs

struct vertex
{
    f32 position[2];
    f32 uv[2];
    f32 color[3];
};

struct quad_mesh {
	i32 x;
	i32 y;
	i32 width;
	i32 height;
	// float orientation;
};

struct render_camera
{
    v2 position;
    f32 ratio;
    f32 scale; // height = scale, width = ratio * scale
};

struct render_context {
	// basic device stuff
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain1* swapChain;
	
	// rc states
	ID3D11RenderTargetView* rtView;
	ID3D11DepthStencilView* dsView; 
	ID3D11InputLayout* layout;
	ID3D11RasterizerState* rasterizerState;
	
	ID3D11VertexShader* vshader;
    ID3D11PixelShader* pshader;
	ID3D11ShaderResourceView* textureView;
	ID3D11DepthStencilState* depthState;
	ID3D11BlendState* blendState;
	
	// buffers and shaders
	int vCount;
	vertex vQueue[4096];
	ID3D11Buffer* vbuffer;
	ID3D11Buffer* ubuffer;
	ID3D11SamplerState* sampler;
};

// functions

HRESULT render_init_d3d11(HWND window, render_context* rContext, game_camera* camera) {
	
	HRESULT hr;
	
	viewport_size vp = platform_getWindowSize(window);
}

#endif /* _DX11H_ */
