#pragma once

#include "SDL3/SDL.h"

//  Handles specific rendering operations such as for openGL and (in the future maybe) metal api
namespace RendererGB
{
// opaque pointer to rendering context
typedef struct RenderContext RenderContext;

/**
 * @brief Initializes the main window with sdl3 and sets up ImGui context.
 *
 * @param window 
 * @param renderContext 
 */
void init(SDL_Window *&window, RenderContext *&renderContext);

void newFrame();
void render(SDL_Window *window);

/**
 * @brief Frees the main window and ImGui resources.
 *
 * @param window 
 * @param renderContext 
 */
void free(SDL_Window *&window, RenderContext *&renderContext);
} // namespace RendererGB
