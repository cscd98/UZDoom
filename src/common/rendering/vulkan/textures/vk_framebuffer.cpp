/*
** vk_framebuffer.cpp
**
** Vulkan backend
**
**---------------------------------------------------------------------------
**
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
** Copyright 2016-2020 Magnus Norddahl
**
** SPDX-License-Identifier: Zlib
**
**---------------------------------------------------------------------------
**
*/

#include <zvulkan/vulkanobjects.h>
#include <zvulkan/vulkandevice.h>
#include <zvulkan/vulkanbuilders.h>
#include <zvulkan/vulkanswapchain.h>
#include "vulkan/system/vk_renderdevice.h"
#include "vulkan/renderer/vk_postprocess.h"
#include "vk_framebuffer.h"
#ifdef __LIBRETRO__
#include "common/platform/libretro/retro_platform.h"
#endif

CVAR(Bool, vk_hdr, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG);
CVAR(Bool, vk_exclusivefullscreen, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG);

VkFramebufferManager::VkFramebufferManager(VulkanRenderDevice* fb) : fb(fb)
{
	SwapChain = VulkanSwapChainBuilder()
		.Create(fb->device.get());

	SwapChainImageAvailableSemaphore = SemaphoreBuilder()
		.DebugName("SwapChainImageAvailableSemaphore")
		.Create(fb->device.get());
}

VkFramebufferManager::~VkFramebufferManager()
{
}

void VkFramebufferManager::AcquireImage()
{
#ifdef __LIBRETRO__
	auto host = Libretro::Vulkan;
	host->wait_sync_index(host->handle);
	if (SwapChain->Lost())
	{
		unsigned mask  = host->get_sync_index_mask(host->handle);
		unsigned count = 0;
		while (mask)
		{
			++count;
			mask >>= 1;
		}
		if (!count)
			throw std::runtime_error("Invalid Vulkan sync index mask");

		SwapChain->CreateOffscreen(fb->GetClientWidth(), fb->GetClientHeight(), count);
		RetroImages.resize(count);

		for (unsigned i = 0; i < count; ++i)
		{
			auto &image                        = RetroImages[i];
			image.image_view                   = SwapChain->GetImageView(i)->view;
			image.image_layout                 = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image.create_info.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image.create_info.image            = SwapChain->GetImage(i)->image;
			image.create_info.viewType         = VK_IMAGE_VIEW_TYPE_2D;
			image.create_info.format           = SwapChain->Format().format;
			image.create_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			RenderFinishedSemaphores.push_back(SemaphoreBuilder().Create(fb->device.get()));
		}
	}

	PresentImageIndex = host->get_sync_index(host->handle);
	if (unsigned(PresentImageIndex) >= RetroImages.size())
		throw std::runtime_error("Invalid Vulkan sync index");

	fb->GetPostprocess()->DrawPresentTexture(fb->mOutputLetterbox, true, false);
#else
	bool exclusiveFullscreen = fb->IsFullscreen() && vk_exclusivefullscreen;
	if (SwapChain->Lost() || fb->GetClientWidth() != CurrentWidth || fb->GetClientHeight() != CurrentHeight || fb->GetVSync() != CurrentVSync || CurrentHdr != vk_hdr || CurrentExclusiveFullscreen != exclusiveFullscreen)
	{
		Framebuffers.clear();

		CurrentWidth = fb->GetClientWidth();
		CurrentHeight = fb->GetClientHeight();
		CurrentVSync = fb->GetVSync();
		CurrentHdr = vk_hdr;
		CurrentExclusiveFullscreen = exclusiveFullscreen;

		SwapChain->Create(CurrentWidth, CurrentHeight, CurrentVSync ? 2 : 3, CurrentVSync, CurrentHdr, CurrentExclusiveFullscreen);

		RenderFinishedSemaphores.clear();
		for (int i = 0; i < SwapChain->ImageCount(); i++)
		{
			RenderFinishedSemaphores.push_back(SemaphoreBuilder()
				.DebugName("RenderFinishedSemaphore")
				.Create(fb->device.get()));
		}
	}

	PresentImageIndex = SwapChain->AcquireImage(SwapChainImageAvailableSemaphore.get());
	if (PresentImageIndex != -1)
	{
		fb->GetPostprocess()->DrawPresentTexture(fb->mOutputLetterbox, true, false);
	}
#endif
}

void VkFramebufferManager::QueuePresent()
{
#ifdef __LIBRETRO__
	if (PresentImageIndex == -1)
		return;

	auto host      = Libretro::Vulkan;
	auto semaphore = RenderFinishedSemaphores[PresentImageIndex]->semaphore;

	host->set_image(host->handle, &RetroImages[PresentImageIndex], 1, &semaphore, VK_QUEUE_FAMILY_IGNORED);

	Libretro::Present();
#else
	if (PresentImageIndex != -1)
		SwapChain->QueuePresent(PresentImageIndex, RenderFinishedSemaphores[PresentImageIndex].get());
#endif
}
