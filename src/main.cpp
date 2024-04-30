#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>
#include <vma/vk_mem_alloc.h>
#include <shaderc/shaderc.h>

// compute shader image write
const char* computeShader_ImageWrite = R"(
    #version 450
    struct SolidColor { vec4 color; };
    layout(set = 0, binding = 0, rgba8ui) uniform readonly  uimage2D inputImage;
    layout(set = 0, binding = 1, rgba8ui) uniform writeonly uimage2D outputImage;
    layout(set = 0, binding = 2, std140)  uniform ubo2 { SolidColor uSolidColor0; };
    void main() {
        return;
    }
)";

// compile compute shader
shaderc_compilation_result_t CompileComputeShader(shaderc_compiler_t compiler, std::string_view source) {
    shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, source.data(), source.size(), shaderc_glsl_default_compute_shader, "Compute shader", "main", nullptr);
    if (shaderc_result_get_compilation_status(result) == shaderc_compilation_status_success) return result;
    std::cout << "Shader compiler: " << shaderc_result_get_error_message(result) << std::endl;
    shaderc_result_release(result);
    return nullptr;
}

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
    assert(device);

    // get device queue
    VkQueue queue{};
    vkGetDeviceQueue(device, 0, 0, &queue);
    assert(queue);

    // create shader compiler
    shaderc_compiler_t shadercCompiler{};
    shadercCompiler = shaderc_compiler_initialize();
    assert(shadercCompiler);

    // compile shader
    shaderc_compilation_result_t computeShaderData{};
    computeShaderData = CompileComputeShader(shadercCompiler, computeShader_ImageWrite);
    assert(computeShaderData);
    // shader module create info
    VkShaderModuleCreateInfo computeShaderModuleCreateInfo{};
    computeShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    computeShaderModuleCreateInfo.pNext = VK_NULL_HANDLE;
    computeShaderModuleCreateInfo.flags = 0;
    computeShaderModuleCreateInfo.codeSize = shaderc_result_get_length(computeShaderData);
    computeShaderModuleCreateInfo.pCode = (uint32_t *)shaderc_result_get_bytes(computeShaderData);
    // create shader module
    VkShaderModule computeShaderModule{}; 
    vkCreateShaderModule(device, &computeShaderModuleCreateInfo, VK_NULL_HANDLE, &computeShaderModule);
    assert(computeShaderModule);
    // pipeline shader stage create info
    VkPipelineShaderStageCreateInfo computeShaderStageCreateInfo{};
    computeShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageCreateInfo.pNext = VK_NULL_HANDLE;
    computeShaderStageCreateInfo.flags = 0;
    computeShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageCreateInfo.module = computeShaderModule;
    computeShaderStageCreateInfo.pName = "main";
    computeShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;

    // descriptor set layout binding
    VkDescriptorSetLayoutBinding inputImage  { 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  1, VK_SHADER_STAGE_ALL, VK_NULL_HANDLE };
    VkDescriptorSetLayoutBinding outputImage { 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  1, VK_SHADER_STAGE_ALL, VK_NULL_HANDLE };
    VkDescriptorSetLayoutBinding solidColor  { 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL, VK_NULL_HANDLE };
    // descriptor set layout create info
    VkDescriptorSetLayoutBinding descSetLayoutBindings[] = { inputImage, outputImage, solidColor };
    VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo{};
    descSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descSetLayoutCreateInfo.pNext = VK_NULL_HANDLE;
    descSetLayoutCreateInfo.flags = 0;
    descSetLayoutCreateInfo.bindingCount = 3;
    descSetLayoutCreateInfo.pBindings = descSetLayoutBindings;
    // create descriptor set layout
    VkDescriptorSetLayout descSetLayout{};
    vkCreateDescriptorSetLayout(device, &descSetLayoutCreateInfo, VK_NULL_HANDLE, &descSetLayout);
    assert(descSetLayout);

    // pipeline layout create info
    VkDescriptorSetLayout descSetLayouts[] { descSetLayout };
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = VK_NULL_HANDLE;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = descSetLayouts;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;
    // create pypeline layout
    VkPipelineLayout pipelineLayout{};
    vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipelineLayout);
    assert(pipelineLayout);

    // compute pipeline create info
    VkComputePipelineCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = VK_NULL_HANDLE;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stage = computeShaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = 0;
    // create compute pipeline
    VkPipeline computePipeline;
    vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VK_NULL_HANDLE, &computePipeline);
    assert(computePipeline);

    // command pool create info
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = VK_NULL_HANDLE;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = 0;
    // create command pool
    VkCommandPool commandPool{};
    vkCreateCommandPool(device, &commandPoolCreateInfo, VK_NULL_HANDLE, &commandPool);
    assert(commandPool);

    // command buffer allocate info
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = VK_NULL_HANDLE;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    // create command buffer
    VkCommandBuffer commandBuffer{};
    vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer);
    assert(commandBuffer);

    // reset command pool and buffer
    vkResetCommandPool(device, commandPool, 0);
    vkResetCommandBuffer(commandBuffer, 0);
    
    // begin command buffer
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = VK_NULL_HANDLE;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = VK_NULL_HANDLE;
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 0, 0, 0, 0);
    //vkCmdDispatch(commandBuffer, 64, 64, 1);
    vkEndCommandBuffer(commandBuffer);

    // queue submit and wait
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = VK_NULL_HANDLE;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = VK_NULL_HANDLE;
    submitInfo.pWaitDstStageMask = VK_NULL_HANDLE;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = VK_NULL_HANDLE;
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    // free command buffer and destroy command pool
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    vkDestroyCommandPool(device, commandPool, VK_NULL_HANDLE);

    // destroy handles
    vkDestroyPipeline(device, computePipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(device, pipelineLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(device, descSetLayout, VK_NULL_HANDLE);
    vkDestroyShaderModule(device, computeShaderModule, VK_NULL_HANDLE);
    shaderc_result_release(computeShaderData);
    shaderc_compiler_release(shadercCompiler);
    vkDestroyDevice(device, VK_NULL_HANDLE);
    auto fnDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fnDestroyDebugUtilsMessengerEXT)
        fnDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessengerEXT, VK_NULL_HANDLE);
    vkDestroyInstance(instance, VK_NULL_HANDLE);
    return 0;
}
