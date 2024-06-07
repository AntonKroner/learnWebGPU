add_library(cimgui STATIC
		imgui_impl_wgpu.h
		imgui_impl_wgpu.cpp
    cimgui.h
    cimgui.cpp
		cimgui_impl_glfw.h
		cimgui_impl_glfw.cpp
		cimgui_impl_wgpu.h
    cimgui_impl_wgpu.cpp
)
set(imgui_DIR ../../vcpkg_installed/x64-linux/share/imgui)
find_package(imgui CONFIG REQUIRED)

target_include_directories(cimgui PUBLIC .)
target_link_libraries(cimgui PUBLIC webgpu glfw imgui::imgui)
