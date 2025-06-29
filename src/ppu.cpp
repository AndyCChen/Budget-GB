#include "ppu.h"
#include "sm83.h"

PPU::PPU(std::vector<Utils::array_u8Vec4> &lcdPixelBuffer, uint8_t &interruptFlags)
	: m_lcdPixelBuffer(lcdPixelBuffer), m_interruptLine(interruptFlags)
{
}

void PPU::tick()
{
	if ((r_lcdControl & LCD_CONTROLS::PPU_ENABLE) == 0)
	{
		return;
	}

	++m_scanlineDotCounter;

	switch (m_ppuMode)
	{
	case Mode::MODE_0: // H-blank

		r_lcdStatus = (r_lcdStatus & ~LCD_STATS::PPU_MODE) | 0;
		m_statInterruptSources &= ~0x7;

		if (r_lcdStatus & LCD_STATS::MODE_0_SELECT)
		{
			m_statInterruptSources |= 0x1;
		}

		if (m_scanlineDotCounter == SCANLINE_DURATION)
		{
			m_scanlineDotCounter = 0;

			// switch to V-blank once 144 scanlines have been drawn
			if (++r_lcdY == 144)
			{
				m_interruptLine |= Sm83::InterruptFlags::InterruptFlags_VBLANK; // request regular vblank interrupt
				m_ppuMode = Mode::MODE_1;
			}
			else
			{
				m_ppuMode = Mode::MODE_2; // begin new rendering scanline starting with oam scan
			}
		}
		break;

	case Mode::MODE_1: //  V-blank

		r_lcdStatus = (r_lcdStatus & ~LCD_STATS::PPU_MODE) | 1;
		m_statInterruptSources &= ~0x7;

		if (r_lcdStatus & LCD_STATS::MODE_1_SELECT)
		{
			m_statInterruptSources |= 0x2;
		}

		if (m_scanlineDotCounter == SCANLINE_DURATION)
		{
			m_scanlineDotCounter = 0;

			// exit V-blank once bottom-most scanline is finished
			if (++r_lcdY == 154)
			{
				m_ppuMode = Mode::MODE_2;
				r_lcdY    = 0;
			}
		}
		break;

	case Mode::MODE_2: // oam scan

		r_lcdStatus = (r_lcdStatus & ~LCD_STATS::PPU_MODE) | 2;
		m_statInterruptSources &= ~0x7;

		if (r_lcdStatus & LCD_STATS::MODE_2_SELECT)
		{
			m_statInterruptSources |= 0x4;
		}

		switch (m_oamScanState)
		{
		case OamScanState::CYCLE_0:
			m_oamScanState = OamScanState::CYCLE_1;
			break;

		case OamScanState::CYCLE_1:

			if (m_oamRam[m_spriteScanner.oamScanIndex].yPosition == (r_lcdY + 16) && m_spriteScanner.secondaryOamIndex < m_spriteScanner.secondaryOAM.size())
			{
				m_spriteScanner.secondaryOAM[m_spriteScanner.secondaryOamIndex++] = m_spriteScanner.oamScanIndex;
			}

			++m_spriteScanner.oamScanIndex;

			m_oamScanState = OamScanState::CYCLE_0;
			break;
		}

		if (m_scanlineDotCounter == MODE_2_DURATION)
		{
			m_ppuMode          = Mode::MODE_3;
			m_pixelRenderState = PixelRenderState::DUMMY_FETCH_NAMETABLE_0;
		}

		break;

	case Mode::MODE_3:

		r_lcdStatus            = (r_lcdStatus & ~LCD_STATS::PPU_MODE) | 3;
		m_statInterruptSources = m_statInterruptSources & ~0x7;

	EXIT_PIXEL_SHIFT:
		switch (m_pixelRenderState)
		{

			// ----------------------- 6 cycle dummy fetch --------------------

		case PixelRenderState::DUMMY_FETCH_NAMETABLE_0:
			m_pixelRenderState = PixelRenderState::DUMMY_FETCH_NAMETABLE_1;
			break;

		case PixelRenderState::DUMMY_FETCH_NAMETABLE_1:
			m_pixelRenderState = PixelRenderState::DUMMY_FETCH_TILE_LO_0;
			break;

		case PixelRenderState::DUMMY_FETCH_TILE_LO_0:
			m_pixelRenderState = PixelRenderState::DUMMY_FETCH_TILE_LO_1;
			break;

		case PixelRenderState::DUMMY_FETCH_TILE_LO_1:
			m_pixelRenderState = PixelRenderState::DUMMY_FETCH_TILE_HI_0;
			break;

		case PixelRenderState::DUMMY_FETCH_TILE_HI_0:
			m_pixelRenderState = PixelRenderState::DUMMY_FETCH_TILE_HI_1;
			break;

		case PixelRenderState::DUMMY_FETCH_TILE_HI_1:
			m_pixelRenderState = PixelRenderState::PREFETCH_NAMETABLE_0;
			break;

			// ------------------ 6 cycle prefetch ---------------------------

		case PixelRenderState::PREFETCH_NAMETABLE_0:
			m_pixelRenderState = PixelRenderState::PREFETCH_NAMETABLE_1;
			break;

		case PixelRenderState::PREFETCH_NAMETABLE_1:
			fetchNametable();
			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_LO_0;
			break;

		case PixelRenderState::PREFETCH_TILE_LO_0:
			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_LO_1;
			break;

		case PixelRenderState::PREFETCH_TILE_LO_1:
			fetchTileLo();
			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_HI_0;
			break;

		case PixelRenderState::PREFETCH_TILE_HI_0:
			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_HI_1;
			break;

		case PixelRenderState::PREFETCH_TILE_HI_1:
			fetchTileHi();
			m_fetcher.fetcherX += 8; // plus eight pixels to advance fetcher to the next tile
			m_bgFifo.patternTileLoShifter = m_fetcher.bgPatternTileLoLatch;
			m_bgFifo.patternTileHiShifter = m_fetcher.bgPatternTileHiLatch;
			m_pixelRenderState            = PixelRenderState::SHIFT_PIXELS_NAMETABLE_0;
			break;

			// ------- shift out any offscreen pixels on first leftmost tile ----------

		case PixelRenderState::SHIFT_PIXELS_NAMETABLE_0:
			if ((r_scrollX & 7) == 0)
			{
				m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_0;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_NAMETABLE_1;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_NAMETABLE_1:
			if ((r_scrollX & 7) == 1)
			{
				m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_1;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				fetchNametable();
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_LO_0;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_LO_0:
			if ((r_scrollX & 7) == 2)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_LO_0;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_LO_1;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_LO_1:
			if ((r_scrollX & 7) == 3)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_LO_1;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				fetchTileLo();
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_HI_0;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_HI_0:
			if ((r_scrollX & 7) == 4)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_HI_0;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_HI_1;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_HI_1:
			if ((r_scrollX & 7) == 5)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_HI_1;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				fetchTileHi();
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_PUSH_FIFO_0;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_PUSH_FIFO_0:
			if ((r_scrollX & 7) == 6)
			{
				m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_0;
				goto EXIT_PIXEL_SHIFT;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_1;
			}
			break;

			// ---------------- start rendering pixels for entire scanline ------------------

		case PixelRenderState::B01S_NAMETABLE_0:
			pushPixelToLCD();
			m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_1;
			break;

		case PixelRenderState::B01S_NAMETABLE_1:
			pushPixelToLCD();
			fetchNametable();
			m_pixelRenderState = PixelRenderState::B01S_TILE_LO_0;
			break;

		case PixelRenderState::B01S_TILE_LO_0:
			pushPixelToLCD();
			m_pixelRenderState = PixelRenderState::B01S_TILE_LO_1;
			break;

		case PixelRenderState::B01S_TILE_LO_1:
			pushPixelToLCD();
			fetchTileLo();
			m_pixelRenderState = PixelRenderState::B01S_TILE_HI_0;
			break;

		case PixelRenderState::B01S_TILE_HI_0:
			pushPixelToLCD();
			m_pixelRenderState = PixelRenderState::B01S_TILE_HI_1;
			break;

		case PixelRenderState::B01S_TILE_HI_1:
			pushPixelToLCD();
			fetchTileHi();
			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_0;
			break;

		case PixelRenderState::B01S_PUSH_FIFO_0:
			pushPixelToLCD();
			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_1;
			break;

		case PixelRenderState::B01S_PUSH_FIFO_1:
			pushPixelToLCD();
			m_bgFifo.patternTileLoShifter = m_fetcher.bgPatternTileLoLatch;
			m_bgFifo.patternTileHiShifter = m_fetcher.bgPatternTileHiLatch;
			m_pixelRenderState            = PixelRenderState::B01S_NAMETABLE_0;
			break;
		}

		break;
	}

	// update bit 2 of stat register based on LYC == LY condition
	if (r_lcdY != r_LYC)
	{
		r_lcdStatus &= ~LCD_STATS::LYC_EQUALS_LY;
		m_statInterruptSources &= ~0x8;
	}
	else
	{
		r_lcdStatus |= LCD_STATS::LYC_EQUALS_LY;

		if (r_lcdStatus & LCD_STATS::LYC_SELECT)
			m_statInterruptSources |= 0x8;
	}

	if (m_statInterruptSources == 0)
		m_sharedInterruptLine = false;
	else
	{
		// request stat interrupt on lo to hi transition
		if (!m_sharedInterruptLine)
			m_interruptLine |= Sm83::InterruptFlags::InterruptFlags_LCD;

		m_sharedInterruptLine = true;
	}
}

uint8_t PPU::readVram(uint16_t position)
{
	if (m_ppuMode != Mode::MODE_3 || (r_lcdControl & 0x80) == 0)
		return m_vram[position & 0x1FFF];
	else
		return 0xFF;
}

void PPU::writeVram(uint16_t position, uint8_t data)
{
	if (m_ppuMode != Mode::MODE_3 || (r_lcdControl & 0x80) == 0)
		m_vram[position & 0x1FFF] = data;
}

void PPU::oamStartWrite(uint8_t data)
{
	m_oamDmaController.dmaInProgress = true;
	r_oamStart                       = data % 0xE0;
}

uint8_t PPU::oamStartRead() const
{
	return r_oamStart;
}

uint8_t PPU::readOam(uint16_t position)
{
	if (m_ppuMode == Mode::MODE_0 || m_ppuMode == Mode::MODE_1)
	{
		uint8_t spriteIndex = static_cast<uint8_t>(((position) >> 2) & 0x3F);
		switch (position & 0x3)
		{
		case 0:
			return m_oamRam[spriteIndex].yPosition;

		case 1:
			return m_oamRam[spriteIndex].xPosition;

		case 2:
			return m_oamRam[spriteIndex].tileIndex;

		case 3:
			return m_oamRam[spriteIndex].attributes;
		default:
			return 0xFF;
		}
	}
	else
		return 0xFF;
}

void PPU::writeOam(uint16_t position, uint8_t data)
{
	if (m_ppuMode == Mode::MODE_0 || m_ppuMode == Mode::MODE_1)
	{
		uint8_t spriteIndex = static_cast<uint8_t>(((position) >> 2) & 0x3F);
		switch (position & 0x3)
		{
		case 0:
			m_oamRam[spriteIndex].yPosition = data;
			break;

		case 1:
			m_oamRam[spriteIndex].xPosition = data;
			break;

		case 2:
			m_oamRam[spriteIndex].tileIndex = data;
			break;

		case 3:
			m_oamRam[spriteIndex].attributes = data;
			break;
		}
	}
}

void PPU::writeOamDMA(uint16_t position, uint8_t data)
{
	uint8_t spriteIndex = static_cast<uint8_t>(((position) >> 2) & 0x3F);
	switch (position & 0x3)
	{
	case 0:
		m_oamRam[spriteIndex].yPosition = data;
		break;

	case 1:
		m_oamRam[spriteIndex].xPosition = data;
		break;

	case 2:
		m_oamRam[spriteIndex].tileIndex = data;
		break;

	case 3:
		m_oamRam[spriteIndex].attributes = data;
		break;
	}
}

void PPU::pushPixelToLCD()
{
	uint8_t colorIndex = (m_bgFifo.patternTileHiShifter & 0x80) | ((m_bgFifo.patternTileLoShifter & 0x80) >> 1);
	colorIndex >>= 6;
	m_bgFifo.shiftFifo();

	colorIndex = (r_bgPaletteData >> (colorIndex * 2)) & 0x3;

	uint16_t lcdPixelIndex = (160 * (143 - r_lcdY)) + (m_fetcher.fetcherX - 8);
	m_fetcher.fetcherX += 1;

	m_lcdPixelBuffer[lcdPixelIndex][0] = m_colorPallete[colorIndex][0]; // r
	m_lcdPixelBuffer[lcdPixelIndex][1] = m_colorPallete[colorIndex][1]; // g
	m_lcdPixelBuffer[lcdPixelIndex][2] = m_colorPallete[colorIndex][2]; // b
	m_lcdPixelBuffer[lcdPixelIndex][3] = 0xFF;                          // a

	// enter H-blank once 160 pixels are drawn
	if (m_fetcher.fetcherX == 160 + 8)
	{
		m_spriteScanner.reset();
		m_fetcher.fetcherX = 0;
		m_ppuMode          = Mode::MODE_0;
	}
}

void PPU::fetchNametable()
{
	// 1001 1BYY YYYX XXXX
	// |||| |||| |||+-++++-       X: x coord of tile
	// |||| ||++-+++-------       Y: y coord of tile
	// |||| |+------------------- B: (0) fetch nametable from tile map 0x9800, (1) fetch nametable from tile map 0x9C00
	// ++++-+-------------- 1001 10: Tiles begin at adress 0x9800

	uint16_t baseAddress = 0x9800 | ((r_lcdControl & 0x08) << 7);

	uint8_t tileX = ((r_scrollX + m_fetcher.fetcherX) & 0xF8) >> 3;
	uint8_t tileY = ((r_scrollY + r_lcdY) & 0xF8) >> 3;

	uint16_t address = baseAddress | (tileY << 5) | tileX;

	m_fetcher.bgNametableTileIndex = m_vram[address & 0x1FFF];
}

void PPU::fetchTileLo()
{
	uint8_t fineY = (r_scrollY + r_lcdY) & 0x7;

	// 100A TTTT TTTT YYYP
	//    | |||| |||| |||+- P: Bit plane selection
	//    | |||| |||| +++-- Y: Fine y offset (row within a 8 pixel high tile)
	//    | ++++-++++------ T: Tile index from nametable
	//    +---------------- A: Addressing mode selection, 0: use 0x9000 as base address, 1: use 0x8000 as base address

	// 0x8000 as base address with unsigned offset
	if (r_lcdControl & 0x10)
	{
		m_fetcher.bgPatternTileAddress = 0x8000 | (m_fetcher.bgNametableTileIndex << 4) | (fineY << 1);
	}
	// 0x9000 as base address with signed offset
	else
	{
		m_fetcher.bgPatternTileAddress = 0x9000 & ~((m_fetcher.bgNametableTileIndex & 0x80) << 5);
		m_fetcher.bgPatternTileAddress |= (m_fetcher.bgNametableTileIndex << 4) | (fineY << 1);
	}

	m_fetcher.bgPatternTileLoLatch = m_vram[m_fetcher.bgPatternTileAddress & 0x1FFF];
}

void PPU::fetchTileHi()
{
	m_fetcher.bgPatternTileAddress += 1;
	m_fetcher.bgPatternTileHiLatch = m_vram[m_fetcher.bgPatternTileAddress & 0x1FFF];
}

void PPU::init(bool useBootrom)
{
	r_lcdControl         = useBootrom ? 0x00 : 0x91;
	r_LYC                = 0;
	r_scrollX            = 0;
	r_scrollY            = 0;
	r_windowX            = 0;
	r_windowY            = 0;
	r_bgPaletteData      = 0;
	r_lcdY               = 0;
	r_lcdStatus          = 0;
	m_scanlineDotCounter = 0;
	r_oamStart           = 0;

	std::memset(&m_oamDmaController, 0, sizeof(OamDmaController));

	m_ppuMode          = Mode::MODE_2;
	m_oamScanState     = OamScanState::CYCLE_0;
	m_pixelRenderState = PixelRenderState::DUMMY_FETCH_NAMETABLE_0;

	std::memset(&m_fetcher, 0, sizeof(Fetcher));
	m_bgFifo.reset();

	std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));

	m_statInterruptSources = 0;
	m_sharedInterruptLine  = 0;

	m_spriteScanner.reset();
}
