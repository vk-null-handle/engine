#pragma once
#include "renderer_types.h"
#include "core/window.h"

typedef enum renderer_backend_api {
	BACKEND_API_VULKAN,
	BACKEND_API_D3D12,
} BackendAPI;

typedef struct renderer_backend {
	b8 (*init)(Window* window);
	void (*shutdown)(void);
	void (*render_frame)(Window *window);
} RendererBackend;

b8 renderer_backend_create(BackendAPI api, RendererBackend* out_backend);
void renderer_backend_destroy(RendererBackend* renderer_backend);
