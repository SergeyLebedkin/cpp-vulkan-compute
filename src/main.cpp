#include <vector>
#include <cassert>
#include <iostream>
#include <vulkan/vulkan.h>

int main(int argc, char** argv) {
    // get vulkan instance version
    uint32_t instanceVersion{};
    vkEnumerateInstanceVersion(&instanceVersion);
    std::cout << "Vulkan API version: " << ".";
    std::cout << VK_API_VERSION_MAJOR(instanceVersion) << ".";
    std::cout << VK_API_VERSION_MINOR(instanceVersion) << ".";
    std::cout << VK_API_VERSION_PATCH(instanceVersion) << std::endl;

    // get vulkan instance layer properties
    uint32_t instanceLayerPropertiesCount{};
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, VK_NULL_HANDLE);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerPropertiesCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerPropertiesCount, instanceLayerProperties.data());
    for (const VkLayerProperties& layerProperty: instanceLayerProperties)
        std::cout << layerProperty.layerName << ": " << layerProperty.description << std::endl;

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
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.ppEnabledLayerNames = VK_NULL_HANDLE;
    instanceCreateInfo.enabledExtensionCount = 0;
    instanceCreateInfo.ppEnabledExtensionNames = VK_NULL_HANDLE;
    // create instance
    VkInstance instance{};
    vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance);
    assert(instance);

    // destroy handles
    vkDestroyInstance(instance, VK_NULL_HANDLE);
    return 0;
}
