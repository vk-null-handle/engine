#include "vk_pipeline.h"
#include "defines.h"
#include "vk_types.h"
#include "vk_utils.h"
#include <vulkan/vulkan_core.h>

b8 vk_graphics_pipeline_create(VulkanDevice* device, VkFormat* format, VulkanPipeline* out_pipeline) {
	// Define pipeline layout
	VkPipelineLayoutCreateInfo pipeline_layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		// Has no descriptor sets or push constants
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0,
	};
	VK_CHECK(vkCreatePipelineLayout(device->logical_dev, &pipeline_layout_info, NULL, &out_pipeline->layout));

	// Define main shader function
	const char* entry_point = "main";
	// Configure shader stages
	VkPipelineShaderStageCreateInfo shader_stages[] = {
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = out_pipeline->vert_shader,
			.pName = entry_point,
		},
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = out_pipeline->frag_shader,
			.pName = entry_point,
		},
	};
	u8 n_shader_stages = ARRAY_COUNT(shader_stages);

	VkVertexInputBindingDescription binding_description = {
		.binding = 0,
		.stride = sizeof(Vertex3D),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	VkVertexInputAttributeDescription attribute_descriptions[] = {
		{
			.binding = 0,
			.location = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex3D, position),
		},
		{
			.binding = 0,
			.location = 1,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex3D, color),
		},
	};
	u8 n_attribute_descriptions = ARRAY_COUNT(attribute_descriptions);

	VkPipelineVertexInputStateCreateInfo vert_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexAttributeDescriptionCount = n_attribute_descriptions,
		.vertexBindingDescriptionCount = 1,
		.pVertexAttributeDescriptions = attribute_descriptions,
		.pVertexBindingDescriptions = &binding_description,
	};

	// Input assembly, tell Vulkan to draw triangle lists
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	// Depth/Stencil configuration
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.stencilTestEnable = VK_FALSE,
	};

	VkPipelineViewportStateCreateInfo viewport_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = NULL,
		.scissorCount = 1,
		.pScissors = NULL,
	};

	// Rasterizer settings
	VkPipelineRasterizationStateCreateInfo raster_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		// Full triangles
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		// Triangles vertices are clockwise
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.lineWidth = 1.0f,
	};

	// No multisampling
	VkPipelineMultisampleStateCreateInfo multisample_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};

	// Alpha blending (disabled)
	VkPipelineColorBlendAttachmentState attach_state = {
		.blendEnable = VK_FALSE,
		// Which colors to draw on swapchain
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo blend_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = &attach_state,
	};

	// We want to be able to change scissor and viewport at runtime
	VkDynamicState dynamic_state[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	u8 n_dynamic_states = ARRAY_COUNT(dynamic_state);

	VkPipelineDynamicStateCreateInfo dynamic_state_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = n_dynamic_states,
		.pDynamicStates = dynamic_state,
	};

	// Structure required for dynamic rendering
	VkPipelineRenderingCreateInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		// Single color attachment (Swapchain)
		.colorAttachmentCount = 1,
		// Set swapchains color and devices depth formats
		.pColorAttachmentFormats = format,
		.depthAttachmentFormat = device->depth_format,
	};

	// Create the graphics pipeline
	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &render_info,
		// Setup shader stages and associated them with vkshader modules
		.stageCount = n_shader_stages,
		.pStages = shader_stages,
		// Blank vertex input since vertexs in shader
		.pVertexInputState = &vert_input_info,
		// Input assembly state to tell pipeline we want triangle list topology
		.pInputAssemblyState = &input_assembly_info,
		// Tell Vulkan we want single viewport and scissor test, but didnt create them statically
		.pViewportState = &viewport_info,
		// Told rasterizer we want back faces culled and front faces filled
		.pRasterizationState = &raster_info,
		// Not using multisampling
		.pMultisampleState = &multisample_info,
		// Enabled depth testing and writing but not stenciling
		.pDepthStencilState = &depth_stencil_info,
		// Configured color blending (Disabled)
		.pColorBlendState = &blend_info,
		// Set viewport and scissor states to be dynamic
		.pDynamicState = &dynamic_state_info,
		// Created pipeline layout not expecting and descriptor sets or push constants
		.layout = out_pipeline->layout,
		// Disabled render passes
		.renderPass = VK_NULL_HANDLE,
	};

	VK_CHECK(vkCreateGraphicsPipelines(device->logical_dev, NULL, 1, &pipeline_info, NULL, &out_pipeline->handle));

	return TRUE;
}

void vk_pipeline_destroy(VulkanDevice* device, VulkanPipeline* pipeline) {
	if (pipeline->vert_shader) {
		vkDestroyShaderModule(device->logical_dev, pipeline->vert_shader, NULL);
	}
	if (pipeline->frag_shader) {
		vkDestroyShaderModule(device->logical_dev, pipeline->frag_shader, NULL);
	}
	if (pipeline->comp_shader) {
		vkDestroyShaderModule(device->logical_dev, pipeline->comp_shader, NULL);
	}

	if (pipeline->layout) {
		vkDestroyPipelineLayout(device->logical_dev, pipeline->layout, NULL);
	}
	if (pipeline->handle) {
		vkDestroyPipeline(device->logical_dev, pipeline->handle, NULL);
	}
}
