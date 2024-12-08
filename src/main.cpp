/*  ----------------------------------- MAIN
	Entry point of the program. Should be the entry point of the compiler too.
	
*/

#define internal static
#define local_persist static
#define global_variable static

#define STR2(x) #x
#define STR(x) STR2(x)

#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPLEMENTATION
#define RAYMATH_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

// DEPENDENCIES :

#include <windows.h>
#include <combaseapi.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

// imgui libs
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.cpp"
#include "imgui_impl_dx11.cpp"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

// raylib techs
#include "raymath.h"

// stb
#include "stb_image.h"

// Custom 
#include "types.h"
#include "platform/platform.h"
#include "platform/io.h"
#include "parser.h"
#include "render/render.h"
#include "render/ui.h"


int WINAPI WinMain(HINSTANCE instance, HINSTANCE previnstance, LPSTR cmdline, int cmdshow)
{
	HRESULT hr;
	
	// window instance
	ui32 width = CW_USEDEFAULT;
    ui32 height = CW_USEDEFAULT;
	HWND window = platform_create_window(instance, width, height);
	
	ui32 platformClockSpeed = platform_get_clock_speed();
	LARGE_INTEGER freq, c1;
	QueryPerformanceCounter(&c1);
	
	f64 currentTime = platform_get_time(platformClockSpeed);
    
	camera rCamera = {0};
	
	// contexes and global structures
	render_context rContext = {0};
	ui_context uiContext = {
		.fps = 0,
		.fps_display_delay = 0.05f, // in seconds
	};
	
	// init rendering context
	hr = render_init_d3d11(window, &rContext, &rCamera);


    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

	imgui_init(window, rContext);
	
	viewport_size window_size = {
		.width = 0,
		.height = 0,
	};
	
	int clockFrequency;
	
	// -------------------------------------------- rendering testing
	
	vertex vertice_data[3] = 
	{
	{{0.f,100.f},{0.f,0.f},{1.f,1.f,1.f}},
	{{-100.f,0.f},{0.f,1.f},{1.f,1.f,1.f}},
	{{100.f,0.f},{1.f,0.f},{1.f,1.f,1.f}},
	};
	
	mesh mesh_data = {
		.vertice_count = 3,
		.vertices = vertice_data,
	};
	
	//  ------------------------------------------- frame loop
	
	for (;;)
    {
		// update states here
		f64 new_time = platform_get_time(platformClockSpeed);
		window_size = platform_get_window_size(window);
	
        // windows api message processing
        MSG msg;
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }
		
		// --------------------------- STATES
		
		    LARGE_INTEGER c2;
            QueryPerformanceCounter(&c2);
            f32 delta = (f32)((f64)(c2.QuadPart - c1.QuadPart) / platformClockSpeed);
            c1 = c2;
			
			// update fps
			update_ui_context(&uiContext, 1.0f/delta, new_time);
		
		// --------------------------- RENDERING

        // reset frame and rendering data
		render_reset_frame(&rContext);
		
		// resize swap chain if needed + updates window_size too
		render_resize_swapchain(window, &window_size, &rContext);

        // can render only if window size is non-zero - we must have backbuffer & RenderTarget view created
        if (rContext.rtView)
        {
            // reset all our pipeline states and input assembler
			render_pipeline_states(&rContext, &window_size);
			
			// clear screen
			f32 color[4] = { 0.f, 0.f, 0.f, 1.f };
			render_clear_screen(&rContext, color);
			
			// add vertex to our queue
			render_mesh_vqueue(&rContext, mesh_data);
			
			// ---------------------------- upload stuff to the gpu
			
			// resize the camera and send it
			render_upload_camera_ubuffer(&rContext, &rCamera, window_size);
			render_upload_dynamic_vbuffer(&rContext);
			
		
			
			// IMGUI RENDER
			imgui_render();
			
			if(ImGui::Begin("test")){
				ImGui::Text("FPS : %f", uiContext.fps);
				
				if(ImGui::Button("button")){
					ImGui::Text("pressed");
				}
			} ImGui::End();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			
			// todo: draw vertices
            rContext.context->Draw(rContext.vCount, 0);
        }

        // change to FALSE to disable vsync
        BOOL vsync = FALSE;
        hr = rContext.swapChain->Present(vsync ? 1 : 0, 0);
		
		// debug code
		hr = rContext.device->GetDeviceRemovedReason();
		
        if (hr == DXGI_STATUS_OCCLUDED)
        {
            // window is minimized, cannot vsync - instead sleep a bit
            if (vsync)
            {
                Sleep(10);
            }
        }	
        else if (FAILED(hr))
        {
            FatalError("Failed to present swap chain! Device lost?");
        }
		
		currentTime = new_time;
    }
}