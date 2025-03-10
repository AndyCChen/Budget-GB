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

	uint8_t cpu_read(uint16_t position);
	void    cpu_write(uint16_t position, uint8_t data);

private:
	std::vector<uint8_t> m_wram;

};