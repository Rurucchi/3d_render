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

void render_init_ubuffer(render_context* rContext) {
	// todo: fix this
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(mx) * 2,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

    rContext->device->CreateBuffer(&desc, NULL, &rContext->ubuffer); 
};

HRESULT render_init_d3d11(HWND window, render_context* rContext, game_camera* camera) {
	/* doc:
	https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdevice */
	
	HRESULT hr;
	viewport_size vp = platform_get_window_size(window);
	
	// create D3D11 device & context
    {
        UINT flags = 0;
	#ifdef NDEBUG
		// this enables VERY USEFUL debug messages in debugger output
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
        D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
        hr = D3D11CreateDevice(
            NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, levels, ARRAYSIZE(levels),
            D3D11_SDK_VERSION, &rContext->device, NULL, &rContext->context);

        // We could try D3D_DRIVER_TYPE_WARP driver type which enables software rendering
        // (could be useful on broken drivers or remote desktop situations)
        AssertHR(hr);
    }
	
    {
		#ifdef NDEBUG
	
		// for debug builds enable VERY USEFUL debug break on API errors
        ID3D11InfoQueue* info;
        rContext->device->QueryInterface(IID_ID3D11InfoQueue, (void**)&info);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
        info->Release();
    }

    {
		// enable debug break for DXGI too
        IDXGIInfoQueue* dxgiInfo;
        hr = DXGIGetDebugInterface1(0, IID_IDXGIInfoQueue, (void**)&dxgiInfo);
        AssertHR(hr);
        dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        dxgiInfo->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, TRUE);
        dxgiInfo->Release();
    }
    // debugger will break on errors
	#endif
	
	// create DXGI swap chain
	{
		/* doc:
		https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nn-d3d11-id3d11query
		https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgidevice 
		https://learn.microsoft.com/en-us/windows/win32/api/_direct3ddxgi/ 
		https://learn.microsoft.com/en-us/windows/win32/api/dxgi/ 
		https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiswapchain 
		https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgifactory 
		https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nn-dxgi-idxgiadapter */ 
		
		// get DXGI device from D3D11 device
		IDXGIDevice* dxgiDevice;
        hr = rContext->device->QueryInterface(IID_IDXGIDevice, (void**)&dxgiDevice);
        AssertHR(hr);
		
		// get DXGI adapter from DXGI device
        IDXGIAdapter* dxgiAdapter;
        hr = dxgiDevice->GetAdapter(&dxgiAdapter);
        AssertHR(hr);

        // get DXGI factory from DXGI adapter
        IDXGIFactory2* factory;
        hr = dxgiAdapter->GetParent(IID_IDXGIFactory2, (void**)&factory);
        AssertHR(hr);

        DXGI_SWAP_CHAIN_DESC1 desc =
        {
            // default 0 value for width & height means to get it from HWND automatically
            //.Width = 0,
            //.Height = 0,

            // or use DXGI_FORMAT_R8G8B8A8_UNORM_SRGB for storing sRGB
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,

            // FLIP presentation model does not allow MSAA framebuffer
            // if you want MSAA then you'll need to render offscreen and manually
            // resolve to non-MSAA framebuffer
			// more info: https://all500234765.github.io/graphics/2019/10/28/msaa-dx11/
            .SampleDesc = { 1, 0 },

            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = 2,

            // we don't want any automatic scaling of window content
            // this is supported only on FLIP presentation model
            .Scaling = DXGI_SCALING_NONE,

            // use more efficient FLIP presentation model
            // Windows 10 allows to use DXGI_SWAP_EFFECT_FLIP_DISCARD
            // for Windows 8 compatibility use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
            // for Windows 7 compatibility use DXGI_SWAP_EFFECT_DISCARD
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        };
	
		// create (actual) swapchain
        hr = factory->CreateSwapChainForHwnd((IUnknown*)rContext->device, window, &desc, NULL, NULL, &rContext->swapChain);
        AssertHR(hr);

        // disable silly Alt+Enter changing monitor resolution to match window size
        factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

		// release init stuff (not needed anymore)
        factory->Release();
        dxgiAdapter->Release();
        dxgiDevice->Release();
	}
	
	// init needed buffers 
	{
		render_init_ubuffer(rContext);	
	}
	
	// vertex & pixel shaders for drawing triangle, plus input layout for vertex input
    {
        // these must match vertex shader input layout (VS_INPUT in vertex shader source below)
        D3D11_INPUT_ELEMENT_DESC desc[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(struct vertex, uv),       D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(struct vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };


		char vsLocation[] = "triangle.vs.fxc";
		char psLocation[] = "triangle.ps.fxc";
	
		complete_file vblob = {0};
		complete_file pblob = {0};
		
		file_fullread(vsLocation, &vblob);
		file_fullread(psLocation, &pblob);
		
		
		// this is where we send shaders to the GPU
		
        hr = rContext->device->CreateVertexShader(vblob.memory, vblob.size, NULL, &rContext->vshader);
        hr = rContext->device->CreatePixelShader(pblob.memory, pblob.size, NULL, &rContext->pshader);
        hr = rContext->device->CreateInputLayout(desc, ARRAYSIZE(desc), vblob.memory, vblob.size, &rContext->layout);

		file_fullfree(&vblob);
		file_fullfree(&pblob);
    }
	
	{
		// todo: rewrite all of this
		
        // checkerboard texture, with 50% transparency on black colors
		
		// todo: asset pipeline for textures
		
		// for testing
        // unsigned int pixels[] =
        // {
            // 0x80000000, 0xffffffff,
            // 0xffffffff, 0x80000000,
        // };
		
		// open and decode the hitcircle texture
		// char* location = gameTextureLocation;
		// complete_file textureFile = {0};
		// complete_img hitcircle_sprite = file_decodePNG(location, &textureFile);

        D3D11_TEXTURE2D_DESC desc =
        {
            .Width = hitcircle_sprite.x,
            .Height = hitcircle_sprite.y,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };

        D3D11_SUBRESOURCE_DATA data =
        {
            .pSysMem = hitcircle_sprite.memory,
            .SysMemPitch = hitcircle_sprite.x * hitcircle_sprite.channels_in_file,
        };

        ID3D11Texture2D* texture;
        rContext->device->CreateTexture2D(&desc, &data, &texture);
        rContext->device->CreateShaderResourceView((ID3D11Resource*)texture, NULL, &rContext->textureView);
        texture->Release();
		file_fullfree(&textureFile);
    }
	
	// sampler
	{
		/* doc:
		https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11device-createsamplerstate
		https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc */
		
        D3D11_SAMPLER_DESC desc =
        {
            .Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
            .AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
            .AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
            .MipLODBias = 0,
            .MaxAnisotropy = 1,
            .MinLOD = 0,
            .MaxLOD = D3D11_FLOAT32_MAX,
        };

        rContext->device->CreateSamplerState(&desc, &rContext->sampler);
    }
	
	{
        // todo: disabled culling for now, check if we enable it later
		// more info: https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
		/* doc:
		https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc
		(concept) https://www.khronos.org/opengl/wiki/Face_Culling */
		
        D3D11_RASTERIZER_DESC desc =
        {
            .FillMode = D3D11_FILL_SOLID,
            .CullMode = D3D11_CULL_NONE,
            .DepthClipEnable = TRUE,
        };
        rContext->device->CreateRasterizerState(&desc, &rContext->rasterizerState);
    }
	
	{
        // todo: disabled depth & stencil test for now, will be enabled later
        D3D11_DEPTH_STENCIL_DESC desc =
        {
            .DepthEnable = FALSE,
            .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D11_COMPARISON_LESS,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
            // .FrontFace = ... 
            // .BackFace = ...
        };
        rContext->device->CreateDepthStencilState(&desc, &rContext->depthState);
    }
	
	
	// create Dynamic Shader Buffer
    {
		RENDER_INIT_DYNAMIC_VertexBuffer(rContext, 2048);
		render_upload_camera_uBuffer(rContext, camera, vp);
    }


	// set rt/ds view on rcontext
	// more info: https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nn-d3d11-id3d11view
    rContext->rtView = NULL;
    rContext->dsView = NULL;

	return hr;
}

#endif /* _DX11H_ */
