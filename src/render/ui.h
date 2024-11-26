/*  ----------------------------------- INFOS
	This header file contains functions and structs related to rendering IMGUI components and UI in general.
	
*/

#ifndef _UIH_
#define _UIH_

// ------------------------------------- IMGUI CUSTOM STUFF

void imgui_init(HWND window, render_context rContext) {
	// IMGUI Init

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(rContext.device, rContext.context);
}

void imgui_render() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
} 

#endif /* _UIH_ */