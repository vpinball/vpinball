// Derived from this Gist by Richard Gale:
//     https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0

// WARNING WARNING heavily modified for VPX

// ImGui BGFX binding

// You can copy and use unmodified imgui_impl_* files in your project. See
// main.cpp for an example of using this. If you use this binding you'll need to
// call 4 functions: ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(),
// ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see
// examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

void ImGui_Implbgfx_Init();
void ImGui_Implbgfx_Shutdown();
void ImGui_Implbgfx_NewFrame();
void ImGui_Implbgfx_RenderDrawLists(int view, int instanceCount, struct ImDrawData* draw_data);

void ImGui_Implbgfx_SetSDRColor(float* col); // vec4 multiplied with color for SDR scaling (including alpha)
void ImGui_Implbgfx_SetStereoOfs(float ofs); // +/- offset applied on stereo view rendering
void ImGui_Implbgfx_SetRotation(int rotate);

// Use if you want to reset your rendering device without losing ImGui state.
void ImGui_Implbgfx_InvalidateDeviceObjects();
bool ImGui_Implbgfx_CreateDeviceObjects();
