#ifndef device_H_
#define device_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <tgmath.h>
#include "library/webgpu.h"
#define STB_IMAGE_IMPLEMENTATION
#include "library/stb_image.h"

typedef struct {
    WGPUDevice device;
    bool requestEnded;
} DeviceData;

static void device_onRequest(
  WGPURequestDeviceStatus status,
  WGPUDevice device,
  const char* message,
  void* pUserData) {
  DeviceData* userData = (DeviceData*)pUserData;
  if (status == WGPURequestDeviceStatus_Success) {
    userData->device = device;
  }
  else {
    printf("Could not get WebGPU device: %s\n", message);
  }
  userData->requestEnded = true;
}
static char* errorToString(WGPUErrorType error) {
  switch (error) {
    case WGPUErrorType_NoError:
      return "WGPUErrorType_NoError";
    case WGPUErrorType_Validation:
      return "WGPUErrorType_Validation";
    case WGPUErrorType_OutOfMemory:
      return "WGPUErrorType_OutOfMemory";
    case WGPUErrorType_Internal:
      return "WGPUErrorType_Internal";
    case WGPUErrorType_Unknown:
      return "WGPUErrorType_Unknown";
    case WGPUErrorType_DeviceLost:
      return "WGPUErrorType_DeviceLost";
    case WGPUErrorType_Force32:
      return "WGPUErrorType_Force32";
  }
}
static void onDeviceError(WGPUErrorType type, const char* message, void* /* pUserData */) {
  printf("Uncaptured device error: %s\n", errorToString(type));
  if (message) {
    printf("%s ", message);
  }
  printf("\n");
  abort();
}
WGPUDevice device_request(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor) {
  DeviceData userData = { .device = 0, .requestEnded = 0 };
  wgpuAdapterRequestDevice(adapter, descriptor, device_onRequest, (void*)&userData);
  assert(userData.requestEnded);
  wgpuDeviceSetUncapturedErrorCallback(userData.device, onDeviceError, 0);
  return userData.device;
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
    result = (char*)malloc(size * sizeof(*result));
    size_t total = 0;
    while (!feof(input) && !ferror(input) && (maxSize > size)) {
      if (total + BUFSIZ > size) {
        size = size * 2;
        result = (char*)realloc(result, size);
      }
      char* buffer = result + total;
      total += fread(buffer, 1, BUFSIZ, input);
    }
    fclose(input);
    result = (char*)realloc(result, total + 1);
    result[total] = '\0';
  }
  return result;
}
WGPUShaderModule device_ShaderModule(WGPUDevice device, const char* path) {
  char* shader = readFile(path);
  if (!shader) {
    fprintf(stderr, "Error opening %s: %s\n", path, strerror(errno));
    return 0;
  }
  WGPUShaderModuleWGSLDescriptor shaderCodeDescriptor = {
    .chain.next = 0,
    .chain.sType = WGPUSType_ShaderModuleWGSLDescriptor,
    .code = shader,
  };
  WGPUShaderModuleDescriptor shaderDescriptor = {
    .nextInChain = &shaderCodeDescriptor.chain,
    .label = path,
  };
  WGPUShaderModule result = wgpuDeviceCreateShaderModule(device, &shaderDescriptor);
  free(shader);
  return result;
}
void device_inspect(WGPUDevice device) {
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
  WGPUTextureDataLayout source = {
    .offset = 0,
  };
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
WGPUTexture device_Texture_load(
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
  WGPUTextureDescriptor textureDescriptor = {
    .nextInChain = 0,
    .dimension = WGPUTextureDimension_2D,
    .format =
      WGPUTextureFormat_RGBA8Unorm, // by convention for bmp, png and jpg file. Be careful with other formats,
    .mipLevelCount =
      bit_width(fmax(textureDescriptor.size.width, textureDescriptor.size.height)),
    .sampleCount = 1,
    .size = {(unsigned int)width, (unsigned int)height, 1},
    .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
    .viewFormatCount = 0,
    .viewFormats = 0,
  };
  WGPUTexture texture = wgpuDeviceCreateTexture(device, &textureDescriptor);
  writeMipMaps(
    device,
    texture,
    textureDescriptor.size,
    textureDescriptor.mipLevelCount,
    pixels);
  stbi_image_free(pixels);
  if (view) {
    WGPUTextureViewDescriptor viewDescriptor = {
      .aspect = WGPUTextureAspect_All,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .baseMipLevel = 0,
      .mipLevelCount = textureDescriptor.mipLevelCount,
      .dimension = WGPUTextureViewDimension_2D,
      .format = textureDescriptor.format,
    };
    printf("texture view");
    *view = wgpuTextureCreateView(texture, &viewDescriptor);
  }
  return texture;
}

#endif // device_H_
