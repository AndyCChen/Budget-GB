#pragma once

#include "emulatorConstants.h"
#include "config.h"

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
void drawMainViewport(const BudgetGbConstants::LcdColorBuffer &pixelBuffer, RenderContext *renderContext, SDL_Window *window);
void setViewportPalette(RenderContext *renderContext, const BudgetGbConfig::Palette &palette);
void endFrame(SDL_Window *window, RenderContext *renderContext);

/**
 * @brief Frees the main window and ImGui resources.
 *
 * @param window
 * @param renderContext
 */
void freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext);
} // namespace RendererGB
