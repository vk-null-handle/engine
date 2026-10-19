#include "renderer.h"

b8 renderer_init(RendererBackend* out_backend, Window* window) {
	renderer_backend_create(BACKEND_API_VULKAN, out_backend);
	return out_backend->init(window);
}

void renderer_shutdown(RendererBackend* backend) {
	backend->shutdown();
}

void renderer_draw_frame(RendererBackend* backend, Window* window) {
	backend->render_frame(window);
}
