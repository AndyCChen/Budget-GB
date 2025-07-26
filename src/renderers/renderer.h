#pragma once

#include "config.h"
#include "emulatorConstants.h"
#include "imgui.h"
#include "utils/vec.h"

#include "SDL3/SDL.h"

#include <cstdint>
#include <memory>
#include <vector>

//  Handles specific rendering operations such as for openGL (and in the future maybe metal api)
namespace RendererGB
{
// opaque handle to rendering context
struct RenderContext;
struct TextureRenderTarget; // a texture that can be used as a render target
struct TexturedQuad;        // A quad with a attached texture containing color indices

/**
 * @brief Initializes the main window with sdl3 and sets up ImGui context.
 *
 * @param window
 * @param renderContext
 */
bool initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale);
void freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext);

void newFrame();
void endFrame(SDL_Window *window, RenderContext *renderContext);

void mainViewportResize(RenderContext *renderContext, int x, int y, int width, int height);
void mainViewportSetRenderTarget(RenderContext *renderContext); // select the main window as the render target (on dx11 this is the backbuffer, on opengl this is just framebuffer 0)

// update the color pallete use for pixels globally
void setGlobalPalette(RenderContext *renderContext, const BudgetGbConfig::Palette &palette);

// TextureRenderTarget

void        textureRenderTargetCreate(RenderContext *renderContext, TextureRenderTarget *&renderTargetTexture, const Utils::Vec2<float> &size);
void        textureRenderTargetFree(TextureRenderTarget *&renderTargetTexture);
void        textureRenderTargetSet(RenderContext *renderContext, TextureRenderTarget *renderTargetTexture, const Utils::Vec2<float> &viewport); // set as current render target                                 // clears the render target
void        textureRenderTargetResize(RenderContext *renderContext, TextureRenderTarget *renderTargetTexture, const Utils::Vec2<float> &size);
ImTextureID textureRenderTargetGetTextureID(TextureRenderTarget *renderTargetTexture); // returns texture id of render target texture (i.e shaderResourceView for dx11 or a Opengl Gluint texture id)

// Textured Quad

void texturedQuadCreate(RenderContext *renderContext, TexturedQuad *&texturedQuad, const Utils::Vec2<float> &textureSize);
void texturedQuadFree(TexturedQuad *&texturedQuad);
void texturedQuadUpdateTexture(RenderContext *renderContext, TexturedQuad *texturedQuad, const uint8_t *const data, const std::size_t size); // Update attached texture containing new color indices, size must be the same as initial texture size
void texturedQuadDraw(RenderContext *renderContext, TexturedQuad *texturedQuad);

// unique pointers with custom deleters

struct TextureRenderTargetDeleter
{
	void operator()(RendererGB::TextureRenderTarget *textureRenderTarget) const
	{
		RendererGB::textureRenderTargetFree(textureRenderTarget);
	}
};

typedef std::unique_ptr<RendererGB::TextureRenderTarget, TextureRenderTargetDeleter> TextureRenderTargetUniquePtr;

struct TexturedQuadDeleter
{
	void operator()(RendererGB::TexturedQuad *texturedQuad) const
	{
		RendererGB::texturedQuadFree(texturedQuad);
	}
};

typedef std::unique_ptr<RendererGB::TexturedQuad, TexturedQuadDeleter> TexturedQuadUniquePtr;

} // namespace RendererGB