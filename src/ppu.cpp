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
		break;
	}
}
