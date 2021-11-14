cmake_minimum_required(VERSION 3.0)
project(VulkanEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SHADERS ${CMAKE_SOURCE_DIR}/Shaders/*.vert ${CMAKE_SOURCE_DIR}/Shaders/*.frag)

find_package(Vulkan REQUIRED)
find_package(SDL2 REQUIRED)
find_package(tinyobjloader REQUIRED)

add_executable(VulkanEngine Main.cpp Buffer.hpp Buffer.cpp Camera.cpp Camera.hpp Core.cpp Core.hpp Descriptors.cpp Descriptors.hpp Device.cpp Device.hpp GameObject.cpp GameObject.hpp Model.cpp Model.hpp MovementController.cpp MovementController.hpp Pipeline.cpp Pipeline.hpp Renderer.cpp Renderer.hpp RenderSystem.cpp RenderSystem.hpp SwapChain.cpp SwapChain.hpp Utils.hpp Window.cpp Window.hpp)
target_compile_options(VulkanEngine PRIVATE -Wall -Wextra)
target_link_libraries(VulkanEngine Vulkan::Vulkan SDL2 tinyobjloader)

add_custom_command(TARGET VulkanEngine PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Models/ ${PROJECT_BINARY_DIR}/Models)

add_custom_command(TARGET VulkanEngine PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/Shaders/)

foreach(FILE ${SHADERS})
    get_filename_component(FILE_NAME ${FILE} NAME)
    set(OUTFILE "${PROJECT_BINARY_DIR}/Shaders/${FILE_NAME}.spv")
    add_custom_command(TARGET VulkanEngine PRE_BUILD COMMAND Vulkan::glslc -c ${FILE} -o ${OUTFILE})
endforeach(FILE)