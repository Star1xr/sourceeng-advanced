/*
 * Minimal Vulkan surface present-mode definitions for the MangoHud HUD core.
 *
 * MangoHud's HUD code references VkPresentModeKHR for its vsync/present-mode
 * display even when built without the Vulkan hook. The Source Engine overlay
 * only uses the OpenGL/GLES renderer, so pulling in the full Vulkan headers
 * (and the loader) is unnecessary.
 */
#pragma once

#ifndef MANGOHUD_MINIMAL_VULKAN_H
#define MANGOHUD_MINIMAL_VULKAN_H

typedef enum VkPresentModeKHR {
	VK_PRESENT_MODE_IMMEDIATE_KHR = 0,
	VK_PRESENT_MODE_MAILBOX_KHR = 1,
	VK_PRESENT_MODE_FIFO_KHR = 2,
	VK_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
	VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR = 1000111000,
	VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR = 1000111001,
} VkPresentModeKHR;

#endif /* MANGOHUD_MINIMAL_VULKAN_H */
