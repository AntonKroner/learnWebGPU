// manually generated bindings due to dear_bindings not working.
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "cimgui.h"
#ifndef IMGUI_DISABLE
  #include <webgpu/webgpu.h>

  struct ImGui_ImplWGPU_InitInfo {
      WGPUDevice Device;
      int NumFramesInFlight;
      WGPUTextureFormat RenderTargetFormat;
      WGPUTextureFormat DepthStencilFormat;
      WGPUMultisampleState PipelineMultisampleState;
  };

  CIMGUI_IMPL_API bool cImGui_ImplWGPU_Init(struct ImGui_ImplWGPU_InitInfo* init_info);
  CIMGUI_IMPL_API void cImGui_ImplWGPU_Shutdown();
  CIMGUI_IMPL_API void cImGui_ImplWGPU_NewFrame();
  CIMGUI_IMPL_API void cImGui_ImplWGPU_RenderDrawData(
    ImDrawData* draw_data,
    WGPURenderPassEncoder pass_encoder);
  CIMGUI_IMPL_API void cImGui_ImplWGPU_InvalidateDeviceObjects();
  CIMGUI_IMPL_API bool cImGui_ImplWGPU_CreateDeviceObjects();

#endif // #ifndef IMGUI_DISABLE
#ifdef __cplusplus
} // End of extern "C" block
#endif
