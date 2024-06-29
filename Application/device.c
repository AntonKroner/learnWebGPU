#include "device.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <tgmath.h>
#include "webgpu.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
    WGPUDevice device;
    bool done;
} Response;

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
  required->limits.maxInterStageShaderComponents = 11;
  required->limits.maxBindGroups = 1;
  required->limits.maxUniformBuffersPerShaderStage = 1;
  required->limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
  required->limits.maxTextureDimension1D = 2048;
  required->limits.maxTextureDimension2D = 2048;
  required->limits.maxTextureArrayLayers = 1;
  required->limits.maxSampledTexturesPerShaderStage = 1;
  required->limits.maxSamplersPerShaderStage = 1;
}
static void device_onRequest(
  WGPURequestDeviceStatus status,
  WGPUDevice device,
  const char* message,
  void* inputResponse) {
  Response* response = (Response*)inputResponse;
  if (status != WGPURequestDeviceStatus_Success) {
    printf("Could not get WebGPU device: %s\n", message);
  }
  else {
    response->device = device;
  }
  response->done = true;
}
static char* logTypeStringify(WGPULoggingType type);
static void onLog(WGPULoggingType type, const char* message, void* /* userdata */) {
  printf("Device log %s: ", logTypeStringify(type));
  if (message) {
    printf("%s ", message);
  }
  printf("\n");
}
static char* errorStringify(WGPUErrorType error);
static void onDeviceError(WGPUErrorType type, const char* message, void* /* pUserData */) {
  printf("Uncaptured device error: %s\n", errorStringify(type));
  if (message) {
    printf("%s ", message);
  }
  printf("\n");
  abort();
}
WGPUDevice Application_device_request(WGPUAdapter adapter) {
  WGPUSupportedLimits supported = { .nextInChain = 0, .limits = { 0 } };
  wgpuAdapterGetLimits(adapter, &supported);
  WGPURequiredLimits required = {
    .nextInChain = 0,
    .limits = supported.limits,
  };
  limitsSet(&required, supported);
  WGPUDeviceDescriptor descriptor = {
    .nextInChain = 0,
    .label = "Device 1",
    .requiredFeaturesCount = 0,
    .requiredFeatures = 0,
    .requiredLimits = &required,
    .defaultQueue = { .label = "default queueuue" }
  };
  Response response = { .device = 0, .done = 0 };
  wgpuAdapterRequestDevice(adapter, &descriptor, device_onRequest, (void*)&response);
  assert(response.done);
  wgpuDeviceSetUncapturedErrorCallback(response.device, onDeviceError, 0);
  wgpuDeviceSetLoggingCallback(response.device, onLog, 0);
  return response.device;
}
static char* readFile(const char* filename);
static void WGPUCompilationMessageStringify(const WGPUCompilationMessage message);
static char* compilationStatusStringify(WGPUCompilationInfoRequestStatus status);
static void compilationPrint(
  WGPUCompilationInfoRequestStatus status,
  struct WGPUCompilationInfo const* info,
  void* /* userdata */) {
  fprintf(
    stderr,
    "Shader module compilation status: %s\n",
    compilationStatusStringify(status));
  for (size_t i = 0; info->messageCount > i; i++) {
    WGPUCompilationMessageStringify(info->messages[i]);
  }
}
WGPUShaderModule Application_device_ShaderModule(WGPUDevice device, const char* path) {
  char* shader = readFile(path);
  if (!shader) {
    fprintf(stderr, "Error opening %s: %s\n", path, strerror(errno));
    return 0;
  }
  WGPUShaderModuleWGSLDescriptor codeDescriptor = {
    .chain.next = 0,
    .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
    .code = shader,
  };
  WGPUShaderModuleDescriptor shaderDescriptor = {
    .nextInChain = &codeDescriptor.chain,
    .label = path,
  };
  WGPUShaderModule result = wgpuDeviceCreateShaderModule(device, &shaderDescriptor);
  wgpuShaderModuleGetCompilationInfo(result, &compilationPrint, 0);
  free(shader);
  return result;
}
void Application_device_inspect(WGPUDevice device) {
  size_t featureCount = wgpuDeviceEnumerateFeatures(device, 0);
  WGPUFeatureName* features = calloc(featureCount, sizeof(*features));
  wgpuDeviceEnumerateFeatures(device, features);
  printf("Device Features: \n");
  for (size_t i = 0; featureCount > i; i++) {
    printf("- %i \n", features[i]);
  }
  free(features);
  WGPUSupportedLimits limits = { .nextInChain = 0 };
  if (wgpuDeviceGetLimits(device, &limits)) {
    printf("Device limits: \n");
    printf("- maxTextureDimension1D: %u \n", limits.limits.maxTextureDimension1D);
    printf("- maxTextureDimension2D: %u \n", limits.limits.maxTextureDimension2D);
    printf("- maxTextureDimension3D: %u \n", limits.limits.maxTextureDimension3D);
    printf("- maxTextureArrayLayers: %u \n", limits.limits.maxTextureArrayLayers);
    printf("- maxBindGroups: %u \n", limits.limits.maxBindGroups);
    printf(
      "- maxDynamicUniformBuffersPerPipelineLayout: %u \n",
      limits.limits.maxDynamicUniformBuffersPerPipelineLayout);
    printf(
      "- maxDynamicStorageBuffersPerPipelineLayout: %u \n",
      limits.limits.maxDynamicStorageBuffersPerPipelineLayout);
    printf(
      "- maxSampledTexturesPerShaderStage: %u \n",
      limits.limits.maxSampledTexturesPerShaderStage);
    printf("- maxSamplersPerShaderStage: %u \n", limits.limits.maxSamplersPerShaderStage);
    printf(
      "- maxStorageBuffersPerShaderStage: %u \n",
      limits.limits.maxStorageBuffersPerShaderStage);
    printf(
      "- maxStorageTexturesPerShaderStage: %u \n",
      limits.limits.maxStorageTexturesPerShaderStage);
    printf(
      "- maxUniformBuffersPerShaderStage: %u \n",
      limits.limits.maxUniformBuffersPerShaderStage);
    printf(
      "- maxUniformBufferBindingSize: %zu \n",
      limits.limits.maxUniformBufferBindingSize);
    printf(
      "- maxStorageBufferBindingSize: %zu \n",
      limits.limits.maxStorageBufferBindingSize);
    printf(
      "- minUniformBufferOffsetAlignment: %u \n",
      limits.limits.minUniformBufferOffsetAlignment);
    printf(
      "- minStorageBufferOffsetAlignment: %u \n",
      limits.limits.minStorageBufferOffsetAlignment);
    printf("- maxVertexBuffers: %u \n", limits.limits.maxVertexBuffers);
    printf("- maxVertexAttributes: %u \n", limits.limits.maxVertexAttributes);
    printf("- maxVertexBufferArrayStride: %u \n", limits.limits.maxVertexBufferArrayStride);
    printf(
      "- maxInterStageShaderComponents: %u \n",
      limits.limits.maxInterStageShaderComponents);
    printf(
      "- maxComputeWorkgroupStorageSize: %u \n",
      limits.limits.maxComputeWorkgroupStorageSize);
    printf(
      "- maxComputeInvocationsPerWorkgroup: %u \n",
      limits.limits.maxComputeInvocationsPerWorkgroup);
    printf("- maxComputeWorkgroupSizeX: %u \n", limits.limits.maxComputeWorkgroupSizeX);
    printf("- maxComputeWorkgroupSizeY: %u \n", limits.limits.maxComputeWorkgroupSizeY);
    printf("- maxComputeWorkgroupSizeZ: %u \n", limits.limits.maxComputeWorkgroupSizeZ);
    printf(
      "- maxComputeWorkgroupsPerDimension: %u \n",
      limits.limits.maxComputeWorkgroupsPerDimension);
  }
}
static void writeMipMaps(
  WGPUDevice device,
  WGPUTexture texture,
  WGPUExtent3D size,
  uint32_t mipLevelCount,
  const uint8_t* pixelsInput) {
  WGPUQueue queue = wgpuDeviceGetQueue(device);
  WGPUImageCopyTexture destination = {
    .texture = texture,
    .origin = {0, 0, 0},
    .aspect = WGPUTextureAspect_All,
  };
  WGPUTextureDataLayout source = { .offset = 0 };
  WGPUExtent3D mipLevelSize = size;
  WGPUExtent3D previousMipLevelSize;
  uint32_t pixelCount = 4 * mipLevelSize.width * mipLevelSize.height;
  uint8_t* previousLevelPixels = calloc(pixelCount, sizeof(*previousLevelPixels));
  uint8_t* pixels = calloc(pixelCount, sizeof(*pixels));
  memcpy(pixels, pixelsInput, pixelCount);
  for (uint32_t level = 0; mipLevelCount > level; ++level) {
    if (level) {
      for (uint32_t i = 0; mipLevelSize.width > i; ++i) {
        for (uint32_t j = 0; mipLevelSize.height > j; ++j) {
          uint8_t* p = &pixels[4 * (j * mipLevelSize.width + i)];
          uint8_t* p00 = &previousLevelPixels
                           [4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
          uint8_t* p01 = &previousLevelPixels
                           [4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
          uint8_t* p10 = &previousLevelPixels
                           [4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
          uint8_t* p11 = &previousLevelPixels
                           [4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
          p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
          p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
          p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
          p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
        }
      }
    }
    pixelCount = 4 * mipLevelSize.width * mipLevelSize.height;
    destination.mipLevel = level;
    source.bytesPerRow = 4 * mipLevelSize.width;
    source.rowsPerImage = mipLevelSize.height;
    wgpuQueueWriteTexture(queue, &destination, pixels, pixelCount, &source, &mipLevelSize);
    memcpy(previousLevelPixels, pixels, pixelCount);
    previousMipLevelSize = mipLevelSize;
    mipLevelSize.width /= 2;
    mipLevelSize.height /= 2;
  }
  free(previousLevelPixels);
  free(pixels);
  wgpuQueueRelease(queue);
}
static uint32_t bit_width(uint32_t m) {
  if (m == 0) {
    return 0;
  }
  else {
    uint32_t w = 0;
    while (m >>= 1) {
      ++w;
    }
    return w;
  }
}
WGPUTexture Application_device_Texture_load(
  WGPUDevice device,
  const char* const path,
  WGPUTextureView* view) {
  int width = 0;
  int height = 0;
  int channels = 0;
  uint8_t* pixels = stbi_load(path, &width, &height, &channels, 4);
  if (!pixels) {
    return 0;
  }
  WGPUTextureDescriptor descriptor = {
    .nextInChain = 0,
    .dimension = WGPUTextureDimension_2D,
 // by convention for bmp, png and jpg file. Be careful with other formats,
    .format = WGPUTextureFormat_RGBA8Unorm,
    .mipLevelCount = bit_width(fmax(descriptor.size.width, descriptor.size.height)),
    .sampleCount = 1,
    .size = {(unsigned int)width, (unsigned int)height, 1},
    .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
    .viewFormatCount = 0,
    .viewFormats = 0,
  };
  WGPUTexture texture = wgpuDeviceCreateTexture(device, &descriptor);
  writeMipMaps(device, texture, descriptor.size, descriptor.mipLevelCount, pixels);
  stbi_image_free(pixels);
  if (view) {
    WGPUTextureViewDescriptor viewDescriptor = {
      .aspect = WGPUTextureAspect_All,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .baseMipLevel = 0,
      .mipLevelCount = descriptor.mipLevelCount,
      .dimension = WGPUTextureViewDimension_2D,
      .format = descriptor.format,
    };
    *view = wgpuTextureCreateView(texture, &viewDescriptor);
  }
  return texture;
}
static char* readFile(const char* filename) {
  char* result = 0;
  FILE* input = fopen(filename, "rb");
  if (!input) {
    result = 0;
  }
  else {
    const size_t maxSize = 10 * 1024 * 1024;
    size_t size = BUFSIZ;
    result = malloc(size * sizeof(*result));
    size_t total = 0;
    while (!feof(input) && !ferror(input) && (maxSize > size)) {
      if (total + BUFSIZ > size) {
        size = size * 2;
        result = realloc(result, size);
      }
      char* buffer = result + total;
      total += fread(buffer, 1, BUFSIZ, input);
    }
    fclose(input);
    result = realloc(result, total + 1);
    result[total] = '\0';
  }
  return result;
}
#define STRINGIFY(value) \
  case value:            \
    return #value
static char* logTypeStringify(WGPULoggingType type) {
  switch (type) {
    STRINGIFY(WGPULoggingType_Verbose);
    STRINGIFY(WGPULoggingType_Info);
    STRINGIFY(WGPULoggingType_Warning);
    STRINGIFY(WGPULoggingType_Error);
    STRINGIFY(WGPULoggingType_Force32);
  }
}
static char* errorStringify(WGPUErrorType error) {
  switch (error) {
    STRINGIFY(WGPUErrorType_NoError);
    STRINGIFY(WGPUErrorType_Validation);
    STRINGIFY(WGPUErrorType_OutOfMemory);
    STRINGIFY(WGPUErrorType_Internal);
    STRINGIFY(WGPUErrorType_Unknown);
    STRINGIFY(WGPUErrorType_DeviceLost);
    STRINGIFY(WGPUErrorType_Force32);
  }
}
static char* compilationStatusStringify(WGPUCompilationInfoRequestStatus status) {
  switch (status) {
    case WGPUCompilationInfoRequestStatus_Success:
      return "Success";
    case WGPUCompilationInfoRequestStatus_Error:
      return "Error";
    case WGPUCompilationInfoRequestStatus_DeviceLost:
      return "DeviceLost";
    case WGPUCompilationInfoRequestStatus_Unknown:
      return "Unknown";
    case WGPUCompilationInfoRequestStatus_Force32:
      return "Force32";
  }
}
static void WGPUCompilationMessageStringify(const WGPUCompilationMessage message) {
  switch (message.type) {
    case WGPUCompilationMessageType_Error:
      perror("Error:\n");
      break;
    case WGPUCompilationMessageType_Warning:
      perror("Warning:\n");
      break;
    case WGPUCompilationMessageType_Info:
      perror("Info:\n");
      break;
    case WGPUCompilationMessageType_Force32:
      perror("Force32:\n");
      break;
  }
  if (message.message) {
    fprintf(stderr, "%s:\n", message.message);
  }
  fprintf(stderr, "lineNum: %zu\n", message.lineNum);
  fprintf(stderr, "linePos: %zu\n", message.linePos);
  fprintf(stderr, "offset: %zu\n", message.offset);
  fprintf(stderr, "length: %zu\n", message.length);
  fprintf(stderr, "utf16LinePos: %zu\n", message.utf16LinePos);
  fprintf(stderr, "utf16Offset: %zu\n", message.utf16Offset);
  fprintf(stderr, "utf16Length: %zu\n", message.utf16Length);
}
#undef STRINGIFY
