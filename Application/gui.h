#ifndef Application_gui_H_
#define Application_gui_H_

#include "webgpu.h"
#include "GLFW/glfw3.h"
#include "cimgui/cimgui.h"
#include "cimgui/cimgui_impl_wgpu.h"
#include "cimgui/cimgui_impl_glfw.h"
#include "./Lightning.h"

bool Application_gui_attach(
  GLFWwindow* window,
  WGPUDevice device,
  WGPUTextureFormat depthFormat) {
  // CIMGUI_CHECKVERSION();
  ImGui_CreateContext(0);
  ImGui_GetIO();
  cImGui_ImplGlfw_InitForOther(window, true);
  struct ImGui_ImplWGPU_InitInfo initInfo = {
    .Device = device,
    .NumFramesInFlight = 3,
    .RenderTargetFormat = WGPUTextureFormat_BGRA8Unorm,
    .DepthStencilFormat = depthFormat,
    .PipelineMultisampleState = { .count = 1,
  .mask = UINT32_MAX,
  .alphaToCoverageEnabled = false,}};
  return cImGui_ImplWGPU_Init(&initInfo);
}
void Application_gui_detach() {
  cImGui_ImplGlfw_Shutdown();
  cImGui_ImplWGPU_Shutdown();
}
void Application_gui_render(
  WGPURenderPassEncoder renderPass,
  Application_Lighting* lightning) {
  cImGui_ImplWGPU_NewFrame();
  cImGui_ImplGlfw_NewFrame();
  ImGui_NewFrame();
  static float f = 0.0f;
  static int counter = 0;
  static bool show_demo_window = true;
  static bool show_another_window = false;
  static ImVec4 clear_color = { .x = 0.45f, .y = 0.55f, .z = 0.60f, .w = 1.00f };
  ImGui_Begin("Hello, world!", 0, 0);
  ImGui_Text("This is some useful text.");
  ImGui_Checkbox("Demo Window", &show_demo_window);
  ImGui_Checkbox("Another Window", &show_another_window);
  ImGui_SliderFloat("float", &f, 0.0f, 1.0f);
  ImGui_ColorEdit3("clear color", (float*)&clear_color, 0);
  if (ImGui_Button("Button")) {
    counter++;
  }
  ImGui_SameLine();
  ImGui_Text("counter = %d", counter);
  ImGuiIO* io = ImGui_GetIO();
  ImGui_Text(
    "Application average %.3f ms/frame (%.1f FPS)",
    1000.0f / io->Framerate,
    io->Framerate);
  // ImGui_End();
  // ImGui_Begin("Lighting", 0, 0);
  bool update = false;
  update =
    ImGui_ColorEdit3("Color #0", lightning->uniforms.colors[0].components, 0) || update;
  update = ImGui_DragFloat3("Direction #0", lightning->uniforms.directions[0].components)
           || update;
  update =
    ImGui_ColorEdit3("Color #1", lightning->uniforms.colors[1].components, 0) || update;
  update = ImGui_DragFloat3("Direction #1", lightning->uniforms.directions[1].components)
           || update;
  update =
    ImGui_SliderFloat("Hardness", &lightning->uniforms.hardness, 1.0f, 100.0f) || update;
  update = ImGui_SliderFloat("Diffusivity", &lightning->uniforms.diffusivity, 0.0f, 1.0f)
           || update;
  update = ImGui_SliderFloat("Specularity", &lightning->uniforms.specularity, 0.0f, 1.0f)
           || update;
  lightning->update = update;
  ImGui_End();
  ImGui_EndFrame();
  ImGui_Render();
  cImGui_ImplWGPU_RenderDrawData(ImGui_GetDrawData(), renderPass);
}

#endif // Application_gui_H_
