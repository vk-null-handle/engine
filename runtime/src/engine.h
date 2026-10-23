#pragma once
#include "core/window.h"
#include "defines.h"
#include "renderer/renderer.h"

typedef struct engine {
  Window *window;
  RendererBackend renderer_backend;
} Engine;

void engine_init(Engine* engine);
void engine_shutdown(Engine* engine);
void engine_run(Engine* engine);
