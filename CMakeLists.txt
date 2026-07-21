cmake_minimum_required(VERSION 3.10)
project(VulkanApp)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
add_executable(VulkanApp main.cpp)

target_link_libraries(VulkanApp PRIVATE Vulkan::Vulkan glfw)
