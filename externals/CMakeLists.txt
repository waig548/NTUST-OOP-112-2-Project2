# stduuid
set(UUID_SYSTEM_GENERATOR ON)
add_subdirectory(stduuid)

# json
add_subdirectory(json)

# CRCpp
set(BUILD_TEST OFF)
add_subdirectory(CRCpp)

# cparse
add_library(cparse
    cparse/shunting-yard.h
    cparse/shunting-yard-exceptions.h
    cparse/shunting-yard.cpp
    cparse/packToken.h
    cparse/packToken.cpp
    cparse/functions.h
    cparse/functions.cpp
    cparse/containers.h
    cparse/containers.cpp

    cparse/builtin-features.inc
    cparse/builtin-features.cpp
)
target_include_directories(cparse PUBLIC "cparse/")
target_include_directories(cparse PUBLIC "cparse/builtin-features")

# color
add_subdirectory(color)

# bimap
add_library(bimap INTERFACE bimap/bimap.hpp)
target_include_directories(bimap INTERFACE "bimap/")

# random
set(Random_BuildTests OFF)
add_subdirectory(random)

# glad
add_subdirectory(glad)

# glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build glfw examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build glfw tests")
set(GLFW_LIBRARY_TYPE STATIC)
add_subdirectory(glfw)

# imgui
add_library(imgui
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h

    imgui/misc/cpp/imgui_stdlib.h
    imgui/misc/cpp/imgui_stdlib.cpp

    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_opengl3.h
)
target_include_directories(imgui PUBLIC "imgui/")
target_include_directories(imgui PUBLIC "imgui/misc/cpp/")
target_include_directories(imgui PUBLIC "imgui/backends/")
target_link_libraries(imgui PRIVATE glfw)

# ImGuiFileDialog
add_subdirectory(ImGuiFileDialog)
target_link_libraries(ImGuiFileDialog PRIVATE imgui)

# stb_image
add_library(stb INTERFACE stb/stb_image.h)
target_include_directories(stb INTERFACE "stb/")