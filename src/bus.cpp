#include "bus.h"

void Bus::clearWram()
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
}

uint8_t Bus::cpuRead(uint16_t position)
{
	return m_wram[position];
}

void Bus::cpuWrite(uint16_t position, uint8_t data)
{
	m_wram[position] = data;
}
