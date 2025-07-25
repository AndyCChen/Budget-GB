#pragma once

#include "emulatorConstants.h"
#include "ppu.h"
#include "renderer.h"
#include "utils/vec.h"

#include <array>
#include <cstdint>
#include <memory>

class PatternTileView
{
  public:
	PatternTileView(const PPU &, const RendererGB::RenderContext *renderContext);
	~PatternTileView();

	// returns false if gui is closed, else return true
	bool drawViewportGui(RendererGB::RenderContext *renderContext);

  private:
	// store window size to compare with new window size each frame to detect resizing
	// so we can resize the tile textures
	// returns true if window resized, else false
	bool updateWindowSize(float width, float height);

	void updateTilePixelBuffer();

	const PPU         &m_ppu;
	Utils::Vec2<float> m_tileTextureSize{};

	BudgetGbConstants::TileColorBuffer m_tilePixelBuffer{};

	std::unique_ptr<RendererGB::PatternTileViewport> m_patternTileViewport;

	uint8_t m_tileX = 0;
	uint8_t m_tileY = 0;
};