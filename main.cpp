#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <vulkan/vk_platform.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan.hpp"
#include <ranges>
#include <cstdint>
#include <exception>
#if defined (__INTELLISENSE__) || !defined(USE_CPP20_MODULLES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include<stdexcept>
#include <cstdlib>
constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
const std::vector<char const*> validationLayers =
    {
      "VK_LAYER_KHRONOS_validation"
    };
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif
class  hellotriangle
{
    public:
        void run()
        {
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }
    private:
        GLFWwindow *windowp = nullptr;
        vk::raii::Context context;
        vk::raii::Instance instancep = nullptr;
        void initWindow()
        {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
            windowp = glfwCreateWindow(WIDTH,HEIGHT,"vulkan",nullptr,nullptr);
        }
        void initVulkan()
        {
            createInstance();
        }
        void mainLoop()
        {
            while(!glfwWindowShouldClose(windowp))
            {
                glfwPollEvents();
            }
        }
        void cleanup()
        {
            glfwDestroyWindow(windowp);
            glfwTerminate();
        }
        void createInstance()
        {
            constexpr vk::ApplicationInfo appInfo
            {
                .pApplicationName = "Sandbox",
                .applicationVersion = VK_MAKE_VERSION(1,0,0),
                .pEngineName = "No Engine",
                .engineVersion = VK_MAKE_VERSION(1,0,0),
                .apiVersion = vk::ApiVersion14
            };


            //validatioin layers
            std::vector<char const*> requiredLayers;
            if(enableValidationLayers)
            {
                requiredLayers.assign(validationLayers.begin(),validationLayers.end());
            }
            auto layerProperties = context.enumerateInstanceLayerProperties();
            auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,[&layerProperties](auto const &requiredLayer){
                return std::ranges::none_of(layerProperties,[requiredLayer](auto const &layerProperty){
                    return strcmp(layerProperty.layerName,requiredLayer) == 0;
                });

            });
            if(unsupportedLayerIt != requiredLayers.end())
            {
                throw std::runtime_error("Error::Required layer not supported: " + std::string(*unsupportedLayerIt));
            }
            //extesnions
            auto requiredExtensions = getRequiredInstanceExtensions();
            auto extensionProperties = context.enumerateInstanceExtensionProperties();
            auto unsupportedPropertyIt = std::ranges::find_if(requiredExtensions,[&extensionProperties](auto const &requiredExtension){
                return std::ranges::none_of(extensionProperties,[requiredExtension](auto const &extensionProperty){
                    return strcmp(extensionProperty.extensionName,requiredExtension) == 0;
                });
            });
            if(unsupportedPropertyIt != requiredExtensions.end())
            {
                throw std::runtime_error("Error::Required extension not supported: "+std::string(*unsupportedPropertyIt));
            }
            vk::InstanceCreateInfo createInfo
            {
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
                .ppEnabledLayerNames = requiredLayers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
                .ppEnabledExtensionNames = requiredExtensions.data()
            };
            instancep = vk::raii::Instance(context,createInfo);


        }
        std::vector<const char*> getRequiredInstanceExtensions()
        {
            uint32_t glfwExtensionCount = 0;
            auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            std::vector extensions(glfwExtensions,glfwExtensions + glfwExtensionCount);
            if(enableValidationLayers)
            {
                extensions.push_back(vk::EXTDebugUtilsExtensionName);
            }
            return extensions;
        }
        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugcallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallBackData,
        void* pUserData
        )
        {
            std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallBackData->pMessage << std::endl;
        }

};
int main()
{
    try{
        hellotriangle app;
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
