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

file(GLOB_RECURSE lib_src
    "src/control/*.cpp"
    "src/model/*.cpp"
    "src/view/*.cpp"
    "src/config.cpp"
)

add_library(${PROJECT_NAME} STATIC
    ${lib_src}
)

add_executable(renderer src/main.cpp)

target_link_libraries(renderer
    ${PROJECT_NAME}
    "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib"
    glfw3
)
