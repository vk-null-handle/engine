#include "vk_buffer.h"
#include <vulkan/vulkan_core.h>
#include "core/assert.h"
#include "renderer/vulkan/vk_types.h"
#include "vk_utils.h"

b8 create_buffer(
	VulkanDevice* device,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VulkanBuffer* out_buffer) {

	VkBufferCreateInfo buffer_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	VK_CHECK(vkCreateBuffer(device->logical_dev, &buffer_info, NULL, &out_buffer->handle));

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(device->logical_dev, out_buffer->handle, &requirements);

	i32 index = find_memory_index(device, requirements.memoryTypeBits, properties);
	ASSERT(index >= 0);

	VkMemoryAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = requirements.size,
		.memoryTypeIndex = index,
	};

	VK_CHECK(vkAllocateMemory(device->logical_dev, &alloc_info, NULL, &out_buffer->memory));
	VK_CHECK(vkBindBufferMemory(device->logical_dev, out_buffer->handle, out_buffer->memory, 0));

	out_buffer->size = size;
	return TRUE;
}

void destroy_buffer(VulkanDevice* device, VulkanBuffer* buffer) {
	vkDestroyBuffer(device->logical_dev, buffer->handle, NULL);
	vkFreeMemory(device->logical_dev, buffer->memory, NULL);
}

void copy_buffer(VulkanDevice* device, VulkanBuffer src, VulkanBuffer dst, VkDeviceSize size) {
	VkCommandBufferAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
		.commandPool = device->command_pool,
	};

	VkCommandBuffer cmd_buffer;
	vkAllocateCommandBuffers(device->logical_dev, &alloc_info, &cmd_buffer);

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(cmd_buffer, &begin_info);
	VkBufferCopy copy_region = {
		.size = size,
	};
	vkCmdCopyBuffer(cmd_buffer, src.handle, dst.handle, 1, &copy_region);
	vkEndCommandBuffer(cmd_buffer);

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd_buffer,
	};

	vkQueueSubmit(device->gfx_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(device->gfx_queue);

	vkFreeCommandBuffers(device->logical_dev, device->command_pool, 1, &cmd_buffer);
}

void map_buffer(VulkanDevice* device, VulkanBuffer* buffer, VkDeviceSize size) {
	VK_CHECK(vkMapMemory(device->logical_dev, buffer->memory, 0, size, 0, &buffer->mapped));
}

void unmap_buffer(VulkanDevice* device, VulkanBuffer* buffer) {
	if (buffer->mapped) {
		vkUnmapMemory(device->logical_dev, buffer->memory);
	}
}
