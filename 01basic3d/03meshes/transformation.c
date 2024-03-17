#include "../basic3d.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "../../library/glfw/include/GLFW/glfw3.h"
#include "../../library/webgpu.h"
#include "../../library/glfw3webgpu/glfw3webgpu.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "../../library/tinyobj_loader_c.h"
#include "../../adapter.h"
#include "../../device.h"
typedef float Vec3[3];
typedef struct {
    Vec3 position;
    Vec3 normal;
    Vec3 color;
} Vertex;
typedef struct {
    Vertex* vertices;
    size_t vertexCount;
} Model;
typedef struct {
    float color[4];
    float time;
    float _pad[3];
} Uniforms;

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
void Model_destroy(Model* model) {
  if (model->vertices) {
    free(model->vertices);
  }
  model->vertexCount = 0;
}
static bool setWindowHints() {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  return true;
}
static void limitsSet(
  WGPURequiredLimits required[static 1],
  WGPUSupportedLimits supported) {
  required->limits.maxVertexAttributes = 3;
  required->limits.maxVertexBuffers = 2;
  required->limits.maxBufferSize = 16 * sizeof(Vertex);
  required->limits.maxVertexBufferArrayStride = sizeof(Vertex);
  required->limits.minStorageBufferOffsetAlignment =
    supported.limits.minStorageBufferOffsetAlignment;
  required->limits.minUniformBufferOffsetAlignment =
    supported.limits.minUniformBufferOffsetAlignment;
  required->limits.maxInterStageShaderComponents = 6;
  required->limits.maxBindGroups = 1;
  required->limits.maxUniformBuffersPerShaderStage = 1;
  required->limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
  required->limits.maxTextureDimension1D = 480;
  required->limits.maxTextureDimension2D = 640;
  required->limits.maxTextureArrayLayers = 1;
}
static WGPUBindGroupLayoutEntry bindingLayoutCreate(size_t size) {
  WGPUBindGroupLayoutEntry result = {
    .buffer.nextInChain = 0,
    .buffer.type = WGPUBufferBindingType_Uniform,
    .buffer.minBindingSize = size,
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
static WGPUDepthStencilState depthStencilStateCreate() {
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
bool basic3d_meshes_transformation() {
  bool result = false;
  WGPUInstanceDescriptor descriptor = { .nextInChain = 0 };
  WGPUInstance instance = 0;
  GLFWwindow* window = 0;
  const Model model = Model_load(RESOURCE_DIR "/meshes/mammoth.obj");
  if (!glfwInit()) {
    perror("Could not initialize GLFW!");
  }
  else if (!(instance = wgpuCreateInstance(&descriptor))) {
    glfwTerminate();
    perror("Could not initialize WebGPU!");
  }
  else if (
    setWindowHints()
    && !(window = glfwCreateWindow(640, 480, "Basic 3D: First Uniform", NULL, NULL))) {
    wgpuInstanceRelease(instance);
    glfwTerminate();
    perror("Could not open window!");
    result = false;
  }
  else {
    WGPUSurface surface = glfwGetWGPUSurface(instance, window);
    WGPURequestAdapterOptions adapterOptions = {
      .nextInChain = 0,
      .compatibleSurface = surface,
    };
    WGPUAdapter adapter = adapter_request(instance, &adapterOptions);
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
    WGPUDevice device = device_request(adapter, &deviceDescriptor);
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    WGPUBufferDescriptor uniformBufferDescriptor = {
      .nextInChain = 0,
      .label = "uniformBuffer",
      .size = sizeof(Uniforms),
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
      .mappedAtCreation = false
    };
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(device, &uniformBufferDescriptor);
    Uniforms uniforms = {
      .time = 0.0f,
      .color = {0.0f, 1.0f, 0.4f, 1.0f},
    };
    WGPUBindGroupLayoutEntry bindingLayout = bindingLayoutCreate(sizeof(Uniforms));
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDescriptor = {
      .nextInChain = 0,
      .entryCount = 1,
      .entries = &bindingLayout,
    };
    WGPUBindGroupLayout bindGroupLayout =
      wgpuDeviceCreateBindGroupLayout(device, &bindGroupLayoutDescriptor);
    WGPUPipelineLayoutDescriptor layoutDescriptor = {
      .nextInChain = 0,
      .bindGroupLayoutCount = 1,
      .bindGroupLayouts = &bindGroupLayout,
    };
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(device, &layoutDescriptor);
    WGPUBindGroupEntry binding = {
      .nextInChain = 0,
      .binding = 0,
      .buffer = uniformBuffer,
      .offset = 0,
      .size = sizeof(Uniforms),
    };
    WGPUBindGroupDescriptor bindGroupDescriptor = {
      .nextInChain = 0,
      .layout = bindGroupLayout,
      .entryCount = bindGroupLayoutDescriptor.entryCount,
      .entries = &binding,
    };
    WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(device, &bindGroupDescriptor);
    WGPUBufferDescriptor vertexBufferDescriptor = {
      .nextInChain = 0,
      .label = "vertex buffer",
      .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex,
      .size = model.vertexCount * sizeof(Vertex),
      .mappedAtCreation = false,
    };
    WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(device, &vertexBufferDescriptor);
    wgpuQueueWriteBuffer(
      queue,
      vertexBuffer,
      0,
      model.vertices,
      vertexBufferDescriptor.size);
    WGPUSwapChainDescriptor swapChainDescriptor = {
      .nextInChain = 0,
      .width = 640,
      .height = 480,
      .usage = WGPUTextureUsage_RenderAttachment,
      .format = WGPUTextureFormat_BGRA8Unorm,
      .presentMode = WGPUPresentMode_Fifo,
    };
    WGPUSwapChain swapChain =
      wgpuDeviceCreateSwapChain(device, surface, &swapChainDescriptor);
    WGPUShaderModule shaderModule =
      device_ShaderModule(device, RESOURCE_DIR "/meshes/transformation.wgsl");
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
      .module = shaderModule,
      .entryPoint = "fs_main",
      .constantCount = 0,
      .constants = 0,
      .targetCount = 1,
      .targets = &colorTarget,
    };
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
       }
    };

    WGPUVertexBufferLayout bufferLayout = {
      .attributeCount = 3,
      .attributes = vertexAttributes,
      .arrayStride = sizeof(Vertex),
      .stepMode = WGPUVertexStepMode_Vertex,
    };
    WGPUDepthStencilState depthStencilState = depthStencilStateCreate();
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
    depthStencilState.depthWriteEnabled = true;
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;
    depthStencilState.format = depthTextureFormat;
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;

    WGPUTextureDescriptor depthTextureDesc = {
      .dimension = WGPUTextureDimension_2D,
      .format = depthTextureFormat,
      .mipLevelCount = 1,
      .sampleCount = 1,
      .size = {640, 480, 1},
      .usage = WGPUTextureUsage_RenderAttachment,
      .viewFormatCount = 1,
      .viewFormats = &depthTextureFormat,
    };
    WGPUTexture depthTexture = wgpuDeviceCreateTexture(device, &depthTextureDesc);
    WGPUTextureViewDescriptor depthTextureViewDesc = {
      .aspect = WGPUTextureAspect_DepthOnly,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .dimension = WGPUTextureViewDimension_2D,
      .format = depthTextureFormat,
    };
    WGPUTextureView depthTextureView =
      wgpuTextureCreateView(depthTexture, &depthTextureViewDesc);

    WGPURenderPipelineDescriptor pipelineDesc = {
      .nextInChain = 0,
      .fragment = &fragmentState,
      .vertex.bufferCount = 1,
      .vertex.buffers = &bufferLayout,
      .vertex.module = shaderModule,
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
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(swapChain);
      if (!nextTexture) {
        perror("Cannot acquire next swap chain texture\n");
        break;
      }

      uniforms.time = (float)glfwGetTime();
      wgpuQueueWriteBuffer(
        queue,
        uniformBuffer,
        sizeof(uniforms.color),
        &uniforms.time,
        sizeof(float));
      wgpuQueueWriteBuffer(
        queue,
        uniformBuffer,
        0,
        &uniforms.color,
        sizeof(uniforms.color));

      WGPUCommandEncoderDescriptor commandEncoderDesc = {
        .nextInChain = 0,
        .label = "Command Encoder",
      };
      WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
      WGPURenderPassColorAttachment renderPassColorAttachment = {
        .view = nextTexture,
        .resolveTarget = 0,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = (WGPUColor){0.05, 0.05, 0.05, 1.0},
      };

      WGPURenderPassDepthStencilAttachment depthStencilAttachment = {
        .view = depthTextureView,
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
      wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
      wgpuRenderPassEncoderSetVertexBuffer(
        renderPass,
        0,
        vertexBuffer,
        0,
        vertexBufferDescriptor.size);
      wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bindGroup, 0, 0);
      wgpuRenderPassEncoderDraw(renderPass, model.vertexCount, 1, 0, 0);
      wgpuRenderPassEncoderEnd(renderPass);
      wgpuTextureViewRelease(nextTexture);
      WGPUCommandBufferDescriptor cmdBufferDescriptor = {
        .nextInChain = 0,
        .label = "command buffer",
      };
      WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
      wgpuCommandEncoderRelease(encoder);
      wgpuQueueSubmit(queue, 1, &command);
      wgpuCommandBufferRelease(command);
      wgpuSwapChainPresent(swapChain);
      wgpuDeviceTick(device);
    }
    Model_destroy(&model);
    wgpuTextureViewRelease(depthTextureView);
    wgpuTextureDestroy(depthTexture);
    wgpuTextureRelease(depthTexture);

    wgpuBufferDestroy(vertexBuffer);
    wgpuBufferRelease(vertexBuffer);
    wgpuBufferDestroy(uniformBuffer);
    wgpuBufferRelease(uniformBuffer);

    wgpuQueueRelease(queue);
    wgpuRenderPipelineRelease(pipeline);
    wgpuShaderModuleRelease(shaderModule);
    wgpuSwapChainRelease(swapChain);
    wgpuDeviceRelease(device);
    wgpuAdapterRelease(adapter);
    wgpuInstanceRelease(instance);
    wgpuSurfaceRelease(surface);
    glfwDestroyWindow(window);
    glfwTerminate();
    result = true;
  }
  return result;
}
