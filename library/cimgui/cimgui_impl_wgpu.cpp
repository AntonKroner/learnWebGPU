// manually generated bindings due to dear_bindings not working.
#include "imgui.h"
#ifndef IMGUI_DISABLE
  #include "imgui_impl_wgpu.h"
  #include <webgpu/webgpu.h>

namespace cimgui {
  #include "cimgui_impl_wgpu.h"
} // namespace cimgui

CIMGUI_IMPL_API void cimgui::cImGui_ImplWGPU_RenderDrawData(
  cimgui::ImDrawData* draw_data,
  WGPURenderPassEncoder pass_encoder) {
  ::ImGui_ImplWGPU_RenderDrawData(reinterpret_cast<::ImDrawData*>(draw_data), pass_encoder);
}
CIMGUI_IMPL_API bool cimgui::cImGui_ImplWGPU_CreateDeviceObjects() {
  return ::ImGui_ImplWGPU_CreateDeviceObjects();
}
CIMGUI_IMPL_API void cimgui::cImGui_ImplWGPU_InvalidateDeviceObjects() {
  ::ImGui_ImplWGPU_InvalidateDeviceObjects();
}
CIMGUI_IMPL_API bool cimgui::cImGui_ImplWGPU_Init(
  cimgui::ImGui_ImplWGPU_InitInfo* init_info) {
  return ::ImGui_ImplWGPU_Init(reinterpret_cast<::ImGui_ImplWGPU_InitInfo*>(init_info));
}
CIMGUI_IMPL_API void cimgui::cImGui_ImplWGPU_Shutdown() {
  ::ImGui_ImplWGPU_Shutdown();
}
CIMGUI_IMPL_API void cimgui::cImGui_ImplWGPU_NewFrame() {
  ::ImGui_ImplWGPU_NewFrame();
}

#endif // #ifndef IMGUI_DISABLE
