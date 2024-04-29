#include <vector>
#include <cassert>
#include <iostream>
#include <vulkan/vulkan.h>

int main(int argc, char** argv) {
    // vulkan extensions
    std::vector<const char *> enabledInstanceLayerNames{ "VK_LAYER_KHRONOS_validation" };
    std::vector<const char *> enabledInstanceExtensionNames{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
    std::vector<const char *> enabledDeviceLayerNames{ "VK_LAYER_KHRONOS_validation" };
    std::vector<const char *> enabledDeviceExtensionNames{ 
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
    };

    // get vulkan instance version
    uint32_t instanceVersion{};
    vkEnumerateInstanceVersion(&instanceVersion);
    std::cout << "Vulkan API version: ";
    std::cout << VK_API_VERSION_MAJOR(instanceVersion) << ".";
    std::cout << VK_API_VERSION_MINOR(instanceVersion) << ".";
    std::cout << VK_API_VERSION_PATCH(instanceVersion) << std::endl;

    // application info
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = VK_NULL_HANDLE;
    applicationInfo.pApplicationName = "cpp-vulkan-compute";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.pEngineName = "vulkan-compute-app";
    applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;
    // instance create info
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = VK_NULL_HANDLE;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledLayerCount = enabledInstanceLayerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayerNames.data();
    instanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames.data();
    // create instance
    VkInstance instance{};
    vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance);
    assert(instance);

    // get physical devices
    uint32_t physicalDevicesCount{};
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices.data());

    // VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo{};
    messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messengerCreateInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerCreateInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerCreateInfo.pfnUserCallback = [](
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) { std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl; return VK_FALSE; };
    messengerCreateInfo.pUserData = nullptr;
    VkDebugUtilsMessengerEXT debugUtilsMessengerEXT{};
    auto fnCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (fnCreateDebugUtilsMessengerEXT != nullptr)
        fnCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo, VK_NULL_HANDLE, &debugUtilsMessengerEXT);

    // get vulkan instance layer properties
    uint32_t instanceLayerPropertiesCount{};
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, VK_NULL_HANDLE);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertiesCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, instanceLayerProperties.data());
    for (const VkLayerProperties& layerProperty: instanceLayerProperties)
        std::cout << "\t" << layerProperty.layerName << ": " << layerProperty.description << std::endl;

    // get physical devices layer properties
    uint32_t instanceExtensionPropertiesCount{};
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &instanceExtensionPropertiesCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> instanceExtensionProperties(instanceExtensionPropertiesCount);
    vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &instanceExtensionPropertiesCount, instanceExtensionProperties.data());
    for (const VkExtensionProperties& extensionProperty: instanceExtensionProperties)
        std::cout << "\t" << extensionProperty.extensionName << ": " << extensionProperty.specVersion << std::endl;

    for (const VkPhysicalDevice& physicalDevice: physicalDevices) {
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        std::cout << physicalDeviceProperties.deviceName << std::endl;
        
        // get physical devices layer properties
        uint32_t deviceLayerPropertiesCount{};
        vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerPropertiesCount, VK_NULL_HANDLE);
        std::vector<VkLayerProperties> deviceLayerProperties(deviceLayerPropertiesCount);
        vkEnumerateDeviceLayerProperties(physicalDevice, &deviceLayerPropertiesCount, deviceLayerProperties.data());
        for (const VkLayerProperties& layerProperty: deviceLayerProperties)
            std::cout << "\t" << layerProperty.layerName << ": " << layerProperty.description << std::endl;

        // get physical devices layer properties
        uint32_t deviceExtensionPropertiesCount{};
        vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &deviceExtensionPropertiesCount, VK_NULL_HANDLE);
        std::vector<VkExtensionProperties> deviceExtensionProperties(deviceExtensionPropertiesCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &deviceExtensionPropertiesCount, deviceExtensionProperties.data());
        for (const VkExtensionProperties& extensionProperty: deviceExtensionProperties)
            std::cout << "\t" << extensionProperty.extensionName << ": " << extensionProperty.specVersion << std::endl;

        // get physical device queue family properties
        uint32_t deviceQueueFamilyPropertiesCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertiesCount, nullptr);
        std::vector<VkQueueFamilyProperties> deviceQueueFamilyProperties(deviceQueueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &deviceQueueFamilyPropertiesCount, deviceQueueFamilyProperties.data());
        for (const VkQueueFamilyProperties& queueFamilyProperty: deviceQueueFamilyProperties) {
            std::cout << "\t";
            std::cout << "VK_QUEUE_GRAPHICS_BIT: " << ((queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "True, " : "False,") << " ";
            std::cout << "VK_QUEUE_COMPUTE_BIT: " << ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) ? "True, " : "False,") << " ";
            std::cout << "VK_QUEUE_TRANSFER_BIT: " << ((queueFamilyProperty.queueFlags & VK_QUEUE_TRANSFER_BIT) ? "True, " : "False,") << " ";
            std::cout << "VK_QUEUE_SPARSE_BINDING_BIT: " << ((queueFamilyProperty.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ? "True, " : "False,") << " ";
            std::cout << "VK_QUEUE_VIDEO_DECODE_BIT_KHR: " << ((queueFamilyProperty.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) ? "True, " : "False,") << " ";
            std::cout << "VK_QUEUE_VIDEO_ENCODE_BIT_KHR: " << ((queueFamilyProperty.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) ? "True, " : "False,") << std::endl;
        }
    }

    // physical device features
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    physicalDeviceFeatures.shaderInt16 = VK_TRUE;
    physicalDeviceFeatures.shaderInt64 = VK_TRUE;
    // device queue create info
    float queuePriorities = 1.0f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = VK_NULL_HANDLE;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = 0;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;
    // device create info
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = VK_NULL_HANDLE;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.enabledLayerCount = enabledDeviceLayerNames.size();
    deviceCreateInfo.ppEnabledLayerNames = enabledDeviceLayerNames.data();
    deviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames.data();
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    // create device
    VkDevice device{};
    vkCreateDevice(physicalDevices[0], &deviceCreateInfo, VK_NULL_HANDLE, &device);

    // destroy handles
    vkDestroyDevice(device, VK_NULL_HANDLE);
    auto fnDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fnDestroyDebugUtilsMessengerEXT)
        fnDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessengerEXT, VK_NULL_HANDLE);
    vkDestroyInstance(instance, VK_NULL_HANDLE);
    return 0;
}
