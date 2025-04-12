#pragma once

#include "utils/vec.h"

#include <cstdint>
#include <vector>

//  Handles specific rendering operations such as for openGL and (in the future maybe) metal api
namespace RendererGB
{
// opaque handle to rendering context
typedef struct RenderContext RenderContext;

/**
 * @brief Initializes the main window with sdl3 and sets up ImGui context.
 *
 * @param window
 * @param renderContext
 */
void initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext);

void newFrame();
void drawMainViewport(std::vector<Utils::array_u8Vec3> &pixelBuffer, RenderContext *renderContext);
void endFrame(SDL_Window *window);

/**
 * @brief Frees the main window and ImGui resources.
 *
 * @param window
 * @param renderContext
 */
void freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext);
} // namespace RendererGB
