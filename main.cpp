#include <cstdlib>
#include <limits>
#define GLFW_INCLUDE_VULKAN
#include <algorithm>
#include <cstring>
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VK_USE_PLATFORM_WAYLAND_KHR
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#if defined (__INTELLISENSE__) || !defined(USE_CPP20_MODULLES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <iostream>
#include<stdexcept>
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

        vk::raii::Context context;
        vk::Extent2D swapChainExtent;
        vk::SurfaceFormatKHR swapChainSurfaceFormat;
        std::vector<vk::Image> swapChainImages;

        GLFWwindow *windowp = nullptr;
        vk::raii::SurfaceKHR surface = nullptr;
        vk::raii::Instance instancep = VK_NULL_HANDLE;
        vk::raii::DebugUtilsMessengerEXT debugMessengerp = VK_NULL_HANDLE;
        vk::raii::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
        vk::raii::Device logicalDevice = VK_NULL_HANDLE;
        vk::raii::Queue graphicsQueue = VK_NULL_HANDLE;
        vk::raii::SwapchainKHR swapChain = VK_NULL_HANDLE;

        std::vector<const char*> requiredDeviceExtension =
            {
              vk::KHRSwapchainExtensionName
            };
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
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createSwapChain();
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
                                                                 vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

            bool supportRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
                                           features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
                                           features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

            return supportsVulkan1_4 && supportGraphics && supportsAllReqExtensions && supportRequiredFeatures;
        }
        //Logical Device
        void createLogicalDevice()
        {

            std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
            uint32_t queueIndex = ~0;
            for(uint32_t qfpIndex = 0;qfpIndex < queueFamilyProperties.size();qfpIndex++)
            {
                if((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
                {
                   queueIndex = qfpIndex;
                   break;
                }
            }
            if(queueIndex == ~0)
            {
                throw std::runtime_error("Error::Could not find a queue for graphics and present.");
            }
            vk::StructureChain<vk::PhysicalDeviceFeatures2,
                               vk::PhysicalDeviceVulkan11Features,
                               vk::PhysicalDeviceVulkan13Features,
                               vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
            featureChain =
                {
                    {},
                    {.shaderDrawParameters = true},
                    {.dynamicRendering = true},
                    {.extendedDynamicState = true}
                };

            float queuePriority = 0.5f;
            vk::DeviceQueueCreateInfo deviceQueueCreateInfo{.queueFamilyIndex = queueIndex,
                                                            .queueCount = 1,
                                                            .pQueuePriorities = &queuePriority};
            vk::DeviceCreateInfo deviceCreateInfo
            {
                .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
                .queueCreateInfoCount = 1,
                .pQueueCreateInfos = &deviceQueueCreateInfo,
                .enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
                .ppEnabledExtensionNames = requiredDeviceExtension.data()
            };
            logicalDevice = vk::raii::Device(physicalDevice,deviceCreateInfo);
            graphicsQueue = vk::raii::Queue(logicalDevice,queueIndex,0);

            //Swap Chain
            auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
            std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
            std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);


        }
        //Suraface creatin
        void createSurface()
        {
            VkSurfaceKHR _surface;
            if(glfwCreateWindowSurface(*instancep,windowp, VK_NULL_HANDLE, &_surface) != 0)
            {
                throw std::runtime_error("Error::Window creation failed");
            }
            surface =  vk::raii::SurfaceKHR(instancep,*surface);

        }
        //Swap chain creation
        void createSwapChain()
        {
            vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
            auto swapChainExtent = chooseSwapExtent(surfaceCapabilities);
            uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);
            std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
            std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR();
            auto swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);
            vk::SwapchainCreateInfoKHR swapChainCreateInfo{.surface = *surface,
                                                           .minImageCount = minImageCount,
                                                           .imageFormat = swapChainSurfaceFormat.format,
                                                           .imageColorSpace = swapChainSurfaceFormat.colorSpace,
                                                           .imageExtent = swapChainExtent,
                                                           .imageArrayLayers = 1,
                                                           .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
                                                           .imageSharingMode = vk::SharingMode::eExclusive,
                                                           .preTransform = surfaceCapabilities.currentTransform,
                                                           .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                           .presentMode = chooseSwapPresentMode(availablePresentModes),
                                                           .clipped = true

            };
            swapChain = vk::raii::SwapchainKHR(logicalDevice,swapChainCreateInfo);
            swapChainImages = swapChain.getImages();
        }
        //surfaec options
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats)
        {
            const auto formatIt = std::ranges::find_if(availableFormats,[](const auto &format){return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;});
            return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
        }
        vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentMode)
        {
            assert(std::ranges::any_of(availablePresentMode,[](auto presentMode){return presentMode == vk::PresentModeKHR::eFifo;}));
            return std::ranges::any_of(availablePresentMode,[](const vk::PresentModeKHR value){return vk::PresentModeKHR::eMailbox == value;}) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
        }
        vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capbilities)
        {
            if(capbilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            {
                return capbilities.currentExtent;
            }
            int width,height;
            glfwGetFramebufferSize(windowp, &width,&height);
            return
            {
                std::clamp<uint32_t>(width,capbilities.minImageExtent.width,capbilities.maxImageExtent.width),
                std::clamp<uint32_t>(height,capbilities.minImageExtent.width,capbilities.maxImageExtent.width)

            };
        }
        uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &surfaceCapabilities)
        {
            auto minImageCount = std::max(3u,surfaceCapabilities.minImageCount);
            if((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
            {
                minImageCount = surfaceCapabilities.maxImageCount;
            }
            return minImageCount;
        }
};
int main()
{
    try{
        hellotriangle app;
        app.run();
    }
    catch(const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
