#ifndef Application_H_
#define Application_H_

#include <stdlib.h>
#include <stdio.h>
#include "./library/webgpu.h"
#include "./library/glfw3webgpu/glfw3webgpu.h"
#include "GLFW/glfw3.h"
#include "./adapter.h"
#include "./device.h"
#include "./Model.h"
#include "./linearAlgebra.h"
#include "./Camera.h"

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
} Application;

Application* Application_create();
bool Application_shouldClose(Application application[static 1]);
void Application_render(Application application[static 1]);
void Application_destroy(Application* application);

static WGPUDepthStencilState DepthStencilState_make() {
  WGPUStencilFaceState face = {
    .compare = WGPUCompareFunction_Always,
    .failOp = WGPUStencilOperation_Keep,
    .depthFailOp = WGPUStencilOperation_Keep,
    .passOp = WGPUStencilOperation_Keep,
  };
  WGPUDepthStencilState result = {
    .nextInChain = 0,
    .format = WGPUTextureFormat_Undefined,
    .depthWriteEnabled = false,
    .depthCompare = WGPUCompareFunction_Always,
    .stencilFront = face,
    .stencilBack = face,
    .stencilReadMask = 0,
    .stencilWriteMask = 0,
    .depthBias = 0,
    .depthBiasSlopeScale = 0,
    .depthBiasClamp = 0,
  };
  return result;
}
static WGPUBindGroupLayoutEntry BindGroupLayoutEntry_make() {
  WGPUBindGroupLayoutEntry result = {
    .buffer.nextInChain = 0,
    .buffer.type = WGPUBufferBindingType_Undefined,
    .buffer.minBindingSize = 0,
    .buffer.hasDynamicOffset = false,
    .sampler.nextInChain = 0,
    .sampler.type = WGPUSamplerBindingType_Undefined,
    .storageTexture.nextInChain = 0,
    .storageTexture.access = WGPUStorageTextureAccess_Undefined,
    .storageTexture.format = WGPUTextureFormat_Undefined,
    .storageTexture.viewDimension = WGPUTextureViewDimension_Undefined,
    .texture.nextInChain = 0,
    .texture.multisampled = false,
    .texture.sampleType = WGPUTextureSampleType_Undefined,
    .texture.viewDimension = WGPUTextureViewDimension_Undefined,
    .binding = 0,
    .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
  };
  return result;
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
  application->texture.texture = device_Texture_load(
    application->device,
    RESOURCE_DIR "/texturing/fourareen/fourareen2K_albedo.jpg",
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
    Camera_move(&application->camera, (float)x, (float)y);
    application->uniforms.matrices.view =
      Matrix4_transpose(Camera_viewGet(application->camera));
  }
}
static void onMouseButton(GLFWwindow* window, int button, int action, int /* mods*/) {
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    double x = 0;
    double y = 0;
    glfwGetCursorPos(window, &x, &y);
    Camera_activate(&application->camera, button, action, (float)x, (float)y);
  }
}
static void onMouseScroll(GLFWwindow* window, double x, double y) {
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    Camera_zoom(&application->camera, (float)x, (float)y);
    application->uniforms.matrices.view =
      Matrix4_transpose(Camera_viewGet(application->camera));
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
    WGPUAdapter adapter = adapter_request(result->instance, &adapterOptions);
    result->device = device_request(adapter);
    wgpuAdapterRelease(adapter);
    result->queue = wgpuDeviceGetQueue(result->device);
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(result->window, &width, &height);
    swapChain_attach(result, WIDTH, HEIGHT);
    texture_attach(result);
    depthBuffer_attach(result, WIDTH, HEIGHT);
    result->shader =
      device_ShaderModule(result->device, RESOURCE_DIR "/texturing/sampler.wgsl");
    Model model = Model_load(RESOURCE_DIR "/texturing/fourareen/fourareen.obj");
    result->vertexCount = model.vertexCount;
    buffers_attach(result, result->vertexCount);
    wgpuQueueWriteBuffer(
      result->queue,
      result->buffers.vertex,
      0,
      model.vertices,
      result->vertexCount * sizeof(Model_Vertex));
    Model_unload(&model);
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
    WGPUDepthStencilState depthStencilState = DepthStencilState_make();
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.format = result->depth.format;
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    WGPUBindGroupLayoutEntry bindingLayouts[] = {
      BindGroupLayoutEntry_make(),
      BindGroupLayoutEntry_make(),
      BindGroupLayoutEntry_make(),
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
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
      .nextInChain = 0,
      .entryCount = 3,
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
  }
  return result;
}
bool Application_shouldClose(Application application[static 1]) {
  return glfwWindowShouldClose(application->window);
}
void Application_render(Application application[static 1]) {
  glfwPollEvents();
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
  WGPURenderPassColorAttachment renderPassColorAttachment = {
    .view = nextTexture,
    .resolveTarget = 0,
    .loadOp = WGPULoadOp_Clear,
    .storeOp = WGPUStoreOp_Store,
    .clearValue = {.r = 0.05, .g = 0.05, .b = 0.05, .a = 1.0},
  };
  WGPURenderPassDepthStencilAttachment depthStencilAttachment = {
    .view = application->depth.view,
    .depthClearValue = 1.0f,
    .depthLoadOp = WGPULoadOp_Clear,
    .depthStoreOp = WGPUStoreOp_Store,
    .depthReadOnly = false,
    .stencilClearValue = 0,
    .stencilLoadOp = WGPULoadOp_Undefined,
    .stencilStoreOp = WGPUStoreOp_Undefined,
    .stencilReadOnly = true,
  };
  WGPURenderPassDescriptor renderPassDesc = {
    .nextInChain = 0,
    .colorAttachmentCount = 1,
    .colorAttachments = &renderPassColorAttachment,
    .depthStencilAttachment = &depthStencilAttachment,
    .timestampWriteCount = 0,
    .timestampWrites = 0,
  };
  WGPURenderPassEncoder renderPass =
    wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
  wgpuRenderPassEncoderSetPipeline(renderPass, application->pipeline);
  wgpuRenderPassEncoderSetVertexBuffer(
    renderPass,
    0,
    application->buffers.vertex,
    0,
    application->vertexCount * sizeof(Model_Vertex));
  wgpuRenderPassEncoderSetBindGroup(renderPass, 0, application->bindGroup, 0, 0);
  wgpuRenderPassEncoderDraw(renderPass, application->vertexCount, 1, 0, 0);
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
