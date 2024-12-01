/*  ----------------------------------- RENDER
	This header file contains functions related to rendering with D3D11.
	
*/

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

#ifndef _RENDERH_
#define _RENDERH_

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

// this will change depending on what we need
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

// camera
struct camera
{
    v2 position;
    f32 ratio;
    f32 scale; // height = scale, width = ratio * scale
};

// camera projection
mx game_OrthographicProjection(camera* game_camera, float width, float height)
{
    float L = game_camera->position.x - width / 2.f;
    float R = game_camera->position.x + width / 2.f;
    float T = game_camera->position.y + height / 2.f;
    float B = game_camera->position.y - height / 2.f;
    mx res = 
    {
        2.0f/(R-L),   0.0f,           0.0f,       0.0f,
        0.0f,         2.0f/(T-B),     0.0f,       0.0f,
        0.0f,         0.0f,           0.5f,       0.0f,
        (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f,
    };
    
    return res;
}

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

void render_create_dynamic_vbuffer(render_context *rContext, int bufferSize) {
	D3D11_BUFFER_DESC desc =
    {
        .ByteWidth = sizeof(vertex) * bufferSize,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
	};

	D3D11_SUBRESOURCE_DATA initial = { .pSysMem = rContext->vQueue };
    rContext->device->CreateBuffer(&desc, NULL, &rContext->vbuffer);
}

void render_upload_camera_ubuffer(render_context *rContext, camera* game_camera, viewport_size vp){	
	
	float width = (float)vp.width;
	float height = (float)vp.height;
	
	mx matrix = game_OrthographicProjection(game_camera, width, height);
	
	D3D11_MAPPED_SUBRESOURCE mapped;
	
	rContext->context->Map(rContext->ubuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &matrix, sizeof(matrix));
	rContext->context->Unmap(rContext->ubuffer, 0);
};

void render_init_pipeline(render_context* rContext, viewport_size* vpSize){
	

	D3D11_VIEWPORT viewport =
	{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = (FLOAT)vpSize->width,
		.Height = (FLOAT)vpSize->height,
		.MinDepth = 0,
		.MaxDepth = 1,
	};

	{
		// Input Assembler
		rContext->context->IASetInputLayout(rContext->layout);
		rContext->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT stride = sizeof(struct vertex);
		UINT offset = 0;
		rContext->context->IASetVertexBuffers(0, 1, &rContext->vbuffer, &stride, &offset);

		// Vertex Shader
		rContext->context->VSSetConstantBuffers(0, 1, &rContext->ubuffer);
		rContext->context->VSSetShader(rContext->vshader, NULL, 0);

		// Rasterizer Stage
		rContext->context->RSSetViewports(1, &viewport);
		rContext->context->RSSetState(rContext->rasterizerState);

		// Pixel Shader
		rContext->context->PSSetSamplers(0, 1, &rContext->sampler);
		rContext->context->PSSetShaderResources(0, 1, &rContext->textureView);
		rContext->context->PSSetShader(rContext->pshader, NULL, 0);

		// Output Merger
		rContext->context->OMSetBlendState(rContext->blendState, NULL, ~0U);
		rContext->context->OMSetDepthStencilState(rContext->depthState, 0);
		rContext->context->OMSetRenderTargets(1, &rContext->rtView, rContext->dsView);
	};
};

void render_resize_swapchain(HWND window, viewport_size* window_size, render_context* rContext) {
	
		HRESULT hr;
	
        // get current size for window client area
		viewport_size new_window_size = platform_get_window_size(window);
		
		
	    if (rContext->rtView == NULL || new_window_size.width != window_size->width || new_window_size.height != window_size->height) {
            if (rContext->rtView)
            {
                // release old swap chain buffers
                rContext->context->ClearState();
                rContext->rtView->Release();
                rContext->dsView->Release();
                rContext->rtView = NULL;
            }

            // resize to new size for non-zero size
            if (new_window_size.width != 0 && new_window_size.height != 0)
            {
                hr = rContext->swapChain->ResizeBuffers(0, new_window_size.width, new_window_size.height, DXGI_FORMAT_UNKNOWN, 0);
                if (FAILED(hr))
                {
                    FatalError("Failed to resize swap chain!");
                }

                // create RenderTarget view for new backbuffer texture
                ID3D11Texture2D* backbuffer;
                hr = rContext->swapChain->GetBuffer(0, IID_ID3D11Texture2D, (void**)&backbuffer);
				// rContext->swapChain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
                hr = rContext->device->CreateRenderTargetView(backbuffer, NULL, &rContext->rtView);
                backbuffer->Release();

                D3D11_TEXTURE2D_DESC depthDesc = 
                {
                    .Width = int_to_ui32(new_window_size.width),
                    .Height = int_to_ui32(new_window_size.height),
                    .MipLevels = 1,
                    .ArraySize = 1,
                    .Format = DXGI_FORMAT_D32_FLOAT, // or use DXGI_FORMAT_D32_FLOAT_S8X24_UINT if you need stencil
                    .SampleDesc = { 1, 0 },
                    .Usage = D3D11_USAGE_DEFAULT,
                    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
                };

                // create new depth stencil texture & DepthStencil view
                ID3D11Texture2D* depth;
                rContext->device->CreateTexture2D(&depthDesc, NULL, &depth);
                rContext->device->CreateDepthStencilView(depth, NULL, &rContext->dsView);
                depth->Release();
            }

			window_size->width = new_window_size.width;
			window_size->height = new_window_size.height;
        }
};


HRESULT render_init_d3d11(HWND window, render_context* rContext, camera* game_camera) {
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
	
	#ifdef NDEBUG
    {
		
	
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


		// char vsLocation[] = "triangle.vs.fxc";
		// char psLocation[] = "triangle.ps.fxc";
	
		// complete_file vblob = {0};
		// complete_file pblob = {0};
		
		// file_fullread(vsLocation, &vblob);
		// file_fullread(psLocation, &pblob);
		
		
		// this is where we send shaders to the GPU
		
        // hr = rContext->device->CreateVertexShader(vblob.memory, vblob.size, NULL, &rContext->vshader);
        // hr = rContext->device->CreatePixelShader(pblob.memory, pblob.size, NULL, &rContext->pshader);
        // hr = rContext->device->CreateInputLayout(desc, ARRAYSIZE(desc), vblob.memory, vblob.size, &rContext->layout);

		// file_fullfree(&vblob);
		// file_fullfree(&pblob);
    }
	
	{
		// todo: rewrite all of this
		
        // checkerboard texture, with 50% transparency on black colors
		
		// todo: asset pipeline for textures
		
		// for testing
        unsigned int pixels[] =
        {
            0x80000000, 0xffffffff,
            0xffffffff, 0x80000000,
        };
		
		// open and decode the hitcircle texture
		// char* location = gameTextureLocation;
		// complete_file textureFile = {0};
		// complete_img hitcircle_sprite = file_decodePNG(location, &textureFile);

        D3D11_TEXTURE2D_DESC desc =
        {
            .Width = 2,
            .Height = 2,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = { 1, 0 },
            .Usage = D3D11_USAGE_IMMUTABLE,
            .BindFlags = D3D11_BIND_SHADER_RESOURCE,
        };

        // D3D11_SUBRESOURCE_DATA data =
        // {
            // .pSysMem = pixels,
            // .SysMemPitch = sizeof(pixels),
        // };

        // ID3D11Texture2D* texture;
        // rContext->device->CreateTexture2D(&desc, &data, &texture);
        // rContext->device->CreateShaderResourceView((ID3D11Resource*)texture, NULL, &rContext->textureView);
        // texture->Release();
		// file_fullfree(&textureFile);
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
		render_create_dynamic_vbuffer(rContext, 2048);
		render_upload_camera_ubuffer(rContext, game_camera, vp);
    }


	// set rt/ds view on rcontext
	// more info: https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nn-d3d11-id3d11view
    rContext->rtView = NULL;
    rContext->dsView = NULL;

	return hr;
}


void render_reset_frame(render_context* rContext){
	rContext->vCount = 0;
};


#endif /* _RENDERH_ */
