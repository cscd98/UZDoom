#pragma once
#ifdef HAVE_VULKAN
#include <zvulkan/vulkandevice.h>
#include "libretro_vulkan.h"

namespace Libretro
{
extern const retro_hw_render_interface_vulkan *Vulkan;
bool NegotiateVulkan();
bool ResetVulkan();
std::unique_ptr<VulkanDevice> BorrowVulkanDevice();
void LockVulkanQueue();
void UnlockVulkanQueue();
} // namespace Libretro
#endif
