cmake_minimum_required(VERSION 3.0...3.22.1)
project(
    learnWebGPU
    VERSION 0.1.0
    LANGUAGES C CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(webgpu.exe
	main.c
	adapter.c
	device.c
	01basic3d/01geometry/triangle.c
	01basic3d/01geometry/buffers.c
	01basic3d/01geometry/vertex.c
	01basic3d/01geometry/attributes.c
	01basic3d/01geometry/index.c
	01basic3d/02uniforms/first.c
	01basic3d/02uniforms/more.c
)
set_target_properties(webgpu.exe PROPERTIES
    C_STANDARD 23
    COMPILE_WARNING_AS_ERROR ON
)
target_compile_definitions(webgpu.exe PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


if (NOT EMSCRIPTEN)
add_subdirectory(library/glfw)
endif()
add_subdirectory(library/glfw3webgpu)
add_subdirectory(library/dawn)
target_link_options(webgpu.exe PRIVATE -lstdc++)
target_link_libraries(webgpu.exe PRIVATE stdc++ glfw webgpu glfw3webgpu)

if (MSVC)
    target_compile_options(webgpu.exe PRIVATE /W4)
else()
    target_compile_options(webgpu.exe PRIVATE -Wall -Wextra -pedantic)
endif()
if(XCODE)
    set_target_properties(webgpu.exe PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()

if (EMSCRIPTEN)
		set_target_properties(webgpu.exe PROPERTIES SUFFIX ".html")
    target_link_options(webgpu.exe PRIVATE
        -sUSE_GLFW=3 # Use Emscripten-provided GLFW
        -sUSE_WEBGPU # Handle WebGPU symbols
        -sALLOW_MEMORY_GROWTH
        --preload-file "${CMAKE_CURRENT_SOURCE_DIR}/resources"
#        -sASYNCIFY # Required by WebGPU-C++
    )
endif()

target_copy_webgpu_binaries(webgpu.exe)
