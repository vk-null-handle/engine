#pragma once
#include "defines.h"
#include "renderer/renderer_backend.h"

b8 vulkan_backend_init(Window* window, BackendState* back_state);
void vulkan_backend_shutdown(BackendState *back_state);
void vulkan_backend_render_frame(Window *window, BackendState *back_state);
