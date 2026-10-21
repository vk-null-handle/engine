#include "vk_backend.h"
#include "core/logger.h"
#include "core/assert.h"
#include "core/memory.h"

#include "defines.h"
#include "renderer/renderer_types.h"
#include "vk_utils.h"
#include "vk_swapchain.h"
#include "vk_types.h"
#include "vk_device.h"
#include "vk_pipeline.h"
#include "vk_sync.h"
#include "vk_cmd_buffer.h"
#include "vk_buffer.h"
#include "vk_image.h"
#include <vulkan/vulkan_core.h>

// TEMP
static VulkanContext vkcontext;
const Vertex3D vertices[] = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
};
const u32 indices[] = {
	0, 1, 2, 2, 3, 0};

b8 vulkan_backend_init(Window* window) {
	static const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
	u32 n_instance_exts;
	const char** exts = win_get_vk_instance_ext(&n_instance_exts);

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkApplicationInfo.html
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "Engine",
		.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
		.apiVersion = VK_API_VERSION_1_4,
	};

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkInstanceCreateInfo.html
	VkInstanceCreateInfo inst_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = layers,
		.enabledExtensionCount = n_instance_exts,
		.ppEnabledExtensionNames = exts,
	};

	VK_CHECK(vkCreateInstance(&inst_info, NULL, &vkcontext.instance.handle));
	LOG_DEBUG("Created Vulkan instance");

	VK_CHECK(win_create_vk_surface(window, vkcontext.instance.handle, &vkcontext.surface));
	LOG_DEBUG("Created Vulkan surface");

	ASSERT(vk_device_create(&vkcontext.device, vkcontext.surface, &vkcontext.instance));
	LOG_DEBUG("Created Vulkan logical device");

	VkExtent2D extent;
	win_get_framebuffer_size(window, &extent.width, &extent.height);
	if (extent.width == 0 || extent.height == 0) {
		extent.width = 1920;
		extent.height = 800;
	}

	ASSERT(vk_swapchain_create(&vkcontext.device, &vkcontext.swapchain, vkcontext.surface, &extent));
	LOG_DEBUG("Created Vulkan swapchain");

	vkcontext.pipeline.vert_shader = vk_shader_load(&vkcontext.device, "shader_vert.glsl", shaderc_vertex_shader);
	vkcontext.pipeline.frag_shader = vk_shader_load(&vkcontext.device, "shader_frag.glsl", shaderc_fragment_shader);
	ASSERT(vk_graphics_pipeline_create(&vkcontext.device, &vkcontext.swapchain.image_format.format, &vkcontext.pipeline));
	LOG_DEBUG("Created Vulkan graphics pipeline");

	vk_sync_resources_create(&vkcontext);
	LOG_DEBUG("Created Vulkan sync resources");

	vk_command_buffers_create(&vkcontext);
	LOG_DEBUG("Created Vulkan command buffers and pool");

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkDrawIndexedIndirectCommand.html
	VkDrawIndexedIndirectCommand draw_command = {
		.indexCount = ARRAY_COUNT(indices),
		.instanceCount = 1,
		.firstIndex = 0,
		.vertexOffset = 0,
		.firstInstance = 0,
	};

	// Create indirect buffer
	ASSERT(create_buffer(&vkcontext.device,
						 sizeof(VkDrawIndirectCommand),
						 VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						 &vkcontext.indirect_buffer));
	// Map it to cpu memory
	map_buffer(&vkcontext.device, &vkcontext.indirect_buffer, sizeof(draw_command));
	mem_copy(vkcontext.indirect_buffer.mapped, &draw_command, sizeof(draw_command));
	unmap_buffer(&vkcontext.device, &vkcontext.indirect_buffer);

	// Vertex buffer
	{
		VkDeviceSize buffer_size = sizeof(vertices[0]) * ARRAY_COUNT(vertices);
		VulkanBuffer staging_buffer;
		ASSERT(create_buffer(&vkcontext.device,
							 buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							 &staging_buffer));
		map_buffer(&vkcontext.device, &staging_buffer, buffer_size);
		mem_copy(staging_buffer.mapped, vertices, buffer_size);
		unmap_buffer(&vkcontext.device, &staging_buffer);

		ASSERT(create_buffer(&vkcontext.device, buffer_size,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							 &vkcontext.vertex_buffer));
		copy_buffer(&vkcontext.device, staging_buffer, vkcontext.vertex_buffer, buffer_size);
		destroy_buffer(&vkcontext.device, &staging_buffer);
	}
	// Index buffer
	{
		VkDeviceSize buffer_size = sizeof(indices[0]) * ARRAY_COUNT(indices);
		VulkanBuffer staging_buffer;
		ASSERT(create_buffer(&vkcontext.device,
							 buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							 &staging_buffer));
		map_buffer(&vkcontext.device, &staging_buffer, buffer_size);
		mem_copy(staging_buffer.mapped, indices, buffer_size);
		unmap_buffer(&vkcontext.device, &staging_buffer);

		ASSERT(create_buffer(&vkcontext.device, buffer_size,
							 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							 &vkcontext.index_buffer));
		copy_buffer(&vkcontext.device, staging_buffer, vkcontext.index_buffer, buffer_size);
		destroy_buffer(&vkcontext.device, &staging_buffer);
	}

	return TRUE;
}

void vulkan_backend_shutdown(void) {
	VK_CHECK(vkDeviceWaitIdle(vkcontext.device.logical_dev));

	if (vkcontext.sync.timeline) {
		vkDestroySemaphore(vkcontext.device.logical_dev, vkcontext.sync.timeline, NULL);
	}

	for (u8 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		struct frame_resources* frame = &vkcontext.frame_resources[i];
		if (frame->image_acquired) {
			vkDestroySemaphore(vkcontext.device.logical_dev, frame->image_acquired, NULL);
		}
	}

	vk_pipeline_destroy(&vkcontext.device, &vkcontext.pipeline);

	if (vkcontext.sync.render_complete) {
		for (u32 i = 0; i < vkcontext.swapchain.n_images; i++) {
			vkDestroySemaphore(vkcontext.device.logical_dev, vkcontext.sync.render_complete[i], NULL);
		}
		mem_free(vkcontext.sync.render_complete, vkcontext.swapchain.n_images * sizeof(VkSemaphore));
	}

	vk_swapchain_destroy(&vkcontext.device, &vkcontext.swapchain);

	if (vkcontext.surface) {
		vkDestroySurfaceKHR(vkcontext.instance.handle, vkcontext.surface, NULL);
	};

	destroy_buffer(&vkcontext.device, &vkcontext.vertex_buffer);
	destroy_buffer(&vkcontext.device, &vkcontext.index_buffer);
	destroy_buffer(&vkcontext.device, &vkcontext.indirect_buffer);
	vk_device_destroy(&vkcontext.device);

	if (vkcontext.instance.handle) {
		vkDestroyInstance(vkcontext.instance.handle, NULL);
	}
}

void vulkan_backend_render_frame(Window* window) {
	// Recreate swapchain if needed
	if (vkcontext.swapchain.require_recreate) {
		VkExtent2D extent;
		win_get_framebuffer_size(window, &extent.width, &extent.height);
		if (extent.width == 0 || extent.height == 0) {
			return; // Wait until the window is restored.
		}

		vkDeviceWaitIdle(vkcontext.device.logical_dev);
		vk_swapchain_destroy(&vkcontext.device, &vkcontext.swapchain);
		vk_swapchain_create(&vkcontext.device, &vkcontext.swapchain, vkcontext.surface, &extent);
		vkcontext.swapchain.require_recreate = FALSE;
		LOG_DEBUG("Swapchain resized");
	}

	// Which frame's synchronization resources to use
	u32 frame_res_index = vkcontext.sync.frame_index++ % MAX_FRAMES_IN_FLIGHT;
	// Value current frame will set timeline semaphore when it completes,
	// so that future frames know what to wait on until resources are avaliable
	u64 signal_value = vkcontext.sync.next_signal_value++;
	// Value current frame will wait until it begins using the resources
	const u64 wait_value = signal_value - MAX_FRAMES_IN_FLIGHT;

	// https://docs.vulkan.org/refpages/latest/refpages/source/VkSemaphoreWaitInfo.html
	VkSemaphoreWaitInfo wait_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		// Only waiting for the one semaphore
		.semaphoreCount = 1,
		.pSemaphores = &vkcontext.sync.timeline,
		.pValues = &wait_value,
	};
	// Wiat for timeline semaphore to reach signal value
	vkWaitSemaphores(vkcontext.device.logical_dev, &wait_info, UINT64_MAX);

	// Now its safe to start recording commands
	FrameResources* frame_res = &vkcontext.frame_resources[frame_res_index];
	// Reset command buffer
	vkResetCommandBuffer(frame_res->command_buffer, 0);

	// Sempaphore to singal when swapchain image is acquired
	VkSemaphore image_acquire = vkcontext.frame_resources[frame_res_index].image_acquired;
	// Get swapchain image for this frame
	u32 image_index = 0;
	VkResult acquire_result = vkAcquireNextImageKHR(vkcontext.device.logical_dev, vkcontext.swapchain.handle, UINT64_MAX, image_acquire, NULL, &image_index);

	// Handle resize and out-of-date images
	if (acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
		vkcontext.swapchain.require_recreate = TRUE;
		return;
	}
	if (acquire_result == VK_SUBOPTIMAL_KHR) {
		// Can render this frame but recreate next frame
		vkcontext.swapchain.require_recreate = TRUE;
	}

	// Begin recording commands
	// https://docs.vulkan.org/refpages/latest/refpages/source/VkCommandBufferBeginInfo.html
	VkCommandBufferBeginInfo cmd_begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		// Only be submited to queue once before needing to be reset
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(frame_res->command_buffer, &cmd_begin_info);

	// Transition the color and depth images
	transition_image(frame_res->command_buffer, vkcontext.swapchain.images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	transition_image(frame_res->command_buffer, vkcontext.swapchain.depth_image.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	// Setup the attachments (color and depth) and begin rendering (dynamic)
	VkRenderingAttachmentInfo color_attach_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = vkcontext.swapchain.image_views[image_index],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,	 // clear the image
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE, // keep data for presentation
		// Clear color
		.clearValue = {
			.color = {0.01f, 0.01f, 0.01f, 1},
		},
	};
	VkRenderingAttachmentInfo depth_attach_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = vkcontext.swapchain.depth_image.view,
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,		 // clear the depth data
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE, // don't care after rendering
		.clearValue = {
			.depthStencil = {1.0f, 0},
		},
	};

	VkRenderingInfo rendering_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {
			.offset = {.x = 0, .y = 0},
			.extent = {
				.width = vkcontext.swapchain.width,
				.height = vkcontext.swapchain.height,
			},
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attach_info,
		.pDepthAttachment = &depth_attach_info,
	};

	// Begin dynamic rendering
	vkCmdBeginRendering(frame_res->command_buffer, &rendering_info);
	{
		// Set viewport and scissor state
		VkViewport viewport = {
			.x = 0,
			.y = 0,
			.width = (float)vkcontext.swapchain.width,
			.height = (float)vkcontext.swapchain.height,
		};
		vkCmdSetViewport(frame_res->command_buffer, 0, 1, &viewport);

		VkRect2D scissor = {
			.offset = {.x = 0, .y = 0},
			.extent = {
				.width = vkcontext.swapchain.width,
				.height = vkcontext.swapchain.height,
			},
		};
		vkCmdSetScissor(frame_res->command_buffer, 0, 1, &scissor);

		// Draw triangle
		vkCmdBindPipeline(frame_res->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkcontext.pipeline.handle);

		VkBuffer vertex_buffers[] = {vkcontext.vertex_buffer.handle};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(frame_res->command_buffer, 0, 1, vertex_buffers, offsets);
		vkCmdBindIndexBuffer(frame_res->command_buffer, vkcontext.index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexedIndirect(frame_res->command_buffer, vkcontext.indirect_buffer.handle, 0, 1, sizeof(VkDrawIndexedIndirectCommand));
	}

	// End dynamic rendering
	vkCmdEndRendering(frame_res->command_buffer);

	// Transition the swapchain image from color attachment to presentation so we can show it
	transition_image(frame_res->command_buffer, vkcontext.swapchain.images[image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// End command buffer
	vkEndCommandBuffer(frame_res->command_buffer);

	// Ensure swapchain image is actually vailable to start color output
	// https://docs.vulkan.org/refpages/latest/refpages/source/VkSemaphoreSubmitInfo.html
	VkSemaphoreSubmitInfo image_acquire_wait_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = image_acquire,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, // Wait before drawing to image
	};
	// signal that the image can be presented
	VkSemaphoreSubmitInfo semaphore_signals[] = {
		{// Render work completion signal
		 .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		 .semaphore = vkcontext.sync.render_complete[image_index],
		 .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT},
		{
			// Entire frame is completed (timeline)
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = vkcontext.sync.timeline,
			.value = signal_value,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		},
	};
	u32 n_semaphore_signals = sizeof(semaphore_signals) / sizeof(semaphore_signals[0]);

	VkCommandBufferSubmitInfo cmd_submit_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = frame_res->command_buffer,
	};

	VkSubmitInfo2 submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &image_acquire_wait_info, // Ensure the image is ready
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmd_submit_info,
		.signalSemaphoreInfoCount = n_semaphore_signals,
		.pSignalSemaphoreInfos = semaphore_signals,
	};
	vkQueueSubmit2(vkcontext.device.gfx_queue, 1, &submit_info, NULL);

	// Present the image
	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &vkcontext.sync.render_complete[image_index], // Render work completed semaphore to set
		.swapchainCount = 1,
		.pSwapchains = &vkcontext.swapchain.handle,
		.pImageIndices = &image_index,
		.pResults = NULL,
	};

	vkQueuePresentKHR(vkcontext.device.gfx_queue, &present_info);
}
