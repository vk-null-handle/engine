#include "vk_device.h"

#include "core/assert.h"
#include "core/logger.h"
#include "core/memory.h"
#include "renderer/vulkan/vk_types.h"
#include "renderer/vulkan/vk_utils.h"
#include <vulkan/vulkan_core.h>

static b8 find_phys_dev(VulkanInstance* instance, VulkanDevice* out_device) {
	u32 n_phys_dev;
	vkEnumeratePhysicalDevices(instance->handle, &n_phys_dev, NULL);
	VkPhysicalDevice phys_devs[n_phys_dev];
	vkEnumeratePhysicalDevices(instance->handle, &n_phys_dev, phys_devs);

	VkPhysicalDevice phys_dev = phys_devs[0]; // Default to first GPU
	for (u32 i = 0; i < n_phys_dev; i++) {
		VkPhysicalDeviceProperties props = {};
		vkGetPhysicalDeviceProperties(phys_devs[i], &props);

		// Get first device
		phys_dev = phys_devs[i];
		out_device->properties = props;
		out_device->physical_dev = phys_dev;
		return TRUE;
	};

	return FALSE;
}

static b8 find_graphics_queue(VulkanDevice* device, VkSurfaceKHR surface) {
	u32 n_queue_fams;
	vkGetPhysicalDeviceQueueFamilyProperties2(device->physical_dev, &n_queue_fams, NULL);
	VkQueueFamilyProperties2 queue_fam_props[n_queue_fams];

	for (u32 i = 0; i < n_queue_fams; ++i) {
		queue_fam_props[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		queue_fam_props[i].pNext = NULL;
	}
	vkGetPhysicalDeviceQueueFamilyProperties2(device->physical_dev, &n_queue_fams, queue_fam_props);

	for (int current_fam_idx = 0; current_fam_idx < n_queue_fams; current_fam_idx++) {
		// Ensure it has presentation support
		VkBool32 hasPresentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(device->physical_dev, current_fam_idx, surface, &hasPresentSupport);

		const VkQueueFamilyProperties2* props = &queue_fam_props[current_fam_idx];
		// Ensure this is a graphics queue with presentation support
		if (props->queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport) {
			device->gfx_queue_fam_idx = current_fam_idx;
			return TRUE;
		}
	}
	return FALSE;
}

void vk_device_query_swapchain_support_info(VkSurfaceKHR surface, VulkanDevice* device, VulkanSwapchainSupportInfo* out_info) {
	// Surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical_dev, surface, &out_info->capabilities);
	// Surface formats
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_dev, surface, &out_info->n_formats, NULL);
	if (out_info->n_formats > 0) {
		if (!out_info->formats) {
			out_info->formats = mem_alloc(sizeof(VkSurfaceFormatKHR) * out_info->n_formats);
		}
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_dev, surface, &out_info->n_formats, out_info->formats);
	}

	// Present modes
	vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical_dev, surface, &out_info->n_present_modes, NULL);
	if (out_info->n_present_modes > 0) {
		if (!out_info->present_modes) {
			out_info->present_modes = mem_alloc(sizeof(VkPresentModeKHR) * out_info->n_present_modes);
		}
		vkGetPhysicalDeviceSurfacePresentModesKHR(device->physical_dev, surface, &out_info->n_present_modes, out_info->present_modes);
	}
}

b8 vk_device_create(VulkanDevice* device, VkSurfaceKHR surface, VulkanInstance* instance) {
	if (!find_phys_dev(instance, device)) {
		LOG_ERROR("Failed to find appropriate physical device");
		return FALSE;
	}
	LOG_DEBUG("Picked physical device: %s", device->properties.deviceName);

	ASSERT(find_graphics_queue(device, surface));

	// Query supported features
	VkPhysicalDeviceVulkan14Features supportedFeatures14 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
		.pNext = NULL,
	};
	VkPhysicalDeviceVulkan13Features supportedFeatures13 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &supportedFeatures14,
	};
	VkPhysicalDeviceVulkan12Features supportedFeatures12 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &supportedFeatures13,
	};
	VkPhysicalDeviceFeatures2 supportedFeatures = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &supportedFeatures12,
	};
	vkGetPhysicalDeviceFeatures2(device->physical_dev, &supportedFeatures);

	// Check if what is need is supported
	if (!supportedFeatures13.dynamicRendering || !supportedFeatures13.synchronization2 || !supportedFeatures12.timelineSemaphore) {
		LOG_ERROR("Physical device doesn't meet the feature requirements");
		return FALSE;
	}

	// Produce a separate features struct chain for device creation
	VkPhysicalDeviceVulkan14Features features14 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
		.pNext = NULL,
	};
	VkPhysicalDeviceVulkan13Features features13 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.pNext = &features14,
		.synchronization2 = VK_TRUE,
		.dynamicRendering = VK_TRUE,
	};
	VkPhysicalDeviceVulkan12Features features12 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		.pNext = &features13,
		.timelineSemaphore = VK_TRUE,
	};
	VkPhysicalDeviceFeatures2 features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &features12,
	};

	f32 queue_priorities = 1.0f;
	VkDeviceQueueCreateInfo gfx_queue_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = device->gfx_queue_fam_idx,
		// Just one queue
		.queueCount = 1,
		.pQueuePriorities = &queue_priorities,
	};

	u32 n_device_exts = 1;
	const char* device_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkDeviceCreateInfo.html
	VkDeviceCreateInfo device_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &features,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &gfx_queue_info,
		.enabledExtensionCount = n_device_exts,
		.ppEnabledExtensionNames = device_exts,
		.pEnabledFeatures = NULL, // Features struct chain is in .pNext
	};

	VK_CHECK(vkCreateDevice(device->physical_dev, &device_info, NULL, &device->logical_dev));

	// Get the graphics queue
	vkGetDeviceQueue(device->logical_dev, device->gfx_queue_fam_idx, 0, &device->gfx_queue);
	ASSERT(device->gfx_queue);

	// TODO: Device depth format checking
	device->depth_format = VK_FORMAT_D32_SFLOAT;

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkCommandPoolCreateInfo.html
	VkCommandPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = device->gfx_queue_fam_idx,
	};
	VK_CHECK(vkCreateCommandPool(device->logical_dev, &pool_info, NULL, &device->command_pool));

	return TRUE;
}

void vk_device_destroy(VulkanDevice* device) {
	if (device->command_pool) {
		vkDestroyCommandPool(device->logical_dev, device->command_pool, NULL);
	}
	if (device->logical_dev) {
		vkDestroyDevice(device->logical_dev, NULL);
	}
	mem_free(device->swapchain_info.formats, sizeof(VkSurfaceFormatKHR) * device->swapchain_info.n_formats);
	mem_free(device->swapchain_info.present_modes, sizeof(VkPresentModeKHR) * device->swapchain_info.n_present_modes);
}
