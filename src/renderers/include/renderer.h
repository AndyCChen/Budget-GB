#pragma once

#include "SDL3/SDL.h"

namespace RendererGB
{
// opaque pointer to rendering context
typedef struct RenderContext RenderContext;

void init(SDL_Window *&window, RenderContext *&renderContext);
void render(SDL_Window *window);
void free(SDL_Window *&window, RenderContext *&renderContext);
} // namespace RendererGB
