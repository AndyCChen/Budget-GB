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
				m_spriteScanner.reset();
				m_spriteFetcher.reset();
				m_spriteFifo.reset();

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
				m_spriteScanner.reset();
				m_spriteFetcher.reset();
				m_spriteFifo.reset();

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

			if (m_spriteScanner.secondaryOAM.length() < m_spriteScanner.secondaryOAM.size())
			{
				uint8_t yPos = m_oamRam[m_spriteScanner.oamScanIndex].yPosition;

				if ((r_lcdY + 16 >= yPos) && (r_lcdY + 16 < yPos + 8))
					m_spriteScanner.secondaryOAM.push(m_spriteScanner.oamScanIndex);
			}

			++m_spriteScanner.oamScanIndex;
			m_oamScanState = OamScanState::CYCLE_0;
			break;
		}

		if (m_scanlineDotCounter == MODE_2_DURATION)
		{
			m_spriteScanner.oamScanIndex = 0;
			m_ppuMode                    = Mode::MODE_3;
			m_oamScanState               = OamScanState::CYCLE_0;
			m_pixelRenderState           = PixelRenderState::DUMMY_FETCH_NAMETABLE_0;
		}

		break;

	case Mode::MODE_3:

		r_lcdStatus            = (r_lcdStatus & ~LCD_STATS::PPU_MODE) | 3;
		m_statInterruptSources = m_statInterruptSources & ~0x7;

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

			// ------------------ 8 cycle prefetch ---------------------------

		case PixelRenderState::PREFETCH_NAMETABLE_0:

			if (!spritePresentCheck())
				m_bgFetcher.fetcherX += 1;

			m_pixelRenderState = PixelRenderState::PREFETCH_NAMETABLE_1;
			break;

		case PixelRenderState::PREFETCH_NAMETABLE_1:

			if (!spritePresentCheck())
				m_bgFetcher.fetcherX += 1;

			fetchNametable();
			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_LO_0;
			break;

		case PixelRenderState::PREFETCH_TILE_LO_0:

			if (!spritePresentCheck())
				m_bgFetcher.fetcherX += 1;

			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_LO_1;
			break;

		case PixelRenderState::PREFETCH_TILE_LO_1:

			if (!spritePresentCheck())
				m_bgFetcher.fetcherX += 1;

			fetchTileLo();
			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_HI_0;
			break;

		case PixelRenderState::PREFETCH_TILE_HI_0:

			if (!spritePresentCheck())
				m_bgFetcher.fetcherX += 1;

			m_pixelRenderState = PixelRenderState::PREFETCH_TILE_HI_1;
			break;

		case PixelRenderState::PREFETCH_TILE_HI_1:

			fetchTileHi();
			if (!spritePresentCheck())
				m_bgFetcher.fetcherX += 1;
			else
				processSpriteFetching();

			m_pixelRenderState = PixelRenderState::PREFETCH_PUSH_FIFO;
			break;

		case PixelRenderState::PREFETCH_PUSH_FIFO:

			if (spritePresentCheck())
			{
				if (!processSpriteFetching())
					m_pixelRenderState = PixelRenderState::PREFETCH_EXIT_SPRITE_FETCH;
			}
			else
			{
				if ((m_bgFetcher.fetcherX++ & 0x7) == 7)
				{
					m_bgFifo.patternTileLoShifter = m_bgFetcher.patternTileLoLatch;
					m_bgFifo.patternTileHiShifter = m_bgFetcher.patternTileHiLatch;
					m_pixelRenderState            = PixelRenderState::SHIFT_PIXELS_NAMETABLE_0;
				}
			}

			break;

		case PixelRenderState::PREFETCH_EXIT_SPRITE_FETCH:

			if ((m_bgFetcher.fetcherX++ & 0x7) == 7)
			{
				m_bgFifo.patternTileLoShifter = m_bgFetcher.patternTileLoLatch;
				m_bgFifo.patternTileHiShifter = m_bgFetcher.patternTileHiLatch;
				m_pixelRenderState            = PixelRenderState::SHIFT_PIXELS_NAMETABLE_0;
			}
			else
				m_pixelRenderState = PixelRenderState::PREFETCH_PUSH_FIFO;

			break;

			// ------- shift out any offscreen pixels on first leftmost tile ----------

		case PixelRenderState::SHIFT_PIXELS_NAMETABLE_0:

			if ((r_scrollX & 7) == 0)
			{
				m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_0;
				goto EXIT_PIXEL_SHIFT;
			}
			else if ((r_scrollX & 7) == 1)
				m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_1;
			else
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_NAMETABLE_1;

			m_bgFifo.shiftFifo();
			break;

		case PixelRenderState::SHIFT_PIXELS_NAMETABLE_1:
			m_bgFifo.shiftFifo();
			fetchNametable();

			if ((r_scrollX & 7) == 2)
				m_pixelRenderState = PixelRenderState::B01S_TILE_LO_0;
			else
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_LO_0;

			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_LO_0:
			m_bgFifo.shiftFifo();

			if ((r_scrollX & 7) == 3)
				m_pixelRenderState = PixelRenderState::B01S_TILE_LO_1;
			else
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_LO_1;

			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_LO_1:
			m_bgFifo.shiftFifo();
			fetchTileLo();

			if ((r_scrollX & 7) == 4)
				m_pixelRenderState = PixelRenderState::B01S_TILE_HI_0;
			else
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_HI_0;

			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_HI_0:
			m_bgFifo.shiftFifo();

			if ((r_scrollX & 7) == 5)
				m_pixelRenderState = PixelRenderState::B01S_TILE_HI_1;
			else
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_HI_1;

			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_HI_1:
			m_bgFifo.shiftFifo();
			fetchTileHi();

			if ((r_scrollX & 7) == 6)
				m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO;
			else
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_PUSH_FIFO_0;

			break;

		case PixelRenderState::SHIFT_PIXELS_PUSH_FIFO_0:
			m_bgFifo.shiftFifo();
			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO;

			break;

			// ---------------- start rendering pixels for entire scanline ------------------

		EXIT_PIXEL_SHIFT:
		case PixelRenderState::B01S_NAMETABLE_0:

			if (!spritePresentCheck())
				pushPixelToLCD();

			m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_1;
			break;

		case PixelRenderState::B01S_NAMETABLE_1:

			if (!spritePresentCheck())
				pushPixelToLCD();

			fetchNametable();
			m_pixelRenderState = PixelRenderState::B01S_TILE_LO_0;
			break;

		case PixelRenderState::B01S_TILE_LO_0:

			if (!spritePresentCheck())
				pushPixelToLCD();

			m_pixelRenderState = PixelRenderState::B01S_TILE_LO_1;
			break;

		case PixelRenderState::B01S_TILE_LO_1:

			if (!spritePresentCheck())
				pushPixelToLCD();

			fetchTileLo();
			m_pixelRenderState = PixelRenderState::B01S_TILE_HI_0;
			break;

		case PixelRenderState::B01S_TILE_HI_0:

			if (!spritePresentCheck())
				pushPixelToLCD();

			m_pixelRenderState = PixelRenderState::B01S_TILE_HI_1;
			break;

		case PixelRenderState::B01S_TILE_HI_1:

			fetchTileHi();
			if (!spritePresentCheck())
				pushPixelToLCD();
			else
				processSpriteFetching();

			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO;
			break;

		case PixelRenderState::B01S_PUSH_FIFO:

			if (spritePresentCheck())
			{
				if (!processSpriteFetching())
					m_pixelRenderState = PixelRenderState::B01S_EXIT_SPRITE_FETCH;
			}
			else
			{
				if (((r_scrollX + m_bgFetcher.fetcherX) & 0x7) == 7)
				{
					pushPixelToLCD();
					m_bgFifo.patternTileLoShifter = m_bgFetcher.patternTileLoLatch;
					m_bgFifo.patternTileHiShifter = m_bgFetcher.patternTileHiLatch;
					m_pixelRenderState            = PixelRenderState::B01S_NAMETABLE_0;
				}
				else
					pushPixelToLCD();
			}

			break;

		case PixelRenderState::B01S_EXIT_SPRITE_FETCH:

			if (((r_scrollX + m_bgFetcher.fetcherX) & 0x7) == 7)
			{
				pushPixelToLCD();
				m_bgFifo.patternTileLoShifter = m_bgFetcher.patternTileLoLatch;
				m_bgFifo.patternTileHiShifter = m_bgFetcher.patternTileHiLatch;
				m_pixelRenderState            = PixelRenderState::B01S_NAMETABLE_0;
			}
			else
			{
				pushPixelToLCD();
				m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO;
			}

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
	uint8_t outputColorIndex = 0;

	uint8_t bgColorIndex = (m_bgFifo.patternTileHiShifter & 0x80) | ((m_bgFifo.patternTileLoShifter & 0x80) >> 1);
	bgColorIndex >>= 6;
	bgColorIndex = (r_bgPaletteData >> (bgColorIndex * 2)) & 0x3;
	m_bgFifo.shiftFifo();

	outputColorIndex = bgColorIndex;

	uint8_t spriteColorIndex = 0, attributes = 0;

	if (m_spriteFifo.clockFifo(m_bgFetcher.fetcherX, spriteColorIndex, attributes))
	{
		if (attributes & SPRITE_ATTRIBUTES::DMG_PALETTE)
			spriteColorIndex = (r_objPaletteData1 >> (spriteColorIndex * 2)) & 0x3;
		else
			spriteColorIndex = (r_objPaletteData0 >> (spriteColorIndex * 2)) & 0x3;

		if (attributes & SPRITE_ATTRIBUTES::PRIORITY)
		{
			if (bgColorIndex == 0)
				outputColorIndex = spriteColorIndex;
		}
		else
			outputColorIndex = spriteColorIndex;
	}

	uint16_t lcdPixelIndex = (160 * (143 - r_lcdY)) + (m_bgFetcher.fetcherX - 8);
	m_bgFetcher.fetcherX += 1;

	m_lcdPixelBuffer[lcdPixelIndex][0] = m_colorPallete[outputColorIndex][0]; // r
	m_lcdPixelBuffer[lcdPixelIndex][1] = m_colorPallete[outputColorIndex][1]; // g
	m_lcdPixelBuffer[lcdPixelIndex][2] = m_colorPallete[outputColorIndex][2]; // b
	m_lcdPixelBuffer[lcdPixelIndex][3] = 0xFF;                                // a

	// enter H-blank once 160 pixels are drawn
	if (m_bgFetcher.fetcherX == 160 + 8)
	{
		m_bgFetcher.fetcherX = 0;
		m_ppuMode            = Mode::MODE_0;
	}
}

void PPU::fetchNametable()
{
	// 1001 1BYY YYYX XXXX
	// |||| |||| |||+-++++-       X: x coord of tile
	// |||| ||++-+++-------       Y: y coord of tile
	// |||| |+------------------- B: (0) fetch nametable from tile map 0x9800, (1) fetch nametable from tile map 0x9C00
	// ++++-+-------------- 1001 10: Tiles begin at adress 0x9800

	uint16_t baseAddress = 0x9800 | ((r_lcdControl & LCD_CONTROLS::BG_TILEMAP) << 7);

	uint8_t tileX = ((r_scrollX + m_bgFetcher.fetcherX) & 0xF8) >> 3;
	uint8_t tileY = ((r_scrollY + r_lcdY) & 0xF8) >> 3;

	uint16_t address = baseAddress | (tileY << 5) | tileX;

	m_bgFetcher.tileIndex = m_vram[address & 0x1FFF];
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
	if (r_lcdControl & LCD_CONTROLS::BG_WINDOW_PATTERN_DATA_AREA)
	{
		m_bgFetcher.patternTileAddress = 0x8000 | (m_bgFetcher.tileIndex << 4) | (fineY << 1);
	}
	// 0x9000 as base address with signed offset
	else
	{
		m_bgFetcher.patternTileAddress = 0x9000 & ~((m_bgFetcher.tileIndex & 0x80) << 5);
		m_bgFetcher.patternTileAddress |= (m_bgFetcher.tileIndex << 4) | (fineY << 1);
	}

	m_bgFetcher.patternTileLoLatch = m_vram[m_bgFetcher.patternTileAddress & 0x1FFF];
}

void PPU::fetchTileHi()
{
	m_bgFetcher.patternTileAddress += 1;
	m_bgFetcher.patternTileHiLatch = m_vram[m_bgFetcher.patternTileAddress & 0x1FFF];
}

bool PPU::spritePresentCheck()
{
	// abort check if sprite fetch is already previously requested
	if (m_spriteFetcher.spriteFetchRequested)
		return m_spriteFetcher.spriteFetchRequested;

	for (uint8_t i = 0; (m_spriteScanner.outputSpriteCount < m_spriteScanner.secondaryOAM.length()) && (i < m_spriteScanner.secondaryOAM.length()); ++i)
	{
		uint8_t spriteIndex = m_spriteScanner.secondaryOAM[i];

		if ((r_lcdControl & LCD_CONTROLS::OBJ_ENABLE) && (m_oamRam[spriteIndex].xPosition == m_bgFetcher.fetcherX))
		{
			m_spriteFetcher.spriteFetchRequested = true;
			m_spriteFetcher.fetchQueue.push(spriteIndex);
			++m_spriteScanner.outputSpriteCount;
		}
	}

	return m_spriteFetcher.spriteFetchRequested;
}

bool PPU::processSpriteFetching()
{
	switch (m_spriteFetchState)
	{

	case PPU::SpriteFetchState::CYCLE_0:
		m_spriteFetchState = SpriteFetchState::CYCLE_1;
		break;

	case PPU::SpriteFetchState::CYCLE_1:
		m_spriteFetchState = SpriteFetchState::CYCLE_2;
		break;

	case PPU::SpriteFetchState::CYCLE_2:
		m_spriteFetchState = SpriteFetchState::CYCLE_3;
		break;

	case PPU::SpriteFetchState::CYCLE_3: // pattern tile lo fetch
	{
		uint8_t spriteIndex = m_spriteFetcher.fetchQueue.peak();
		uint8_t tileIndex   = m_oamRam[spriteIndex].tileIndex;

		// 8 by 8 sprites
		if ((r_lcdControl & LCD_CONTROLS::OBJ_SIZE) == 0)
		{
			uint8_t fineY                      = ((r_lcdY + 16) - m_oamRam[spriteIndex].yPosition) & 0x7;
			m_spriteFetcher.patternTileAddress = 0x8000 | (tileIndex << 4) | (fineY << 1);
		}
		// 8 by 16 sprites
		else
		{
			assert(0 && "8 by 16 sprites not implemented!");
		}

		m_spriteFetcher.patternTileLoLatch = m_vram[m_spriteFetcher.patternTileAddress & 0x1FFF];
		m_spriteFetchState                 = SpriteFetchState::CYCLE_4;
		break;
	}

	case PPU::SpriteFetchState::CYCLE_4:
		m_spriteFetchState = SpriteFetchState::CYCLE_5;
		break;

	case PPU::SpriteFetchState::CYCLE_5: // pattern tile hi fetch
	{
		uint8_t spriteIndex = m_spriteFetcher.fetchQueue.peak();
		// todo handle y flip with attribute
		// uint8_t attributes  = m_oamRam[spriteIndex].attributes;

		OutputSprite outSprite{};

		outSprite.xPosition            = m_oamRam[spriteIndex].xPosition;
		outSprite.attributes           = m_oamRam[spriteIndex].attributes;
		outSprite.patternTileLoShifter = m_spriteFetcher.patternTileLoLatch;
		outSprite.patternTileHiShifter = m_vram[++m_spriteFetcher.patternTileAddress & 0x1FFF];

		m_spriteFifo.m_outputSprites.push(outSprite);

		m_spriteFetcher.fetchQueue.pop();
		if (m_spriteFetcher.fetchQueue.isEmpty())
			m_spriteFetcher.spriteFetchRequested = false;

		m_spriteFetchState = SpriteFetchState::CYCLE_0;
		break;
	}
	}

	return m_spriteFetcher.spriteFetchRequested;
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
	r_objPaletteData0    = 0;
	r_objPaletteData1    = 0;
	r_lcdY               = 0;
	r_lcdStatus          = 0;
	m_scanlineDotCounter = 0;
	r_oamStart           = 0;

	m_ppuMode          = Mode::MODE_2;
	m_oamScanState     = OamScanState::CYCLE_0;
	m_pixelRenderState = PixelRenderState::DUMMY_FETCH_NAMETABLE_0;

	std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));

	m_statInterruptSources = 0;
	m_sharedInterruptLine  = 0;

	m_oamDmaController.reset();
	m_bgFetcher.reset();
	m_bgFifo.reset();
	m_spriteFifo.reset();
	m_spriteScanner.reset();
	m_spriteFetcher.reset();
}

bool PPU::SpriteFifo::clockFifo(uint8_t fetcherX, uint8_t &spriteColorIndex, uint8_t &attributes)
{
	if (m_outputSprites.isEmpty())
		return false;

	bool spriteFound = false;

	for (uint8_t i = 0; i < m_outputSprites.length(); ++i)
	{
		if (fetcherX >= m_outputSprites[i].xPosition && fetcherX < m_outputSprites[i].xPosition + 8)
		{
			if (!spriteFound)
			{
				attributes       = m_outputSprites[i].attributes;
				spriteColorIndex = (m_outputSprites[i].patternTileHiShifter & 0x80) | ((m_outputSprites[i].patternTileLoShifter & 0x80) >> 1);
				spriteColorIndex >>= 6;

				if (spriteColorIndex != 0)
					spriteFound = true;
			}

			m_outputSprites[i].patternTileLoShifter <<= 1;
			m_outputSprites[i].patternTileHiShifter <<= 1;
		}
	}

	return spriteFound;
}
