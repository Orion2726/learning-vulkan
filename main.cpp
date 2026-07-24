#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>
#include<map>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
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
        vk::raii::Instance instancep = VK_NULL_HANDLE;
        vk::raii::DebugUtilsMessengerEXT debugMessengerp = VK_NULL_HANDLE;
        vk::raii::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
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
            SetupDebugMessenger();
            pickPhysicalDevice();
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
        //CREATE INSTANCE
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

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
            vk::DebugUtilsMessageTypeFlagsEXT type,
            const vk::DebugUtilsMessengerCallbackDataEXT* pCallBackData,
            void* pUserData)
        {
            std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallBackData->pMessage << std::endl;
            return vk::False;
        }
        void SetupDebugMessenger()
        {
                if(!enableValidationLayers) return;
                vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
                vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlag(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral|
                                                                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                                                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                                vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding
                                                                );
                vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
                                                                .messageSeverity = severityFlags,
                                                                .messageType = messageTypeFlag,
                                                                .pfnUserCallback = &debugCallback
                                                                };
                 debugMessengerp = instancep.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
        }
        //PHYSICIAL DEVICE
        void pickPhysicalDevice()
        {
            std::vector<vk::raii::PhysicalDevice> physicalDevices = instancep.enumeratePhysicalDevices();
            auto const devIter = std::ranges::find_if(physicalDevices,[&](auto const & physicalDevice){return isDeviceSuitable(physicalDevice);});
            if(devIter == physicalDevices.end())
            {
                throw std::runtime_error("Error::Failed to find a suitable GPU");
            }
            physicalDevice = *devIter;
        }
        bool isDeviceSuitable(vk::raii::PhysicalDevice const & physicalDevice)
        {

            bool supportsVulkan1_4 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion14;
            auto queueFamiles = physicalDevice.getQueueFamilyProperties();
            bool supportGraphics = std::ranges::any_of(queueFamiles,[](auto const &qfp){return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });
            //extension check
            std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};
            auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
            bool supportsAllReqExtensions = std::ranges::all_of(requiredDeviceExtension,[&availableDeviceExtensions](auto const &requiredDeviceExtension){
                return std::ranges::any_of(availableDeviceExtensions,[requiredDeviceExtension](auto const &availableDeviceExtension)
                {
                    return strcmp(availableDeviceExtension.extensionName,requiredDeviceExtension) == 0; } );

            });
            auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                                 vk::PhysicalDeviceVulkan11Features,
                                                                 vk::PhysicalDeviceVulkan13Features,
                                                                 vk::PhysicalDeviceVulkan14Features,
                                                                 vk::PhysicalDeviceExtendedDynamicState2FeaturesEXT>();

            bool supportRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                           features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                           features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

            return supportsVulkan1_4 && supportGraphics && supportsAllReqExtensions && supportRequiredFeatures;
        }
    };
