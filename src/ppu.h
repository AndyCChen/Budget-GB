#pragma once

#include "utils/vec.h"
#include <array>
#include <cstdint>
#include <vector>

class PPU
{
  public:
	static constexpr unsigned int VRAM_SIZE = 1024 * 8;

	std::array<std::array<uint8_t, 3>, 4> m_colorPallete =
		{{
			{224, 248, 208},
			{136, 192, 112},
			{52, 104, 86},
			{8, 24, 32},
		}};

	// Bits
	// 0: enable/disable BG and window
	// 1: enable/disable objects
	// 2: toggle between 8x8 or 8x16 sprites
	// 3: select 0x9800 or 0x9C00 to use for BG tile map
	// 4: control addressing mode for BG and window
	// 5: window enable
	// 6: select 0x9800 or 0x9C00 to use for window tile map
	// 7: enable/disable lcd (ppu)
	uint8_t r_lcdControl = 0;
	uint8_t r_LYC        = 0; // value to compare against the current scanline (lcdY)
	uint8_t r_scrollX    = 0;
	uint8_t r_scrollY    = 0;
	uint8_t r_windowX    = 0;
	uint8_t r_windowY    = 0;

	uint8_t r_bgPaletteData = 0;

	struct Fetcher
	{
		uint8_t  fetcherX             = 0;
		uint8_t  bgNametableTileIndex = 0;
		uint16_t bgPatternTileAddress = 0;
		uint8_t  bgPatternTileLoLatch = 0;
		uint8_t  bgPatternTileHiLatch = 0;
	};

	struct BgFifo
	{
		uint8_t patternTileLoShifter = 0;
		uint8_t patternTileHiShifter = 0;

		void shiftFifo()
		{
			patternTileLoShifter <<= 1;
			patternTileHiShifter <<= 1;
		}
	};

  private:
	static constexpr unsigned int MODE_2_DURATION   = 80;  // oam scan lasts 80 dots
	static constexpr unsigned int SCANLINE_DURATION = 456; // each scanline always lasts 456 dots

	uint8_t &m_interruptLine;

	std::array<uint8_t, VRAM_SIZE>    m_vram;
	std::vector<Utils::array_u8Vec4> &m_lcdPixelBuffer;

	uint8_t r_lcdY      = 0; // holds the current horizontal scanline
	uint8_t r_lcdStatus = 0;

	uint16_t m_scanlineDotCounter = 0;

	enum class Mode
	{
		MODE_0 = 0, // H-blank
		MODE_1,     // V-blank
		MODE_2,     // oam scan
		MODE_3,     // pixel rendering
	};

	enum class OamScanState
	{
		CYCLE_0 = 0,
		CYCLE_1,
	};

	// B01: initial dummy fetch to fill shift registers, takes total of 6 dots
	// B01S: remaining fetches of mode 3
	// B = nametable fetches, aka the tile index is fetched
	// 0, 1 = lo and hi tiles are fetches with nametable index
	// s = where sprite fetches in inserted if needed

	enum class PixelRenderState
	{
		// first fetch is discarded aka do nothing (6 cycles)

		DUMMY_FETCH_NAMETABLE_0,
		DUMMY_FETCH_NAMETABLE_1,
		DUMMY_FETCH_TILE_LO_0,
		DUMMY_FETCH_TILE_LO_1,
		DUMMY_FETCH_TILE_HI_0,
		DUMMY_FETCH_TILE_HI_1,

		// prefetch to fill shift registers

		PREFETCH_NAMETABLE_0,
		PREFETCH_NAMETABLE_1,
		PREFETCH_TILE_LO_0,
		PREFETCH_TILE_LO_1,
		PREFETCH_TILE_HI_0,
		PREFETCH_TILE_HI_1,

		// first tile to be rendered can be paused for r_scrollX % 8 cycles to shift out pixels that should be offscreen

		SHIFT_PIXELS_NAMETABLE_0,
		SHIFT_PIXELS_NAMETABLE_1,
		SHIFT_PIXELS_TILE_LO_0,
		SHIFT_PIXELS_TILE_LO_1,
		SHIFT_PIXELS_TILE_HI_0,
		SHIFT_PIXELS_TILE_HI_1,
		SHIFT_PIXELS_PUSH_FIFO_0,

		// render pixels for the rest of the scanline

		B01S_NAMETABLE_0,
		B01S_NAMETABLE_1,
		B01S_TILE_LO_0,
		B01S_TILE_LO_1,
		B01S_TILE_HI_0,
		B01S_TILE_HI_1,
		B01S_PUSH_FIFO_0,
		B01S_PUSH_FIFO_1,
	};

	Mode             m_ppuMode          = Mode::MODE_2;
	OamScanState     m_oamScanState     = OamScanState::CYCLE_0;
	PixelRenderState m_pixelRenderState = PixelRenderState::DUMMY_FETCH_NAMETABLE_0;

	Fetcher m_fetcher;
	BgFifo  m_bgFifo;

	void fetchNametable();

	// lo/hi tile fetches for background and window

	void fetchTileLo();
	void fetchTileHi();

  public:
	PPU(std::vector<Utils::array_u8Vec4> &lcdPixelBuffer, uint8_t &interruptFlags);

	// tick ppu for 4 dots (1 cpu m-cycle == 4 ppu dots)
	void tick();

	uint8_t readVram(uint16_t position)
	{
		if (m_ppuMode != Mode::MODE_3)
			return m_vram[position & 0x1FFF];
		else
			return 0xFF;
	}

	void writeVram(uint16_t position, uint8_t data)
	{
		//if (m_ppuMode != Mode::MODE_3)
			m_vram[position & 0x1FFF] = data;
	}

	/**
	 * @brief Should only be used by logger to read into vram for instruction logging.
	 * Does not respect read access restrictions from ppu rendering.
	 * @param position
	 * @return
	 */
	uint8_t loggerReadVram(uint16_t position)
	{
		return m_vram[position & 0x1FFF];
	}

	uint8_t getLcdY() const
	{
		return r_lcdY;
	}

	uint8_t getLcdStatus() const
	{
		return r_lcdStatus;
	}

	void setLcdStatus(uint8_t in)
	{
		// bits 0-2 are read only
		r_lcdStatus = in & 0xF8;
	}

	void reset()
	{
		std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));
	}

	void pushPixelToLCD();
};
