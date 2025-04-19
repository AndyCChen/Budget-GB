#pragma once

#include "utils/vec.h"
#include <SDL3/SDL.h>

#include <cstdint>
#include <vector>

//  Handles specific rendering operations such as for openGL (and in the future maybe metal api)
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
bool initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale);

void newFrame();
void setMainViewportSize(RenderContext *renderContext, int x, int y, int width, int height);
void drawMainViewport(const std::vector<Utils::array_u8Vec4> &pixelBuffer, RenderContext *renderContext, SDL_Window *window);
void endFrame(SDL_Window *window, RenderContext *renderContext);

/**
 * @brief Frees the main window and ImGui resources.
 *
 * @param window
 * @param renderContext
 */
void freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext);
} // namespace RendererGB
