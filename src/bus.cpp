#include "bus.h"

using namespace BusConstants;

Bus::Bus(Cartridge *cartridge, BusMode mode)
{
	m_mode = mode;
	m_cartridge = cartridge;
	std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));

	switch (m_mode)
	{

	case Bus::BusMode::NONE:
		m_wram.resize(DEFAULT_WRAM_SIZE);
		break;

	case Bus::BusMode::SM83_TEST:
		m_wram.resize(TEST_MODE_WRAM_SIZE);
		break;
	}
}

void Bus::clearWram()
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
}

uint8_t Bus::cpuRead(uint16_t position)
{
	if (m_mode == BusMode::NONE)
	{
		if (position < CARTRIDGE_ROM_END)
		{
			return m_cartridge->cartridgeRead(position);
		}
		else if (position < VRAM_END)
		{
			return m_vram[position & 0x1FFF];
		}
		else if (position < EXTERNAL_RAM_END)
		{
			return 0;
		}
		else if (position < ECHO_RAM_END)
		{
			return m_wram[(position & 0xDFFF) & 0x1FFF];
		}
		else if (position < UNUSABLE_END)
		{
			return 0;
		}
		else if (position < IO_REGISTERS_END)
		{
			return 0;
		}
		else if (position < HRAM_END)
		{
			return 0;
		}
		// interrupt enable register at 0xFFFF
		else
		{
			return 0;
		}
	}
	else
	{
		return m_wram[position];
	}
}

void Bus::cpuWrite(uint16_t position, uint8_t data)
{
	if (m_mode == BusMode::NONE)
	{
		if (position < CARTRIDGE_ROM_END)
		{
		}
		else if (position < VRAM_END)
		{
			m_vram[position & 0x1FFF] = data;
		}
		else if (position < EXTERNAL_RAM_END)
		{
		}
		else if (position < ECHO_RAM_END)
		{
			m_wram[(position & 0xDFFF) & 0x1FFF] = data;
		}
		else if (position < UNUSABLE_END)
		{
		}
		else if (position < IO_REGISTERS_END)
		{
		}
		else if (position < HRAM_END)
		{
		}
		// interrupt enable register at 0xFFFF
		else
		{
		}
	}
	else
	{
		m_wram[position] = data;
	}
}
