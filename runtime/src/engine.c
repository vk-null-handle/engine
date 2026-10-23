#include "engine.h"
#include "game.h"

void engine_init(Engine* engine) {
	engine->window = win_init(1920, 1080, "Engine");
	game_init();
	renderer_init(&engine->renderer_backend, engine->window);
}

void engine_run(Engine* engine) {
	while (!win_should_close(engine->window)) {
		win_poll_events();
		game_tick();
		renderer_draw_frame(engine->window, &engine->renderer_backend);
	}
}

void engine_shutdown(Engine* engine) {
	renderer_shutdown(&engine->renderer_backend);
	win_shutdown(engine->window);
	game_shutdown();
}
