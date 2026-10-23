#pragma once
#include "defines.h"
#include "core/window.h"

typedef enum renderer_backend_api {
	BACKEND_API_VULKAN,
	BACKEND_API_D3D12,
} BackendAPI;

typedef struct backend_state {
  void* internal_state;
} BackendState;

typedef struct renderer_backend {
  BackendState back_state;
  b8 (*init)(Window* window, BackendState *state);
	void (*shutdown)(BackendState *state);
	void (*render_frame)(Window *window, BackendState *state);
} RendererBackend;

b8 renderer_backend_create(BackendAPI api, RendererBackend* out_backend);
void renderer_backend_destroy(RendererBackend* renderer_backend);
