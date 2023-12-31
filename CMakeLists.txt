cmake_minimum_required(VERSION 3.20)
project(ThesisImplementation)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${PROJECT_SOURCE_DIR})

find_package(Vulkan REQUIRED)

include_directories(
    "$ENV{VULKAN_SDK}/Include"
    ${PROJECT_SOURCE_DIR}/3rdParty/include
)
link_directories(${CMAKE_SOURCE_DIR}/3rdParty/lib)

add_compile_definitions(IMGUI_USER_CONFIG="${PROJECT_SOURCE_DIR}/src/my_imgui_config.h")

set(imgui_src
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/backends/imgui_impl_vulkan.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/backends/imgui_impl_glfw.cpp
    ${PROJECT_SOURCE_DIR}/3rdParty/include/imgui/misc/cpp/imgui_stdlib.cpp)

file(GLOB_RECURSE rendering_lib_src
    "src/preprocessing/*.cpp" # temporary, remove when switching to real models
    "src/control/*.cpp"
    "src/model/*.cpp"
    "src/view/*.cpp"
)
add_library(rendering STATIC
    ${rendering_lib_src}
    ${imgui_src}
)
add_executable(renderer src/renderer.cpp)
target_link_libraries(renderer
    rendering
    "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib"
    glfw3
)

file(GLOB_RECURSE preprocessing_lib_src
    "src/preprocessing/*.cpp"
)
add_library(preprocessing STATIC
    ${preprocessing_lib_src}
)
add_executable(preprocessor src/preprocessor.cpp)
target_link_libraries(preprocessor
    preprocessing
)
