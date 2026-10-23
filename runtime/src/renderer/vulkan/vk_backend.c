#include "vk_backend.h"
#include "core/logger.h"
#include "core/memory.h"
#include <vulkan/vulkan_core.h>

typedef struct internal_state {
	u32 data;
} InternalState;

b8 vulkan_backend_init(Window* window, BackendState* back_state) {
	back_state->internal_state = mem_alloc(sizeof(InternalState));
	InternalState* state = (InternalState*)back_state->internal_state;

	LOG_DEBUG("Vulkan Initialized");
	return TRUE;
}

void vulkan_backend_shutdown(BackendState* back_state) {
	InternalState* state = (InternalState*)back_state->internal_state;
}

void vulkan_backend_render_frame(Window* window, BackendState* back_state) {
	InternalState* state = (InternalState*)back_state->internal_state;
}
