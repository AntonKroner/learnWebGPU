#ifndef Application_H_
#define Application_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tgmath.h>
#include "library/webgpu.h"
#include "./library/glfw3webgpu/glfw3webgpu.h"
#include "./library/glfw/include/GLFW/glfw3.h"
#include "./library/tinyobj_loader_c.h"
#include "./adapter.h"
#include "./device.h"
#include "linearAlgebra.h"
#include "Camera.h"

typedef float Vec3[3];
typedef float Vec2[2];
typedef struct {
    Vec3 position;
    Vec3 normal;
    Vec3 color;
    Vec2 uv;
} Vertex;
typedef struct {
    Vertex* vertices;
    size_t vertexCount;
} Model;
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
bool Application_shouldClose(Application* application);
void Application_render(Application application[static 1]);
void Application_destroy(Application* application);

static void loadFile(
  void* /* ctx */,
  const char* filename,
  const int /* is_mtl */,
  const char* /* obj_filename */,
  char** buffer,
  size_t* length) {
  long string_size = 0, read_size = 0;
  FILE* handler = fopen(filename, "r");
  if (handler) {
    fseek(handler, 0, SEEK_END);
    string_size = ftell(handler);
    rewind(handler);
    *buffer = (char*)malloc(sizeof(char) * (string_size + 1));
    read_size = fread(*buffer, sizeof(char), (size_t)string_size, handler);
    (*buffer)[string_size] = '\0';
    if (string_size != read_size) {
      free(*buffer);
      *buffer = NULL;
    }
    fclose(handler);
  }
  *length = read_size;
}
static Model Model_load(char* const file) {
  tinyobj_shape_t* shapes = 0;
  tinyobj_material_t* materials = 0;
  tinyobj_attrib_t attributes;
  size_t shapesCount;
  size_t materialsCount;
  tinyobj_attrib_init(&attributes);
  Model result = { .vertexCount = 0, .vertices = 0 };
  const int loaded = tinyobj_parse_obj(
    &attributes,
    &shapes,
    &shapesCount,
    &materials,
    &materialsCount,
    file,
    loadFile,
    0,
    TINYOBJ_FLAG_TRIANGULATE);
  if (loaded == TINYOBJ_SUCCESS) {
    result.vertexCount = attributes.num_faces;
    result.vertices = calloc(attributes.num_faces, sizeof(Vertex));
    for (size_t i = 0; attributes.num_faces > i; i++) {
      const tinyobj_vertex_index_t face = attributes.faces[i];
      result.vertices[i].color[0] = 1.0;
      result.vertices[i].color[1] = 1.0;
      result.vertices[i].color[2] = 1.0;
      result.vertices[i].position[0] = attributes.vertices[3 * face.v_idx];
      result.vertices[i].position[1] = -1 * attributes.vertices[3 * face.v_idx + 2];
      result.vertices[i].position[2] = attributes.vertices[3 * face.v_idx + 1];
      result.vertices[i].normal[0] = attributes.normals[3 * face.vn_idx];
      result.vertices[i].normal[1] = -1 * attributes.normals[3 * face.vn_idx + 2];
      result.vertices[i].normal[2] = attributes.normals[3 * face.vn_idx + 1];
      result.vertices[i].uv[0] = attributes.texcoords[2 * face.vt_idx];
      result.vertices[i].uv[1] = 1 - attributes.texcoords[2 * face.vt_idx + 1];
    }
    tinyobj_attrib_free(&attributes);
    if (shapes) {
      tinyobj_shapes_free(shapes, shapesCount);
    }
    if (materials) {
      printf("materials count: %zu\n", materialsCount);
      tinyobj_materials_free(materials, materialsCount);
    }
  }
  return result;
}
static void Model_unload(Model* model) {
  if (model->vertices) {
    free(model->vertices);
  }
  model->vertexCount = 0;
}
static void limitsSet(
  WGPURequiredLimits required[static 1],
  WGPUSupportedLimits supported) {
  required->limits.maxVertexAttributes = 4;
  required->limits.maxVertexBuffers = 2;
  required->limits.maxBufferSize = 150000 * 44; // FIXME
  required->limits.maxVertexBufferArrayStride = 44; // FIXME
  required->limits.minStorageBufferOffsetAlignment =
    supported.limits.minStorageBufferOffsetAlignment;
  required->limits.minUniformBufferOffsetAlignment =
    supported.limits.minUniformBufferOffsetAlignment;
  required->limits.maxInterStageShaderComponents = 8;
  required->limits.maxBindGroups = 1;
  required->limits.maxUniformBuffersPerShaderStage = 1;
  required->limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
  required->limits.maxTextureDimension1D = 2048;
  required->limits.maxTextureDimension2D = 2048;
  required->limits.maxTextureArrayLayers = 1;
  required->limits.maxSampledTexturesPerShaderStage = 1;
  required->limits.maxSamplersPerShaderStage = 1;
}
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
static void onResize(GLFWwindow* window, int width, int height) {
  Application* application = (Application*)glfwGetWindowUserPointer(window);
  if (application) {
    wgpuTextureDestroy(application->depth.texture);
    wgpuTextureRelease(application->depth.texture);
    wgpuTextureViewRelease(application->depth.view);
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
    wgpuSwapChainRelease(application->swapChain);
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
    && !(result->window = glfwCreateWindow(640, 480, "Application", NULL, NULL))) {
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
    WGPUSupportedLimits supported = { .nextInChain = 0 };
    wgpuAdapterGetLimits(adapter, &supported);
    WGPURequiredLimits required = { .nextInChain = 0, .limits = supported.limits };
    limitsSet(&required, supported);
    WGPUDeviceDescriptor deviceDescriptor = {
      .nextInChain = 0,
      .label = "Device 1",
      .requiredFeaturesCount = 0,
      .requiredLimits = &required,
      .defaultQueue = { .label = "default queueuue" }
    };
    result->device = device_request(adapter, &deviceDescriptor);
    wgpuAdapterRelease(adapter);
    result->queue = wgpuDeviceGetQueue(result->device);
    WGPUSwapChainDescriptor swapChainDescriptor = {
      .nextInChain = 0,
      .width = 640,
      .height = 480,
      .usage = WGPUTextureUsage_RenderAttachment,
      .format = WGPUTextureFormat_BGRA8Unorm,
      .presentMode = WGPUPresentMode_Fifo,
    };
    result->swapChain =
      wgpuDeviceCreateSwapChain(result->device, result->surface, &swapChainDescriptor);
    result->shader =
      device_ShaderModule(result->device, RESOURCE_DIR "/texturing/sampler.wgsl");
    result->texture.texture = device_Texture_load(
      result->device,
      RESOURCE_DIR "/texturing/fourareen/fourareen2K_albedo.jpg",
      &result->texture.view);
    Model model = Model_load(RESOURCE_DIR "/texturing/fourareen/fourareen.obj");
    result->vertexCount = model.vertexCount;
    WGPUBufferDescriptor vertexBufferDescriptor = {
      .nextInChain = 0,
      .label = "vertex buffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
      .size = model.vertexCount * sizeof(Vertex),
      .mappedAtCreation = false,
    };
    result->buffers.vertex =
      wgpuDeviceCreateBuffer(result->device, &vertexBufferDescriptor);
    wgpuQueueWriteBuffer(
      result->queue,
      result->buffers.vertex,
      0,
      model.vertices,
      vertexBufferDescriptor.size);
    Model_unload(&model);

    WGPUBufferDescriptor uniformBufferDescriptor = {
      .nextInChain = 0,
      .label = "uniformBuffer",
      .size = sizeof(Uniforms),
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
      .mappedAtCreation = false
    };
    result->buffers.uniform =
      wgpuDeviceCreateBuffer(result->device, &uniformBufferDescriptor);
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
    result->texture.sampler = wgpuDeviceCreateSampler(result->device, &samplerDescriptor);
    // depth
    result->depth.format = WGPUTextureFormat_Depth24Plus;
    WGPUTextureDescriptor depthTextureDescriptor = {
      .dimension = WGPUTextureDimension_2D,
      .format = result->depth.format,
      .mipLevelCount = 1,
      .sampleCount = 1,
      .size = {640, 480, 1},
      .usage = WGPUTextureUsage_RenderAttachment,
      .viewFormatCount = 1,
      .viewFormats = &result->depth.format,
    };
    result->depth.texture =
      wgpuDeviceCreateTexture(result->device, &depthTextureDescriptor);
    WGPUTextureViewDescriptor depthTextureViewDescriptor = {
      .aspect = WGPUTextureAspect_DepthOnly,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .dimension = WGPUTextureViewDimension_2D,
      .format = result->depth.format,
    };
    result->depth.view =
      wgpuTextureCreateView(result->depth.texture, &depthTextureViewDescriptor);
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
      .format = swapChainDescriptor.format,
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
       .offset = offsetof(Vertex, normal),
       },
      {
       // color
        .shaderLocation = 2,
       .format = WGPUVertexFormat_Float32x3,
       .offset = offsetof(Vertex, color),
       },
      {
       // uv coordinates
        .shaderLocation = 3,
       .format = WGPUVertexFormat_Float32x2,
       .offset = offsetof(Vertex, uv),
       }
    };
    WGPUVertexBufferLayout bufferLayout = {
      .attributeCount = 4,
      .attributes = vertexAttributes,
      .arrayStride = sizeof(Vertex),
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
bool Application_shouldClose(Application* application) {
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
    .clearValue = (WGPUColor){0.05, 0.05, 0.05, 1.0},
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
    application->vertexCount * sizeof(Vertex));
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
  wgpuBufferDestroy(application->buffers.vertex);
  wgpuBufferRelease(application->buffers.vertex);
  wgpuBufferDestroy(application->buffers.uniform);
  wgpuBufferRelease(application->buffers.uniform);
  wgpuTextureDestroy(application->texture.texture);
  wgpuTextureRelease(application->texture.texture);
  wgpuTextureViewRelease(application->texture.view);
  wgpuTextureDestroy(application->depth.texture);
  wgpuTextureRelease(application->depth.texture);
  wgpuTextureViewRelease(application->depth.view);
  wgpuSamplerRelease(application->texture.sampler);
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
