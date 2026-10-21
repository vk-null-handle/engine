#pragma once
#include "defines.h"
#include "vk_types.h"

b8 create_buffer(
	VulkanDevice* device,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VulkanBuffer* out_buffer);

void destroy_buffer(VulkanDevice* device, VulkanBuffer* buffer);

void copy_buffer(VulkanDevice* device, VulkanBuffer src, VulkanBuffer dst, VkDeviceSize size);

void map_buffer(VulkanDevice* device, VulkanBuffer* buffer, VkDeviceSize size);
void unmap_buffer(VulkanDevice* device, VulkanBuffer* buffer);
