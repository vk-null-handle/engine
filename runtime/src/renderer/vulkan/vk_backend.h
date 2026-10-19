#pragma once
#include "defines.h"
#include "core/window.h"

b8 vulkan_backend_init(Window *window);
void vulkan_backend_shutdown(void);
void vulkan_backend_render_frame(Window* window);
