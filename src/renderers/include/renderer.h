#pragma once

#include "SDL3/SDL.h"

namespace RendererGB
{
void init(SDL_Window*& window);
void render(SDL_Window* window);
void free(SDL_Window*& window);
}