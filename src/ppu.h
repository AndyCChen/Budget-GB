#pragma once

#include "emulatorConstants.h"
#include "utils/ppuArray.h"
#include "utils/vec.h"

#include <array>
#include <cstdint>
#include <vector>

class PPU
{
  public:
	static constexpr unsigned int VRAM_SIZE = 1024 * 8;

	void init(bool useBootrom);

	uint8_t r_LYC             = 0; // value to compare against the current scanline (lcdY)
	uint8_t r_scrollX         = 0;
	uint8_t r_scrollY         = 0;
	uint8_t r_windowX         = 0;
	uint8_t r_windowY         = 0;
	uint8_t r_oamStart        = 0;
	uint8_t r_bgPaletteData   = 0;
	uint8_t r_objPaletteData0 = 0;
	uint8_t r_objPaletteData1 = 0;

  private:
	static constexpr unsigned int MODE_2_DURATION      = 80;  // oam scan lasts 80 dots
	static constexpr unsigned int SCANLINE_DURATION    = 456; // each scanline always lasts 456 dots
	static constexpr unsigned int SPRITES_PER_SCANLINE = 10;  // each scanline can only output max 10 sprites

	struct BackgroundFetcher
	{
		uint16_t patternTileAddress = 0;
		uint8_t  patternTileLoLatch = 0;
		uint8_t  patternTileHiLatch = 0;
		uint8_t  tileIndex          = 0;
		uint8_t  fetchCounter       = 0;
		bool     fetchComplete      = false;

		void reset()
		{
			patternTileAddress = 0;
			patternTileLoLatch = 0;
			patternTileHiLatch = 0;
			tileIndex          = 0;
			fetchCounter       = 0;
			fetchComplete      = false;
		}
	};

	struct SpriteFetcher
	{
		bool spriteFetchPending = false;

		uint16_t patternTileAddress = 0;
		uint8_t  patternTileLoLatch = 0;

		Utils::PPUArray<uint8_t, SPRITES_PER_SCANLINE> fetchQueue; // sprites queued up for fetching once bg tile fetches are complete

		void reset()
		{
			spriteFetchPending = false;
			patternTileAddress = 0;
			patternTileLoLatch = 0;
			fetchQueue.clear();
		}
	};

	struct Sprite
	{
		uint8_t yPosition  = 0;
		uint8_t xPosition  = 0;
		uint8_t tileIndex  = 0;
		uint8_t attributes = 0;
	};

	struct OutputSprite
	{
		uint8_t xPosition            = 0;
		uint8_t patternTileLoShifter = 0;
		uint8_t patternTileHiShifter = 0;
		uint8_t attributes           = 0;
		uint8_t shiftCounter         = 0; // incremented every time tiles are shifted, value of 8 means empty

		bool isEmpty() const
		{
			return shiftCounter == 8;
		}
	};

	class BackgroundFifo
	{
	  public:
		uint8_t patternTileLoShifter = 0;
		uint8_t patternTileHiShifter = 0;
		uint8_t shiftCounter         = 0;

		void clockFifo()
		{
			patternTileLoShifter <<= 1;
			patternTileHiShifter <<= 1;
			++shiftCounter;
		}

		// considered emtpy once 8 pixels have been shifted out
		bool isEmpty() const
		{
			return shiftCounter == 8;
		}

		void reset()
		{
			patternTileLoShifter = 0;
			patternTileHiShifter = 0;
			shiftCounter         = 0;
		}
	};

	class SpriteFifo
	{
	  public:
		Utils::PPUArray<OutputSprite, SPRITES_PER_SCANLINE> m_outputSprites;

		void reset()
		{
			m_outputSprites.clear();
			m_outputSprites.fill({0xFF, 0xFF, 0xFF, 0xFF, 0});
		}

		// search through array of output sprites and shifts any sprites that are in range
		// return true when sprite is found for outputing to lcd display
		bool clockFifo(uint8_t fetcherX, uint8_t &spriteColorIndex, uint8_t &attributes);

		void clockFifo(uint8_t fetcherX);
	};

	class SpriteScanner
	{
	  public:
		Utils::PPUArray<uint8_t, SPRITES_PER_SCANLINE> secondaryOAM; // hold selected sprites from oam scan process

		void reset()
		{
			secondaryOAM.clear();
		}
	};

	struct OamDmaController
	{
		uint8_t byteCounter   = 0;
		bool    dmaInProgress = false;

		void reset()
		{
			byteCounter   = 0;
			dmaInProgress = false;
		}
	};

	enum LCD_CONTROLS
	{
		BG_WINDOW_ENABLE            = 1 << 0,
		OBJ_ENABLE                  = 1 << 1,
		OBJ_SIZE                    = 1 << 2, // 0: 8x8 sprites, 1: 8x16 sprites
		BG_TILEMAP                  = 1 << 3,
		BG_WINDOW_PATTERN_DATA_AREA = 1 << 4,
		WINDOW_ENABLE               = 1 << 5,
		WINDOW_TILEMAP              = 1 << 6,
		PPU_ENABLE                  = 1 << 7,
	};

	enum SPRITE_ATTRIBUTES
	{
		DMG_PALETTE = 1 << 4,
		X_FLIP      = 1 << 5,
		Y_FLIP      = 1 << 6,
		PRIORITY    = 1 << 7, // 1: bg and window color indices 1-3 are drawn over this sprite
	};

	enum LCD_STATS
	{
		PPU_MODE      = 0x03,
		LYC_EQUALS_LY = 0x04,
		MODE_0_SELECT = 0x08,
		MODE_1_SELECT = 0x10,
		MODE_2_SELECT = 0x20,
		LYC_SELECT    = 0x40,
	};

	enum class Mode
	{
		MODE_0 = 0, // H-blank
		MODE_1,     // V-blank
		MODE_2,     // oam scan
		MODE_3,     // pixel rendering
	};

	// B01: initial dummy fetch to fill shift registers, takes total of 6 dots
	// B01S: remaining fetches of mode 3
	// B = nametable fetches, aka the tile index is fetched
	// 0, 1 = lo and hi tiles are fetches with nametable index
	// s = where sprite fetches in inserted if needed

	enum class PixelRenderState
	{
		// first fetch fills shift registers
		B01_FETCH,

		// second fetch handles horizontal scrolling and any sprites that are partially off the left side of the screen
		FIRST_B01S,

		// render bg and sprite pixels for the rest of the scanline
		B01S,

		// set up first window tile
		W01,

		// continue rendering rest of window and sprite pixels
		W01S,
	};

	enum class SpriteFetchState
	{
		CYCLE_0,
		CYCLE_1,
		CYCLE_2,
		CYCLE_3,
		CYCLE_4,
		CYCLE_5,
	};

	BudgetGbConstants::LcdColorBuffer &lcdColorBuffer;

	// 0: mode 0 state
	// 1: mode 1 state
	// 2: mode 2 state
	// 3: LYC state
	uint8_t  m_statInterruptSources;
	bool     m_sharedInterruptLine = false; // interrupt is requested when this transitions from low to high
	uint8_t &m_interruptLine;               // interrupt line from the cpu

	std::array<uint8_t, VRAM_SIZE> m_vram;
	std::array<Sprite, 40>         m_oamRam;

	SpriteScanner     m_spriteScanner;
	SpriteFetcher     m_spriteFetcher;
	SpriteFifo        m_spriteFifo;
	BackgroundFetcher m_bgFetcher;
	BackgroundFifo    m_bgFifo;

	// bits
	// 0, 1: Holds ppu's current mode status
	// 2: Set when r_LYC == r_lcdY
	// 3: Mode 0 select for stat interrupt
	// 4: Mode 1 select for stat interrupt
	// 5: Mode 2 select for stat interrupt
	// 6: LYC == LY select for stat interrupt
	uint8_t r_lcdStatus = 0;
	uint8_t r_lcdY      = 0; // holds the current horizontal scanline

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

	uint16_t m_scanlineDotCounter = 0;

	Mode             m_ppuMode          = Mode::MODE_2;
	PixelRenderState m_pixelRenderState = PixelRenderState::B01_FETCH;
	SpriteFetchState m_spriteFetchState = SpriteFetchState::CYCLE_0;

	uint8_t m_pixelX = 0; // track the pixel that is the ppu is currently on, ranges (0 - 159 inclusive)

	bool   m_frameDone  = false;
	size_t m_dotCounter = 0;

	struct WindowRegisters
	{
		bool WxMatch = false;
		bool WyMatch = false;

		uint8_t TileX        = 0;
		uint8_t LineCounterY = 0; // track Y line of window, base 1

		void reset()
		{
			WxMatch      = false;
			WyMatch      = false;
			TileX        = 0;
			LineCounterY = 0;
		}

	} m_window;

	// tile fetches for background and window

	void fetchNametable();
	void fetchTileLo();
	void fetchTileHi();

	// check if a sprite is present on current fetcherX coordinate
	bool spritePresentCheck();

	// 6 cycle sprite fetch
	// returns status for is sprite fetch being requested
	bool processSpriteFetching();
	void pushPixelToLCD();

	// step through 6 cycle tile fetch sequence
	void bgFetchStep();

	// load tile into reseted bg fifo shift regsiters, reset bg fetch state
	void loadBgFifo()
	{
		m_bgFifo.reset();
		m_bgFifo.patternTileLoShifter = m_bgFetcher.patternTileLoLatch;
		m_bgFifo.patternTileHiShifter = m_bgFetcher.patternTileHiLatch;
		m_bgFetcher.reset();
	}
	
  public:
	PPU(BudgetGbConstants::LcdColorBuffer &lcdColorBuffer, uint8_t &interruptFlags);

	OamDmaController m_oamDmaController;

	// tick ppu for 4 dots (1 cpu m-cycle == 4 ppu dots)
	void tick();

	uint8_t readVram(uint16_t position);
	void    writeVram(uint16_t position, uint8_t data);

	void    oamStartWrite(uint8_t data);
	uint8_t oamStartRead() const;

	uint8_t readOam(uint16_t position);
	void    writeOam(uint16_t position, uint8_t data);
	void    writeOamDMA(uint16_t position, uint8_t data);

	// poll if ppu frame is done
	bool isFrameComplete()
	{
		bool status = m_frameDone;
		m_frameDone &= false;

		return status;
	}

	const std::array<uint8_t, VRAM_SIZE> &getVram() const
	{
		return m_vram;
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
		if (r_lcdControl & LCD_CONTROLS::PPU_ENABLE)
			return r_lcdStatus;
		else
			return r_lcdStatus & 0xFC; // bits 0,1 read as zero when ppu is disabled
	}

	void setLcdStatus(uint8_t in)
	{
		// bits 0-2 are read only
		r_lcdStatus = (in & 0xF8) | (r_lcdStatus & 0x7);
	}

	uint8_t getLcdControl() const
	{
		return r_lcdControl;
	}

	void setLcdControl(uint8_t data)
	{
		if (r_lcdControl & LCD_CONTROLS::PPU_ENABLE && (data & LCD_CONTROLS::PPU_ENABLE) == 0)
		{
			ppuDisable();
		}

		r_lcdControl = data;
	}

	void ppuDisable();
};
