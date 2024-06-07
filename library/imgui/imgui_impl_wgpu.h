// dear imgui: Renderer for WebGPU
// This needs to be used along with a Platform Binding (e.g. GLFW)
// (Please note that WebGPU is currently experimental, will not run on non-beta browsers, and may break.)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'WGPUTextureView' as ImTextureID. Read the FAQ about ImTextureID!
//  [X] Renderer: Large meshes support (64k+ vertices) with 16-bit indices.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#pragma once
#ifdef __cplusplus
  #include "imgui.h" // IMGUI_IMPL_API
#else
  #include "cimgui.h" // IMGUI_IMPL_API
#endif

#ifndef IMGUI_DISABLE

  #include <webgpu/webgpu.h>

// Initialization data, for ImGui_ImplWGPU_Init()
struct ImGui_ImplWGPU_InitInfo {
    WGPUDevice Device;
    int NumFramesInFlight;
    WGPUTextureFormat RenderTargetFormat;
    WGPUTextureFormat DepthStencilFormat;
    WGPUMultisampleState PipelineMultisampleState;
};

CIMGUI_IMPL_API bool ImGui_ImplWGPU_Init(struct ImGui_ImplWGPU_InitInfo* init_info);
CIMGUI_IMPL_API void ImGui_ImplWGPU_Shutdown();
CIMGUI_IMPL_API void ImGui_ImplWGPU_NewFrame();
CIMGUI_IMPL_API void ImGui_ImplWGPU_RenderDrawData(
  ImDrawData* draw_data,
  WGPURenderPassEncoder pass_encoder);

// Use if you want to reset your rendering device without losing Dear ImGui state.
CIMGUI_IMPL_API void ImGui_ImplWGPU_InvalidateDeviceObjects();
CIMGUI_IMPL_API bool ImGui_ImplWGPU_CreateDeviceObjects();

#endif // #ifndef IMGUI_DISABLE
