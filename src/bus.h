#pragma once

#include <vector>
#include <cstdint>

class Bus
{
	friend class Sm83JsonTest;

public:
	Bus() : m_wram(1024 * 64) 
	{

	};

	void    clearWram();

	uint8_t cpuRead(uint16_t position);
	void    cpuWrite(uint16_t position, uint8_t data);

private:
	std::vector<uint8_t> m_wram;

};
