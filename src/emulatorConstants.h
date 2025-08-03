#pragma once

#include <array>
#include <cstdint>

namespace BudgetGbConstants
{
static constexpr uint32_t LCD_WIDTH    = 160;
static constexpr uint32_t LCD_HEIGHT   = 144;
static constexpr uint32_t CLOCK_RATE_T = 4194304; // gameboy clock frequency

static constexpr uint32_t AUDIO_SAMPLE_RATE = 48000;

static constexpr uint32_t TILE_VIEW_WIDTH  = 128; // pixel width of tile viewport
static constexpr uint32_t TILE_VIEW_HEIGHT = 192; // pixel height of tile viewport

static constexpr uint32_t TILE_WIDTH  = 8; // pixel width of a single tile
static constexpr uint32_t TILE_HEIGHT = 8; // pixel height of single tile

typedef std::array<uint8_t, LCD_WIDTH * LCD_HEIGHT>             LcdColorBuffer;
typedef std::array<uint8_t, TILE_VIEW_WIDTH * TILE_VIEW_HEIGHT> TileColorBuffer;
} // namespace BudgetGbConstants
