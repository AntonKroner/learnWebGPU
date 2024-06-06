add_library(cimgui STATIC cimgui.c)
target_include_directories(cimgui PUBLIC .)
target_link_libraries(cimgui PUBLIC glfw webgpu)
