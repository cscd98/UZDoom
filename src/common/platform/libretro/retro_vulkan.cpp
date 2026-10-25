#include "retro_platform.h"
#include "retro_vulkan.h"

#ifdef HAVE_VULKAN
#include <vector>
namespace Libretro
{
const retro_hw_render_interface_vulkan *Vulkan = nullptr;
namespace
{
VkPhysicalDeviceFeatures enabled{};
bool negotiated = false;

const VkApplicationInfo *ApplicationInfo()
{
    static const VkApplicationInfo info{VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr,
        "UZDoom", 1, "UZDoom", 1, VK_API_VERSION_1_1};
    return &info;
}

bool CreateDevice(retro_vulkan_context *context, VkInstance instance, VkPhysicalDevice gpu,
    VkSurfaceKHR surface, PFN_vkGetInstanceProcAddr getProc,
    const char **extensions, unsigned extensionCount, const char **layers, unsigned layerCount,
    const VkPhysicalDeviceFeatures *)
{
    volkInitializeCustom(getProc);
    volkLoadInstance(instance);
    std::vector<VkPhysicalDevice> devices;

    if (gpu)
        devices.push_back(gpu);
    else
    {
        uint32_t count = 0;
        if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS)
            return false;

        devices.resize(count);
        if (vkEnumeratePhysicalDevices(instance, &count, devices.data()) != VK_SUCCESS)
            return false;
    }
    for (auto physical : devices)
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, nullptr);
        
        std::vector<VkQueueFamilyProperties> families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical, &count, families.data());

        for (unsigned i = 0; i < count; ++i)
        {
            if ((families[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) !=
                (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT))
                continue;

            VkBool32 present = !surface;
            if (surface)
                vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &present);

            if (!present)
                continue;

            VkPhysicalDeviceFeatures features{};
            vkGetPhysicalDeviceFeatures(physical, &features);
            if (!features.depthClamp || !features.samplerAnisotropy)
                continue;

            float priority = 1.0f;
            VkDeviceQueueCreateInfo queue{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
            queue.queueFamilyIndex = i; queue.queueCount = 1; queue.pQueuePriorities = &priority;
            VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
            info.queueCreateInfoCount = 1; info.pQueueCreateInfos = &queue;
            info.enabledExtensionCount = extensionCount; info.ppEnabledExtensionNames = extensions;
            info.enabledLayerCount = layerCount; info.ppEnabledLayerNames = layers;
            info.pEnabledFeatures = &features;

            VkDevice device{};
            if (vkCreateDevice(physical, &info, nullptr, &device) != VK_SUCCESS)
                continue;

            volkLoadDevice(device);
            context->gpu = physical; context->device = device;
            context->queue_family_index = context->presentation_queue_family_index = i;
            vkGetDeviceQueue(device, i, 0, &context->queue);
            context->presentation_queue = context->queue;
            enabled = features;
            negotiated = true;

            return true;
        }
    }
    return false;
}
} // anonymous namespace

bool NegotiateVulkan()
{
    negotiated = false;
    Vulkan = nullptr;
    static const retro_hw_render_context_negotiation_interface_vulkan negotiation = {
        RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN, 1, ApplicationInfo, CreateDevice,
        nullptr, nullptr, nullptr};

    return Environment(RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE,
        const_cast<retro_hw_render_context_negotiation_interface_vulkan *>(&negotiation));
}
bool ResetVulkan()
{
    if (!negotiated || !Environment(RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE, &Vulkan) || !Vulkan ||
        Vulkan->interface_type != RETRO_HW_RENDER_INTERFACE_VULKAN ||
        Vulkan->interface_version != RETRO_HW_RENDER_INTERFACE_VULKAN_VERSION)
        return false;

    return Vulkan->set_image && Vulkan->wait_sync_index && Vulkan->get_sync_index &&
        Vulkan->get_sync_index_mask && Vulkan->lock_queue && Vulkan->unlock_queue;
}
std::unique_ptr<VulkanDevice> BorrowVulkanDevice()
{
    if (!Vulkan) throw std::runtime_error("Frontend Vulkan interface is unavailable");
    auto instance = std::make_shared<VulkanInstance>(Vulkan->instance,
        Vulkan->get_instance_proc_addr, VK_API_VERSION_1_1);
    return std::make_unique<VulkanDevice>(instance, Vulkan->gpu, Vulkan->device,
        Vulkan->queue, Vulkan->queue_index, enabled);
}

void LockVulkanQueue()
{
    if (Vulkan)
        Vulkan->lock_queue(Vulkan->handle);
}

void UnlockVulkanQueue()
{
    if (Vulkan)
        Vulkan->unlock_queue(Vulkan->handle);
}
}
#endif
