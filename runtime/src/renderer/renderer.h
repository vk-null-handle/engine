#pragma once
#include "renderer_backend.h"

b8 renderer_init(RendererBackend* out_backend, Window* window);
void renderer_shutdown(RendererBackend* backend);
void renderer_draw_frame(RendererBackend* backend, Window* window);
