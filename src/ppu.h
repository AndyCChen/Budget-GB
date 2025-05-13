#pragma once

#include <array>
#include <cstdint>

class PPU
{
  public:
	static constexpr unsigned int VRAM_SIZE = 1024 * 8;

	uint8_t m_lcdControl = 0;
	uint8_t m_LYC        = 0;
	uint8_t m_scrollX    = 0;
	uint8_t m_scrollY    = 0;
	uint8_t m_windowX    = 0;
	uint8_t m_windowY    = 0;

  private:
	static constexpr unsigned int MODE_2_DURATION = 80; // oam scan lasts 80 dots

	std::array<uint8_t, VRAM_SIZE> m_vram;

	uint8_t m_lcdY      = 0; // holds the current horizontal scanline
	uint8_t m_lcdStatus = 0;

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

	enum class PixelRenderState
	{
		
	};

	Mode         m_ppuMode      = Mode::MODE_2;
	OamScanState m_oamScanState = OamScanState::CYCLE_0;

  public:
	PPU();

	// tick ppu for 4 dots (1 cpu m-cycle == 4 ppu dots)
	void tick();

	uint8_t readVram(uint16_t position)
	{
		return m_vram[position & 0x1FFF];
	}

	void writeVram(uint16_t position, uint8_t data)
	{
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
		return m_lcdY;
	}

	uint8_t getLcdStatus() const
	{
		return m_lcdStatus;
	}

	void setLcdStatus(uint8_t in)
	{
		// bits 0-2 are read only
		m_lcdStatus = in & 0xF8;
	}

	void reset()
	{
		std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));
	}
};
