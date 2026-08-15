cmake_minimum_required(VERSION 3.20)
project(vk-renderer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

# stb_image
FetchContent_Declare(
    stb_image
    URL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    DOWNLOAD_NO_EXTRACT TRUE
)

# tinyobjloader
FetchContent_Declare(
    tinyobjloader
    URL https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/release/tiny_obj_loader.h
    DOWNLOAD_NO_EXTRACT TRUE
)

# vulkan
find_package(Vulkan REQUIRED)
# Prevent CMake from stripping the Vulkan include path from compile_commands.json.
# Clangd doesn't search implicit compiler paths, so it needs the explicit -I flag.
if(Vulkan_INCLUDE_DIRS)
  list(REMOVE_ITEM CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}")
endif()

# volk (needs to be after vulkan)
FetchContent_Declare(
    volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG 1.4.350
    GIT_SHALLOW TRUE
)

# vulkan memory allocator
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.4.0
    GIT_SHALLOW TRUE
)

# slang
FetchContent_Declare(
   slang
   GIT_REPOSITORY https://github.com/shader-slang/slang.git
   GIT_TAG v2026.14.1
   GIT_SHALLOW TRUE
)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw glm stb_image tinyobjloader volk VulkanMemoryAllocator slang)

add_executable(${PROJECT_NAME}
  src/main.cpp
  defines/volk_implementation.cpp
  defines/vma_implementation.cpp
  src/Core/Window.cpp
  src/Render/vk_context.cpp
  src/Render/Managers/vk_instance.cpp
  src/Render/Managers/vk_device.cpp
  src/Render/Managers/vk_memory.cpp
  src/Render/Managers/vk_swapchain.cpp
  src/Render/Managers/vk_pipeline.cpp
  src/Render/Managers/vk_sync.cpp
  src/Render/Types/Shader.cpp
  src/Render/Renderer.cpp
  src/Application/Application.cpp
  src/Input/Input.cpp
  src/Camera/arcball_camera.cpp
)

add_custom_target(run
  COMMAND vk-renderer
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
  DEPENDS vk-renderer
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${stb_image_SOURCE_DIR}
  ${tinyobjloader_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_RADIANS GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
    volk
    GPUOpen::VulkanMemoryAllocator
    slang
)
