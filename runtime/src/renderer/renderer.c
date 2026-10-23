#include "renderer.h"

b8 renderer_init(RendererBackend* out_backend, Window* window) {
	renderer_backend_create(BACKEND_API_VULKAN, out_backend);
	return out_backend->init(window, &out_backend->back_state);
}

void renderer_shutdown(RendererBackend* backend) {
	backend->shutdown(&backend->back_state);
}

void renderer_draw_frame(Window* window, RendererBackend* backend) {
	backend->render_frame(window, &backend->back_state);
}
