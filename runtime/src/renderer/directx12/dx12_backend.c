#include "dx12_backend.h"
#include "core/memory.h"

typedef struct internal_state {
	u32 data;
} InternalState;

b8 dx12_backend_init(Window* window, BackendState* back_state) {
	back_state->internal_state = mem_alloc(sizeof(InternalState));
	InternalState* state = (InternalState*)back_state->internal_state;
	return TRUE;
}

void dx12_backend_shutdown(BackendState* back_state) {
	InternalState* state = (InternalState*)back_state->internal_state;
}

void dx12_backend_render_frame(Window* window, BackendState* back_state) {
	InternalState* state = (InternalState*)back_state->internal_state;
}
