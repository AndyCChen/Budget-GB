#include "ppu.h"

PPU::PPU()
{
	reset();
}

void PPU::tick()
{
	++m_scanlineDotCounter;

	switch (m_ppuMode)
	{
	case Mode::MODE_0:
		break;

	case Mode::MODE_1:
		break;

	case Mode::MODE_2: // oam scan

		switch (m_oamScanState)
		{
		case OamScanState::CYCLE_0:
			m_oamScanState = OamScanState::CYCLE_1;
			break;

		case OamScanState::CYCLE_1:
			m_oamScanState = OamScanState::CYCLE_0;
			break;
		}

		if (m_scanlineDotCounter == MODE_2_DURATION)
			m_ppuMode = Mode::MODE_3;

		break;

	case Mode::MODE_3:

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
			m_bgFifo.patternTileLoShifter = m_fetcher.bgPatternTileLoLatch;
			m_bgFifo.patternTileHiShifter = m_fetcher.bgPatternTileHiLatch;
			m_pixelRenderState            = PixelRenderState::SHIFT_PIXELS_NAMETABLE_0;
			break;

			// ------- shift out any offscreen pixels on first leftmost tile ----------

		case PixelRenderState::SHIFT_PIXELS_NAMETABLE_0:
			if ((r_scrollX & 7) == 0)
			{
				m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_1;
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
				m_pixelRenderState = PixelRenderState::B01S_TILE_LO_0;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_LO_2;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_LO_2:
			if ((r_scrollX & 7) == 2)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_LO_1;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_LO_3;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_LO_3:
			if ((r_scrollX & 7) == 3)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_HI_0;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_HI_4;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_HI_4:
			if ((r_scrollX & 7) == 4)
			{
				m_pixelRenderState = PixelRenderState::B01S_TILE_HI_1;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_TILE_HI_5;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_TILE_HI_5:
			if ((r_scrollX & 7) == 5)
			{
				m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_0;
			}
			else
			{
				m_bgFifo.shiftFifo();
				m_pixelRenderState = PixelRenderState::SHIFT_PIXELS_PUSH_FIFO_6;
			}
			break;

		case PixelRenderState::SHIFT_PIXELS_PUSH_FIFO_6:
			if ((r_scrollX & 7) != 6)
			{
				m_bgFifo.shiftFifo();
			}
			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_1;
			break;

			// ---------------- continue rendering pixels for entire scanline ------------------

		case PixelRenderState::B01S_NAMETABLE_0:
			m_fetcher.fetcherX += 1;
			m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_1;
			break;
		case PixelRenderState::B01S_NAMETABLE_1:
			m_fetcher.fetcherX += 1;
			fetchNametable();
			m_pixelRenderState = PixelRenderState::B01S_TILE_LO_0;
			break;
		case PixelRenderState::B01S_TILE_LO_0:
			m_fetcher.fetcherX += 1;
			m_pixelRenderState = PixelRenderState::B01S_TILE_LO_1;
			break;
		case PixelRenderState::B01S_TILE_LO_1:
			m_fetcher.fetcherX += 1;
			fetchTileLo();
			m_pixelRenderState = PixelRenderState::B01S_TILE_HI_0;
			break;
		case PixelRenderState::B01S_TILE_HI_0:
			m_fetcher.fetcherX += 1;
			m_pixelRenderState = PixelRenderState::B01S_TILE_HI_1;
			break;
		case PixelRenderState::B01S_TILE_HI_1:
			m_fetcher.fetcherX += 1;
			fetchTileHi();
			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_0;
			break;
		case PixelRenderState::B01S_PUSH_FIFO_0:
			m_fetcher.fetcherX += 1;
			m_pixelRenderState = PixelRenderState::B01S_PUSH_FIFO_1;
			break;
		case PixelRenderState::B01S_PUSH_FIFO_1:
			m_fetcher.fetcherX += 1;
			m_pixelRenderState = PixelRenderState::B01S_NAMETABLE_0;
			break;
		}

		break;
	}
}

void PPU::fetchNametable()
{
	// 1001 10YY YYYX XXXX

	uint16_t baseAddress = 0x9800 | ((r_lcdControl & 0x08) << 7);

	uint8_t tileX = ((r_scrollX + m_fetcher.fetcherX) & 0xF8) >> 3;
	uint8_t tileY = ((r_scrollY + r_lcdY) & 0xF8) >> 3;

	uint16_t address = baseAddress | ((tileY << 5) | tileX);

	m_fetcher.bgNametableTileIndex = m_vram[address & 0x1FFF];
}

void PPU::fetchTileLo()
{
	uint8_t fineY = (r_scrollY + r_lcdY) & 0x7;

	// NNNN TTTT TTTT YYYP

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
