#ifndef Application_H_
#define Application_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "webgpu.h"
#include "GLFW/glfw3.h"
#include "glfw3webgpu/glfw3webgpu.h"
#include "./adapter.h"
#include "./device.h"
#include "./BindGroupLayoutEntry.h"
#include "./DepthStencilState.h"
#include "./RenderPass.h"
#include "./Model.h"
#include "./Camera.h"
#include "./Lightning.h"
#include "../linearAlgebra.h"

#include "cimgui/cimgui.h"
#include "cimgui/cimgui_impl_wgpu.h"
#include "cimgui/cimgui_impl_glfw.h"

#define WIDTH (640)
#define HEIGHT (480)

typedef struct {
    struct {
        Matrix4 projection;
        Matrix4 view;
        Matrix4 model;
    } matrices;
    float color[4];
    float time;
    float _pad[3];
} Uniforms;
typedef struct {
    char* name;
    GLFWwindow* window;
    WGPUInstance instance;
    WGPUSurface surface;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSwapChain swapChain;
    WGPUShaderModule shader;
    struct {
        WGPUTextureFormat format;
        WGPUTexture texture;
        WGPUTextureView view;
    } depth;
    struct {
        WGPUSampler sampler;
        WGPUTexture texture;
        WGPUTextureView view;
    } texture;
    struct {
        WGPUBuffer vertex;
        WGPUBuffer uniform;
    } buffers;
    size_t vertexCount;
    WGPURenderPipeline pipeline;
    WGPUBindGroup bindGroup;
    Uniforms uniforms;
    Camera camera;
    Application_Lighting lightning;
} Application;

Application* Application_create();
bool Application_shouldClose(Application application[static 1]);
void Application_render(Application application[static 1]);
void Application_destroy(Application* application);

static bool gui_attach(Application application[static 1]) {
  // CIMGUI_CHECKVERSION();
  ImGui_CreateContext(0);
  ImGui_GetIO();
  cImGui_ImplGlfw_InitForOther(application->window, true);
  struct ImGui_ImplWGPU_InitInfo initInfo = {
    .Device = application->device,
    .NumFramesInFlight = 3,
    .RenderTargetFormat = WGPUTextureFormat_BGRA8Unorm,
    .DepthStencilFormat = application->depth.format,
    .PipelineMultisampleState = { .count = 1,
  .mask = UINT32_MAX,
  .alphaToCoverageEnabled = false,}};
  return cImGui_ImplWGPU_Init(&initInfo);
}
static void gui_render(
  Application application[static 1],
  WGPURenderPassEncoder renderPass) {
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
  ImGui_End();
  ImGui_Begin("Lighting", 0, 0);
  bool update = false;
  update =
    ImGui_ColorEdit3("Color #0", application->lightning.uniforms.colors[0], 0) || update;
  update = ImGui_DragFloat3("Direction #0", application->lightning.uniforms.directions[0])
           || update;
  update =
    ImGui_ColorEdit3("Color #1", application->lightning.uniforms.colors[1], 0) || update;
  update = ImGui_DragFloat3("Direction #1", application->lightning.uniforms.directions[1])
           || update;
  application->lightning.update = update;
  ImGui_End();
  ImGui_EndFrame();
  ImGui_Render();
  cImGui_ImplWGPU_RenderDrawData(ImGui_GetDrawData(), renderPass);
}
static void gui_detach(Application application[static 1]) {
  cImGui_ImplGlfw_Shutdown();
  cImGui_ImplWGPU_Shutdown();
}
static void buffers_attach(Application application[static 1], size_t vertexCount) {
  WGPUBufferDescriptor vertexBufferDescriptor = {
    .nextInChain = 0,
    .label = "vertex buffer",
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
    .mappedAtCreation = false,
    .size = vertexCount * sizeof(Model_Vertex),
  };
  application->buffers.vertex =
    wgpuDeviceCreateBuffer(application->device, &vertexBufferDescriptor);
  WGPUBufferDescriptor uniformBufferDescriptor = {
    .nextInChain = 0,
    .label = "uniform buffer",
    .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
    .mappedAtCreation = false,
    .size = sizeof(Uniforms),
  };
  application->buffers.uniform =
    wgpuDeviceCreateBuffer(application->device, &uniformBufferDescriptor);
}
static void buffers_detach(Application application[static 1]) {
  wgpuBufferDestroy(application->buffers.vertex);
  wgpuBufferRelease(application->buffers.vertex);
  wgpuBufferDestroy(application->buffers.uniform);
  wgpuBufferRelease(application->buffers.uniform);
}
static void texture_attach(Application application[static 1]) {
  application->texture.texture = Application_device_Texture_load(
    application->device,
    RESOURCE_DIR "/fourareen/fourareen2K_albedo.jpg",
    &application->texture.view);
  WGPUSamplerDescriptor samplerDescriptor = {
    .addressModeU = WGPUAddressMode_ClampToEdge,
    .addressModeV = WGPUAddressMode_ClampToEdge,
    .addressModeW = WGPUAddressMode_ClampToEdge,
    .magFilter = WGPUFilterMode_Linear,
    .minFilter = WGPUFilterMode_Linear,
    .mipmapFilter = WGPUMipmapFilterMode_Linear,
    .lodMinClamp = 0.0f,
    .lodMaxClamp = 1.0f,
    .compare = WGPUCompareFunction_Undefined,
    .maxAnisotropy = 1,
  };
  application->texture.sampler =
    wgpuDeviceCreateSampler(application->device, &samplerDescriptor);
}
static void texture_detach(Application application[static 1]) {
  wgpuTextureDestroy(application->texture.texture);
  wgpuTextureRelease(application->texture.texture);
  wgpuTextureViewRelease(application->texture.view);
  wgpuSamplerRelease(application->texture.sampler);
}
static void depthBuffer_attach(Application application[static 1], int width, int height) {
  application->depth.format = WGPUTextureFormat_Depth24Plus;
  WGPUTextureDescriptor depthTextureDescriptor = {
    .dimension = WGPUTextureDimension_2D,
    .format = application->depth.format,
    .mipLevelCount = 1,
    .sampleCount = 1,
    .size = {width, height, 1},
    .usage = WGPUTextureUsage_RenderAttachment,
    .viewFormatCount = 1,
    .viewFormats = &application->depth.format,
  };
  application->depth.texture =
    wgpuDeviceCreateTexture(application->device, &depthTextureDescriptor);
  WGPUTextureViewDescriptor depthTextureViewDescriptor = {
    .aspect = WGPUTextureAspect_DepthOnly,
    .baseArrayLayer = 0,
    .arrayLayerCount = 1,
    .baseMipLevel = 0,
    .mipLevelCount = 1,
    .dimension = WGPUTextureViewDimension_2D,
    .format = application->depth.format,
  };
  application->depth.view =
    wgpuTextureCreateView(application->depth.texture, &depthTextureViewDescriptor);
}
static void depthBuffer_detach(Application application[static 1]) {
  wgpuTextureDestroy(application->depth.texture);
  wgpuTextureRelease(application->depth.texture);
  wgpuTextureViewRelease(application->depth.view);
}
static void swapChain_attach(Application application[static 1], int width, int height) {
  if (application->swapChain) {
    wgpuSwapChainRelease(application->swapChain);
  }
  WGPUSwapChainDescriptor swapChainDescriptor = {
    .nextInChain = 0,
    .width = width,
    .height = height,
    .usage = WGPUTextureUsage_RenderAttachment,
    .format = WGPUTextureFormat_BGRA8Unorm,
    .presentMode = WGPUPresentMode_Fifo,
  };
  application->swapChain = wgpuDeviceCreateSwapChain(
    application->device,
    application->surface,
    &swapChainDescriptor);
}
static void onResize(GLFWwindow* window, int width, int height) {
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    swapChain_attach(application, width, height);
    depthBuffer_detach(application);
    depthBuffer_attach(application, width, height);
    application->uniforms.matrices.projection = Matrix4_transpose(
      Matrix4_perspective(45, ((float)width / (float)height), 0.01f, 100.0f));
  }
}
static void onMouseMove(GLFWwindow* window, double x, double y) {
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    Application_Camera_move(&application->camera, (float)x, (float)y);
    application->uniforms.matrices.view =
      Matrix4_transpose(Application_Camera_viewGet(application->camera));
  }
}
static void onMouseButton(GLFWwindow* window, int button, int action, int /* mods*/) {
  ImGuiIO* io = ImGui_GetIO();
  if (io->WantCaptureMouse) {
    return;
  }
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    double x = 0;
    double y = 0;
    glfwGetCursorPos(window, &x, &y);
    Application_Camera_activate(&application->camera, button, action, (float)x, (float)y);
  }
}
static void onMouseScroll(GLFWwindow* window, double x, double y) {
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    Application_Camera_zoom(&application->camera, (float)x, (float)y);
    application->uniforms.matrices.view =
      Matrix4_transpose(Application_Camera_viewGet(application->camera));
  }
}
static bool setWindowHints() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  return true;
}
Application* Application_create() {
  WGPUInstanceDescriptor descriptor = { .nextInChain = 0 };
  Application* result = calloc(1, sizeof(*result));
  if (!result) {
    perror("Application allocation failed.");
    result = 0;
  }
  else if (!glfwInit()) {
    free(result);
    perror("Could not initialize GLFW.");
    result = 0;
  }
  else if (!(result->instance = wgpuCreateInstance(&descriptor))) {
    glfwTerminate();
    free(result);
    perror("Could not initialize WebGPU!");
    result = 0;
  }
  else if (
    setWindowHints()
    && !(result->window = glfwCreateWindow(WIDTH, HEIGHT, "Application", NULL, NULL))) {
    wgpuInstanceRelease(result->instance);
    glfwTerminate();
    free(result);
    perror("Could not open window!");
    result = 0;
  }
  else {
    glfwSetWindowUserPointer(result->window, result);
    glfwSetFramebufferSizeCallback(result->window, onResize);
    glfwSetCursorPosCallback(result->window, onMouseMove);
    glfwSetMouseButtonCallback(result->window, onMouseButton);
    glfwSetScrollCallback(result->window, onMouseScroll);
    result->surface = glfwGetWGPUSurface(result->instance, result->window);
    WGPURequestAdapterOptions adapterOptions = {
      .nextInChain = 0,
      .compatibleSurface = result->surface,
    };
    WGPUAdapter adapter = Application_adapter_request(result->instance, &adapterOptions);
    result->device = Application_device_request(adapter);
    wgpuAdapterRelease(adapter);
    result->queue = wgpuDeviceGetQueue(result->device);
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(result->window, &width, &height);
    swapChain_attach(result, WIDTH, HEIGHT);
    texture_attach(result);
    depthBuffer_attach(result, WIDTH, HEIGHT);
    result->shader = Application_device_ShaderModule(
      result->device,
      RESOURCE_DIR "/lightning/control.wgsl");
    Model model = Model_load(RESOURCE_DIR "/fourareen/fourareen.obj");
    result->vertexCount = model.vertexCount;
    buffers_attach(result, result->vertexCount);
    wgpuQueueWriteBuffer(
      result->queue,
      result->buffers.vertex,
      0,
      model.vertices,
      result->vertexCount * sizeof(Model_Vertex));
    Model_unload(&model);
    result->lightning = Application_Lightning_create(result->device);
    // pipeline
    WGPUBlendState blendState = {
      .color.srcFactor = WGPUBlendFactor_SrcAlpha,
      .color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
      .color.operation = WGPUBlendOperation_Add,
      .alpha.srcFactor = WGPUBlendFactor_Zero,
      .alpha.dstFactor = WGPUBlendFactor_One,
      .alpha.operation = WGPUBlendOperation_Add
    };
    WGPUColorTargetState colorTarget = {
      .nextInChain = 0,
      .format = WGPUTextureFormat_BGRA8Unorm,
      .blend = &blendState,
      .writeMask = WGPUColorWriteMask_All,
    };
    WGPUFragmentState fragmentState = {
      .nextInChain = 0,
      .module = result->shader,
      .entryPoint = "fs_main",
      .constantCount = 0,
      .constants = 0,
      .targetCount = 1,
      .targets = &colorTarget,
    };
    WGPUDepthStencilState depthStencilState = Application_DepthStencilState_make();
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.format = result->depth.format;
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    WGPUBindGroupLayoutEntry bindingLayouts[] = {
      Application_BindGroupLayoutEntry_make(),
      Application_BindGroupLayoutEntry_make(),
      Application_BindGroupLayoutEntry_make(),
      Application_BindGroupLayoutEntry_make(),
    };
    bindingLayouts[0].buffer.type = WGPUBufferBindingType_Uniform;
    bindingLayouts[0].buffer.minBindingSize = sizeof(Uniforms);
    bindingLayouts[1].binding = 1;
    bindingLayouts[1].visibility = WGPUShaderStage_Fragment;
    bindingLayouts[1].texture.sampleType = WGPUTextureSampleType_Float;
    bindingLayouts[1].texture.viewDimension = WGPUTextureViewDimension_2D;
    bindingLayouts[2].binding = 2;
    bindingLayouts[2].visibility = WGPUShaderStage_Fragment;
    bindingLayouts[2].sampler.type = WGPUSamplerBindingType_Filtering;
    bindingLayouts[3].binding = 3;
    bindingLayouts[3].visibility = WGPUShaderStage_Fragment;
    bindingLayouts[3].buffer.type = WGPUBufferBindingType_Uniform;
    bindingLayouts[3].buffer.minBindingSize = sizeof(Application_Lighting_Uniforms);
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
      .nextInChain = 0,
      .entryCount = 4,
      .entries = bindingLayouts,
    };
    WGPUBindGroupLayout bindGroupLayout =
      wgpuDeviceCreateBindGroupLayout(result->device, &bindGroupLayoutDescriptor);
    WGPUPipelineLayoutDescriptor layoutDescriptor = {
      .nextInChain = 0,
      .bindGroupLayoutCount = 1,
      .bindGroupLayouts = &bindGroupLayout,
    };
    WGPUPipelineLayout layout =
      wgpuDeviceCreatePipelineLayout(result->device, &layoutDescriptor);
    WGPUVertexAttribute vertexAttributes[] = {
      {
       // position
        .shaderLocation = 0,
       .format = WGPUVertexFormat_Float32x3,
       .offset = 0,
       },
      {
       // normal
        .shaderLocation = 1,
       .format = WGPUVertexFormat_Float32x3,
       .offset = offsetof(Model_Vertex, normal),
       },
      {
       // color
        .shaderLocation = 2,
       .format = WGPUVertexFormat_Float32x3,
       .offset = offsetof(Model_Vertex, color),
       },
      {
       // uv coordinates
        .shaderLocation = 3,
       .format = WGPUVertexFormat_Float32x2,
       .offset = offsetof(Model_Vertex, uv),
       }
    };
    WGPUVertexBufferLayout bufferLayout = {
      .attributeCount = 4,
      .attributes = vertexAttributes,
      .arrayStride = sizeof(Model_Vertex),
      .stepMode = WGPUVertexStepMode_Vertex,
    };
    WGPURenderPipelineDescriptor pipelineDesc = {
      .nextInChain = 0,
      .fragment = &fragmentState,
      .vertex.bufferCount = 1,
      .vertex.buffers = &bufferLayout,
      .vertex.module = result->shader,
      .vertex.entryPoint = "vs_main",
      .vertex.constantCount = 0,
      .vertex.constants = 0,
      .primitive.topology = WGPUPrimitiveTopology_TriangleList,
      .primitive.stripIndexFormat = WGPUIndexFormat_Undefined,
      .primitive.frontFace = WGPUFrontFace_CCW,
      .primitive.cullMode = WGPUCullMode_None,
      .depthStencil = &depthStencilState,
      .multisample.count = 1,
      .multisample.mask = ~0u,
      .multisample.alphaToCoverageEnabled = false,
      .layout = layout,
    };
    result->pipeline = wgpuDeviceCreateRenderPipeline(result->device, &pipelineDesc);
    // bind group
    WGPUBindGroupEntry bindings[] = {
      {
       .nextInChain = 0,
       .binding = 0,
       .buffer = result->buffers.uniform,
       .offset = 0,
       .size = sizeof(Uniforms),
       },
      {
       .nextInChain = 0,
       .binding = 1,
       .textureView = result->texture.view,
       },
      {
       .nextInChain = 0,
       .binding = 2,
       .sampler = result->texture.sampler,
       },
      {
       .nextInChain = 0,
       .binding = 3,
       .buffer = result->lightning.buffer,
       .offset = 0,
       .size = sizeof(Application_Lighting_Uniforms),
       }
    };
    WGPUBindGroupDescriptor bindGroupDescriptor = {
      .nextInChain = 0,
      .layout = bindGroupLayout,
      .entryCount = bindGroupLayoutDescriptor.entryCount,
      .entries = bindings,
    };
    result->bindGroup = wgpuDeviceCreateBindGroup(result->device, &bindGroupDescriptor);
    result->camera.position.x = -2.0f;
    result->camera.position.y = -3.0f;
    result->camera.zoom = -1.2;
    Uniforms uniforms = {
      .matrices.model = Matrix4_transpose(Matrix4_diagonal(1.0)),
      .matrices.view = Matrix4_transpose(Matrix4_lookAt(
        Vector3_make(-2.0f, -3.0f, 2.0f),
        Vector3_fill(0.0f),
        Vector3_make(0, 0, 1.0f))),
      .matrices.projection =
        Matrix4_transpose(Matrix4_perspective(45, 640.0f / 480.0f, 0.01f, 100.0f)),
      .time = 0.0f,
      .color = {0.0f, 1.0f, 0.4f, 1.0f},
    };
    result->uniforms = uniforms;
    if (!gui_attach(result)) {
      printf("gui problem!!\n");
    }
    Application_Lightning_update(&result->lightning, result->queue);
  }
  return result;
}
bool Application_shouldClose(Application application[static 1]) {
  return glfwWindowShouldClose(application->window);
}
void Application_render(Application application[static 1]) {
  glfwPollEvents();
  Application_Lightning_update(&application->lightning, application->queue);
  WGPUTextureView nextTexture =
    wgpuSwapChainGetCurrentTextureView(application->swapChain);
  if (!nextTexture) {
    perror("Cannot acquire next swap chain texture\n");
    return;
  }
  application->uniforms.time = (float)glfwGetTime();
  wgpuQueueWriteBuffer(
    application->queue,
    application->buffers.uniform,
    0,
    &application->uniforms,
    sizeof(Uniforms));
  WGPUCommandEncoderDescriptor commandEncoderDesc = {
    .nextInChain = 0,
    .label = "Command Encoder",
  };
  WGPUCommandEncoder encoder =
    wgpuDeviceCreateCommandEncoder(application->device, &commandEncoderDesc);
  WGPURenderPassEncoder renderPass =
    Application_RenderPassEncoder_make(encoder, nextTexture, application->depth.view);
  wgpuRenderPassEncoderSetPipeline(renderPass, application->pipeline);
  wgpuRenderPassEncoderSetVertexBuffer(
    renderPass,
    0,
    application->buffers.vertex,
    0,
    application->vertexCount * sizeof(Model_Vertex));
  wgpuRenderPassEncoderSetBindGroup(renderPass, 0, application->bindGroup, 0, 0);
  wgpuRenderPassEncoderDraw(renderPass, application->vertexCount, 1, 0, 0);
  gui_render(application, renderPass);
  wgpuRenderPassEncoderEnd(renderPass);
  wgpuTextureViewRelease(nextTexture);
  WGPUCommandBufferDescriptor cmdBufferDescriptor = {
    .nextInChain = 0,
    .label = "command buffer",
  };
  WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
  wgpuCommandEncoderRelease(encoder);
  wgpuQueueSubmit(application->queue, 1, &command);
  wgpuCommandBufferRelease(command);
  wgpuSwapChainPresent(application->swapChain);
  wgpuDeviceTick(application->device);
}
void Application_destroy(Application* application) {
  Application_Lightning_destroy(application->lightning);
  gui_detach(application);
  buffers_detach(application);
  texture_detach(application);
  depthBuffer_detach(application);
  wgpuBindGroupRelease(application->bindGroup);
  wgpuRenderPipelineRelease(application->pipeline);
  wgpuShaderModuleRelease(application->shader);
  wgpuSwapChainRelease(application->swapChain);
  wgpuQueueRelease(application->queue);
  wgpuDeviceRelease(application->device);
  wgpuInstanceRelease(application->instance);
  wgpuSurfaceRelease(application->surface);
  glfwDestroyWindow(application->window);
  glfwTerminate();
  free(application);
}

#endif // Application_H_
