#include "window.h"
#include "core/logger.h"
#include "core/memory.h"
#include "core/assert.h"
#include <GLFW/glfw3.h>

struct window {
	GLFWwindow* handle;
};

Window* win_init(u32 width, u32 height, char* title) {
	Window* out_window = mem_alloc(sizeof(struct window));

	if (!out_window || !glfwInit()) {
		return FALSE;
	}
	if (!glfwVulkanSupported()) {
		LOG_ERROR("Vulkan is not supported on this system");
		glfwTerminate();
		return NULL;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	out_window->handle = glfwCreateWindow(width, height, title, NULL, NULL);

	if (out_window->handle == NULL) {
		glfwTerminate();
		return NULL;
	}
	glfwSetWindowUserPointer(out_window->handle, out_window);
	return out_window;
}

void win_shutdown(Window* window) {
	ASSERT(window);

	glfwDestroyWindow(window->handle);
	glfwTerminate();
	mem_free(window, sizeof(struct window));
}

b8 win_should_close(const Window* window) {
	return glfwWindowShouldClose(window->handle);
}

void win_get_framebuffer_size(Window* window, i32* out_width, i32* out_height) {
	glfwGetFramebufferSize(window->handle, out_width, out_height);
}

void win_poll_events(void) {
	glfwPollEvents();
}

b8 win_create_vk_surface(Window* window, VkInstance instance, VkSurfaceKHR* out_surface) {
	return glfwCreateWindowSurface(instance, window->handle, NULL, out_surface);
}

const char** win_get_instance_ext(u32* count) {
	return glfwGetRequiredInstanceExtensions(count);
}
