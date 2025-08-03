#pragma once

#include "emulatorConstants.h"
#include "ppu.h"
#include "renderer.h"
#include "utils/vec.h"

#include "fmt/core.h"
#include <array>
#include <cstdint>
#include <memory>
class PatternTileView
{
  public:
	PatternTileView(const PPU &, RendererGB::RenderContext *renderContext);
	~PatternTileView() = default;

	// returns false if gui is closed, else return true
	bool drawViewportGui(RendererGB::RenderContext *renderContext);

  private:
	// store window size to compare with new window size each frame to detect resizing
	// so we can resize the tile textures
	// returns true if window resized, else false
	bool updateWindowSize(float width, float height);

	// update the main pixel buffer
	void updateTileViewPixelBuffer();

	// update the small preview pixel buffer
	void updateTilePreviewPixelBuffer();

	static constexpr Utils::Vec2<float> MIN_TILE_VIEW_SIZE{64, 64};
	static constexpr Utils::Vec2<float> TILE_PREVIEW_SIZE{160, 160};

	const PPU         &m_ppu;
	Utils::Vec2<float> m_tileViewportSize{0, 0};

	BudgetGbConstants::TileColorBuffer m_tileViewPixelBuffer{};

	std::array<uint8_t, BudgetGbConstants::TILE_WIDTH * BudgetGbConstants::TILE_HEIGHT> m_tilePreviewPixelBuffer{};

	RendererGB::TextureRenderTargetUniquePtr m_tileViewRenderTarget;
	RendererGB::TexturedQuadUniquePtr        m_tileViewQuad;

	RendererGB::TextureRenderTargetUniquePtr m_tilePreviewRenderTarget;
	RendererGB::TexturedQuadUniquePtr        m_tilePreviewQuad;

	uint8_t m_tileX = 0;
	uint8_t m_tileY = 0;
};