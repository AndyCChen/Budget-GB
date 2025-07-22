#include "ppu.h"
#include "sm83.h"

PPU::PPU(BudgetGbConstants::LcdColorBuffer &lcdColorBuffer, uint8_t &interruptFlags)
	: lcdColorBuffer(lcdColorBuffer), m_interruptLine(interruptFlags)
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
				m_window.reset();
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

		// check if lcdY matches the windowY at the start of mode 2
		if (m_scanlineDotCounter == 1)
			m_window.WyMatch |= r_windowY == r_lcdY;

		if (m_scanlineDotCounter & 1)
		{
			if (m_spriteScanner.secondaryOAM.length() < m_spriteScanner.secondaryOAM.size())
			{
				uint8_t yPos = m_oamRam[m_scanlineDotCounter >> 1].yPosition;

				// 8 by 16 sprites
				if (r_lcdControl & LCD_CONTROLS::OBJ_SIZE)
				{
					if ((r_lcdY + 16 >= yPos) && (r_lcdY + 16 < yPos + 16))
						m_spriteScanner.secondaryOAM.push(static_cast<uint8_t>(m_scanlineDotCounter >> 1));
				}
				// 8 by 8 sprites
				else
				{
					if ((r_lcdY + 16 >= yPos) && (r_lcdY + 16 < yPos + 8))
						m_spriteScanner.secondaryOAM.push(static_cast<uint8_t>(m_scanlineDotCounter >> 1));
				}
			}
		}

		if (m_scanlineDotCounter == MODE_2_DURATION)
		{
			m_ppuMode          = Mode::MODE_3;
			m_pixelRenderState = PixelRenderState::B01_FETCH;
		}

		break;

	case Mode::MODE_3:

		r_lcdStatus            = (r_lcdStatus & ~LCD_STATS::PPU_MODE) | 3;
		m_statInterruptSources = m_statInterruptSources & ~0x7;

		switch (m_pixelRenderState)
		{
		// 6 cycle fetch to fill shift registers
		case PixelRenderState::B01_FETCH:

			bgFetchStep();

			if (m_bgFetcher.fetchComplete)
			{
				loadBgFifo();
				m_pixelRenderState = PixelRenderState::FIRST_B01S;
			}

			break;

		case PixelRenderState::FIRST_B01S:

			bgFetchStep();

			// sprite fetch pending needs to be handled
			if (spritePresentCheck())
			{
				if (m_bgFetcher.fetchComplete)
					processSpriteFetching();
			}
			// halt clocking fifos if a sprite fetch is pending
			else
			{
				m_window.WxMatch |= (r_windowX == m_pixelX) && m_window.WyMatch;

				m_spriteFifo.clockFifo(m_pixelX);

				if (m_bgFifo.shiftCounter >= (r_scrollX & 7))
					++m_pixelX;

				m_bgFifo.clockFifo();

				// restart fetcher to set up windows on next cycle
				if ((r_lcdControl & LCD_CONTROLS::WINDOW_ENABLE) && m_window.WxMatch)
				{
					m_bgFetcher.reset();
					m_window.LineCounterY += 1;
					m_pixelRenderState = PixelRenderState::W01;
				}
				else if (m_bgFifo.isEmpty())
				{
					loadBgFifo();
					m_pixelRenderState = PixelRenderState::B01S;
				}
			}

			break;

		case PixelRenderState::B01S:

			bgFetchStep();

			if (spritePresentCheck())
			{
				if (m_bgFetcher.fetchComplete)
					processSpriteFetching();
			}
			else
			{
				m_window.WxMatch |= (r_windowX == m_pixelX) && m_window.WyMatch;

				if (m_pixelX >= 8)
					pushPixelToLCD();
				else
				{
					m_bgFifo.clockFifo();
					m_spriteFifo.clockFifo(m_pixelX);
					++m_pixelX;
				}

				// restart fetcher to set up windows on next cycle
				if ((r_lcdControl & LCD_CONTROLS::WINDOW_ENABLE) && m_window.WxMatch)
				{
					m_bgFetcher.reset();
					m_window.LineCounterY += 1;
					m_pixelRenderState = PixelRenderState::W01;
				}
				else if (m_bgFifo.isEmpty())
				{
					loadBgFifo();
				}
			}

			break;

		case PixelRenderState::W01:

			bgFetchStep();

			if (m_bgFetcher.fetchComplete)
			{
				loadBgFifo();
				m_pixelRenderState = PixelRenderState::W01S;
			}

			break;

		case PixelRenderState::W01S:

			bgFetchStep();

			if (spritePresentCheck())
			{
				if (m_bgFetcher.fetchComplete)
					processSpriteFetching();
			}
			else
			{
				if (m_pixelX >= 8)
					pushPixelToLCD();
				else
				{
					m_bgFifo.clockFifo();
					m_spriteFifo.clockFifo(m_pixelX);
					++m_pixelX;
				}

				if (m_bgFifo.isEmpty())
				{
					loadBgFifo();
				}
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

void PPU::bgFetchStep()
{
	// cycles 0 to 5, does nothing once fetch completes and is restarted either manually or when loading the bg fifo
	if (m_bgFetcher.fetchCounter <= 5)
	{
		++m_bgFetcher.fetchCounter;

		switch (m_bgFetcher.fetchCounter)
		{
		case 1:
			fetchNametable();
			break;

		case 3:
			fetchTileLo();
			break;

		case 5:
			fetchTileHi();
			m_bgFetcher.fetchComplete = true;
			break;

		default:
			break;
		}
	}
}

void PPU::pushPixelToLCD()
{
	uint8_t outputColorIndex = 0;

	uint8_t bgColorIndex = (m_bgFifo.patternTileHiShifter & 0x80) | ((m_bgFifo.patternTileLoShifter & 0x80) >> 1);
	bool    isBgZero     = bgColorIndex;
	bgColorIndex >>= 6;
	bgColorIndex = (r_bgPaletteData >> (bgColorIndex * 2)) & 0x3;
	m_bgFifo.clockFifo();

	outputColorIndex = bgColorIndex;

	uint8_t spriteColorIndex = 0, attributes = 0;

	if (m_spriteFifo.clockFifo(m_pixelX, spriteColorIndex, attributes))
	{
		if (attributes & SPRITE_ATTRIBUTES::DMG_PALETTE)
			spriteColorIndex = (r_objPaletteData1 >> (spriteColorIndex * 2)) & 0x3;
		else
			spriteColorIndex = (r_objPaletteData0 >> (spriteColorIndex * 2)) & 0x3;

		if (attributes & SPRITE_ATTRIBUTES::PRIORITY)
		{
			if (isBgZero == 0)
				outputColorIndex = spriteColorIndex;
		}
		else
			outputColorIndex = spriteColorIndex;
	}

	uint16_t lcdPixelIndex = (160 * (143 - r_lcdY)) + (m_pixelX - 8);
	m_pixelX += 1;

	lcdColorBuffer[lcdPixelIndex] = outputColorIndex;

	// enter H-blank once 160 pixels are drawn
	if (m_pixelX == 160 + 8)
	{
		m_window.WxMatch = false;
		m_window.TileX   = 0;
		m_pixelX         = 0;
		m_ppuMode        = Mode::MODE_0;
	}
}

void PPU::fetchNametable()
{
	// 1001 1BYY YYYX XXXX
	// |||| |||| |||+-++++-       X: x coord of tile
	// |||| ||++-+++-------       Y: y coord of tile
	// |||| |+------------------- B: (0) fetch nametable from tile map 0x9800, (1) fetch nametable from tile map 0x9C00
	// ++++-+-------------- 1001 10: Tiles begin at adress 0x9800

	if ((r_lcdControl & LCD_CONTROLS::WINDOW_ENABLE) && m_window.WxMatch)
	{
		uint16_t baseAddress = 0x9800 | ((r_lcdControl & LCD_CONTROLS::WINDOW_TILEMAP) << 4);

		uint8_t tileY = ((m_window.LineCounterY - 1) & 0xF8) >> 3;

		uint16_t address = baseAddress | (tileY << 5) | m_window.TileX++;

		m_bgFetcher.tileIndex = m_vram[address & 0x1FFF];
	}
	else
	{
		uint16_t baseAddress = 0x9800 | ((r_lcdControl & LCD_CONTROLS::BG_TILEMAP) << 7);

		uint8_t tileX = ((r_scrollX + m_pixelX) & 0xF8) >> 3;
		uint8_t tileY = ((r_scrollY + r_lcdY) & 0xF8) >> 3;

		uint16_t address = baseAddress | (tileY << 5) | tileX;

		m_bgFetcher.tileIndex = m_vram[address & 0x1FFF];
	}
}

void PPU::fetchTileLo()
{
	uint8_t fineY = 0;

	if ((r_lcdControl & LCD_CONTROLS::WINDOW_ENABLE) && m_window.WxMatch)
		fineY = (m_window.LineCounterY - 1) & 0x7;
	else
		fineY = (r_scrollY + r_lcdY) & 0x7;

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
	if (m_spriteFetcher.spriteFetchPending)
		return m_spriteFetcher.spriteFetchPending;

	for (uint8_t i = 0; i < m_spriteScanner.secondaryOAM.length(); ++i)
	{
		uint8_t spriteIndex = m_spriteScanner.secondaryOAM[i];

		if (spriteIndex != 0xFF && (r_lcdControl & LCD_CONTROLS::OBJ_ENABLE) && (m_oamRam[spriteIndex].xPosition == m_pixelX))
		{
			m_spriteFetcher.spriteFetchPending = true;
			m_spriteFetcher.fetchQueue.push(spriteIndex);
			m_spriteScanner.secondaryOAM[i] = 0xFF; // mark as deleted
		}
	}

	if (m_spriteFetcher.spriteFetchPending)
		m_spriteScanner.secondaryOAM.remove([](uint8_t n) { return n == 0xFF; });

	return m_spriteFetcher.spriteFetchPending;
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
			uint8_t fineY = ((r_lcdY + 16) - m_oamRam[spriteIndex].yPosition) & 0x7;

			if (m_oamRam[spriteIndex].attributes & SPRITE_ATTRIBUTES::Y_FLIP)
			{
				fineY                              = 7 - fineY;
				m_spriteFetcher.patternTileAddress = 0x8000 | (tileIndex << 4) | (fineY << 1);
			}
			else
			{
				m_spriteFetcher.patternTileAddress = 0x8000 | (tileIndex << 4) | (fineY << 1);
			}
		}
		// 8 by 16 sprites
		else
		{
			uint8_t fineY = (r_lcdY + 16) - m_oamRam[spriteIndex].yPosition;

			if (m_oamRam[spriteIndex].attributes & SPRITE_ATTRIBUTES::Y_FLIP)
			{
				fineY = 15 - fineY;
				tileIndex += (fineY & 0x8) ? 1 : 0;
				m_spriteFetcher.patternTileAddress = 0x8000 | (tileIndex << 4) | ((fineY & 0x7) << 1);
			}
			else
			{
				tileIndex += (fineY & 0x8) ? 1 : 0;
				m_spriteFetcher.patternTileAddress = 0x8000 | (tileIndex << 4) | ((fineY & 0x7) << 1);
			}
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

		OutputSprite outSprite{};

		outSprite.xPosition            = m_oamRam[spriteIndex].xPosition;
		outSprite.attributes           = m_oamRam[spriteIndex].attributes;
		outSprite.patternTileLoShifter = m_spriteFetcher.patternTileLoLatch;
		outSprite.patternTileHiShifter = m_vram[++m_spriteFetcher.patternTileAddress & 0x1FFF];

		if (outSprite.attributes & SPRITE_ATTRIBUTES::X_FLIP)
		{
			outSprite.patternTileLoShifter = ((outSprite.patternTileLoShifter & 0xF0) >> 4) | ((outSprite.patternTileLoShifter & 0x0F) << 4);
			outSprite.patternTileLoShifter = ((outSprite.patternTileLoShifter & 0xCC) >> 2) | ((outSprite.patternTileLoShifter & 0x33) << 2);
			outSprite.patternTileLoShifter = ((outSprite.patternTileLoShifter & 0xAA) >> 1) | ((outSprite.patternTileLoShifter & 0x55) << 1);

			outSprite.patternTileHiShifter = ((outSprite.patternTileHiShifter & 0xF0) >> 4) | ((outSprite.patternTileHiShifter & 0x0F) << 4);
			outSprite.patternTileHiShifter = ((outSprite.patternTileHiShifter & 0xCC) >> 2) | ((outSprite.patternTileHiShifter & 0x33) << 2);
			outSprite.patternTileHiShifter = ((outSprite.patternTileHiShifter & 0xAA) >> 1) | ((outSprite.patternTileHiShifter & 0x55) << 1);
		}

		m_spriteFifo.m_outputSprites.push(outSprite);

		m_spriteFetcher.fetchQueue.pop();
		if (m_spriteFetcher.fetchQueue.isEmpty())
			m_spriteFetcher.spriteFetchPending = false;

		m_spriteFetchState = SpriteFetchState::CYCLE_0;
		break;
	}
	}

	return m_spriteFetcher.spriteFetchPending;
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
	m_pixelRenderState = PixelRenderState::B01_FETCH;

	m_pixelX = 0;
	m_window.reset();

	// std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));

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

	bool spriteFound       = false;
	bool removeEmptySprite = false;

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
			++m_outputSprites[i].shiftCounter;

			if (m_outputSprites[i].isEmpty())
			{
				m_outputSprites[i].xPosition = 0xFF;
				removeEmptySprite            = true;
			}
		}
	}

	if (removeEmptySprite)
		m_outputSprites.remove([](OutputSprite sprt) { return sprt.xPosition == 0xFF; });

	return spriteFound;
}

void PPU::SpriteFifo::clockFifo(uint8_t fetcherX)
{
	if (m_outputSprites.isEmpty())
		return;

	bool removeEmptySprite = false;

	for (uint8_t i = 0; i < m_outputSprites.length(); ++i)
	{
		if (fetcherX >= m_outputSprites[i].xPosition && fetcherX < m_outputSprites[i].xPosition + 8)
		{
			m_outputSprites[i].patternTileLoShifter <<= 1;
			m_outputSprites[i].patternTileHiShifter <<= 1;
			++m_outputSprites[i].shiftCounter;

			if (m_outputSprites[i].isEmpty())
			{
				m_outputSprites[i].xPosition = 0xFF;
				removeEmptySprite            = true;
			}
		}
	}

	if (removeEmptySprite)
		m_outputSprites.remove([](OutputSprite sprt) { return sprt.xPosition == 0xFF; });
}
