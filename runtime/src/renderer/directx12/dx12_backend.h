#pragma once
#include "defines.h"
#include "core/window.h"
#include "renderer/renderer_backend.h"

b8 dx12_backend_init(Window* window, BackendState* back_state);
void dx12_backend_shutdown(BackendState* back_state);
void dx12_backend_render_frame(Window *window,BackendState* back_state);
