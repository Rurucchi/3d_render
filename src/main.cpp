/*  ----------------------------------- MAIN
	Entry point of the program. Should be the entry point of the compiler too.
	
*/

#define internal static
#define local_persist static
#define global_variable static

#define STR2(x) #x
#define STR(x) STR2(x)

// compiler stuff
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#pragma comment (lib, "gdi32")
#pragma comment (lib, "user32")
#pragma comment (lib, "dxguid")
#pragma comment (lib, "dxgi")
#pragma comment (lib, "d3d11")
#pragma comment (lib, "d3dcompiler")

// microsoft 

#include <windows.h>
#include <combaseapi.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

// std
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>



// raylib 
// #define RL_VECTOR2_TYPE
// #define RL_VECTOR3_TYPE
// #define RL_MATRIX_TYPE
#define RAYMATH_IMPLEMENTATION
#include "raylib/raymath.h"
// #include "raylib/rcamera_ex.h"
#define RCAMERA_IMPLEMENTATION
#define RCAMERA_STANDALONE
#include "raylib/rcamera.h"

// stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// imgui libs
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_IMPLEMENTATION
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.cpp"
#include "imgui_impl_dx11.cpp"
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

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
    	
	Camera camera = { 0 };
    camera.position = { -10.0f, 10.0f, -10.0f };  // Camera position
    camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 90.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
	
	// contexes and global structures
	render_context rContext = {0};
	ui_context uiContext = {
		.fps = 0,
		.fps_display_delay = 0.05f, // in seconds
	};
	
	// init rendering context
	hr = render_init_d3d11(window, &rContext);


    // show the window
    ShowWindow(window, SW_SHOWDEFAULT);

	imgui_init(window, rContext);
	
	viewport_size window_size = {
		.width = 0,
		.height = 0,
	};
	
	int clockFrequency;
	
	// -------------------------------------------- rendering testing
	
	// this is our box
	struct vertex vertice_data[] = {
    // Front face
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-right

    // Back face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-left
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-right

    // Left face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-left
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-right

    // Right face
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-right

    // Top face
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-right

    // Bottom face
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-left
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-left
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Top-right
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}, // Bottom-right
};

	mesh mesh_data = {
		.vertice_count = 36,
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
			f32 color[4] = { 0.2f, 0.2f, 0.2f, 1.f }; // black
			render_clear_screen(&rContext, color);
			
			// ----- upload stuff to the gpu before rendering
			
			// resize the camera and send it
			render_upload_frame_buffer(&rContext, &camera, window_size);
			
			
			// ----- rendering
			render_draw_mesh(&rContext, mesh_data);
			
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
        }

        // change to FALSE to disable vsync
        BOOL vsync = TRUE;
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