#pragma once
#include "defines.h"
#include "renderer/renderer_types.h"
#include <vulkan/vulkan_core.h>

#define MAX_FRAMES_IN_FLIGHT 2

// Per frame resources
typedef struct frame_resources {
	VkCommandBuffer command_buffer;
	VkSemaphore image_acquired;
} FrameResources;

typedef struct vulkan_sync {
  // Global GPU timeline
  VkSemaphore timeline;
  u64 next_signal_value;
  
  u64 frame_index;
  VkSemaphore *render_complete;
} VulkanSync;

typedef struct vulkan_pipeline {
  VkPipeline handle;
  VkPipelineLayout layout;

  VkShaderModule vert_shader;
  VkShaderModule comp_shader;
  VkShaderModule frag_shader;
} VulkanPipeline;

typedef struct vulkan_image {
  VkImage handle;
  VkDeviceMemory memory;
  VkImageView view;
  u32 width, height;
} VulkanImage;

typedef struct vulkan_swapchain {
  b8 require_recreate;
  u32 width, height;

  VkSurfaceFormatKHR image_format;
  VkSwapchainKHR handle;

  u32 n_images;
  VkImage *images;
  VkImageView *image_views;

  VulkanImage depth_image;
} VulkanSwapchain;

typedef struct vulkan_swapchain_support_info {
  VkSurfaceCapabilitiesKHR capabilities;
  u32 n_formats;
  VkSurfaceFormatKHR* formats;
  u32 n_present_modes;
  VkPresentModeKHR* present_modes;
} VulkanSwapchainSupportInfo;

typedef struct vulkan_device {
  VkDevice logical_dev;
  VkPhysicalDevice physical_dev;

  VkQueue gfx_queue;
  u32 gfx_queue_fam_idx;
  
  VkPhysicalDeviceProperties properties;
  VulkanSwapchainSupportInfo swapchain_info;
  
  VkFormat depth_format;
	VkCommandPool command_pool;
} VulkanDevice;

typedef struct vulkan_buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  VkDeviceSize size;
  void* mapped; 
} VulkanBuffer;

typedef struct vulkan_instance {
  VkInstance handle;
  const char** layers;
  const char** exts;
  u32 n_layers, n_ext;
} VulkanInstance;

typedef struct vulkan_context {
  VkSurfaceKHR surface;
  FrameResources frame_resources[MAX_FRAMES_IN_FLIGHT];

  VulkanSync sync;
  VulkanSwapchain swapchain;
  VulkanDevice device;
  VulkanInstance instance;
  VulkanPipeline pipeline;

  VulkanBuffer indirect_buffer;
  VulkanBuffer vertex_buffer;
} VulkanContext;
