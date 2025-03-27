#include "fmt/base.h"
#include <fstream>
#include <iostream>

#include "cartridge.h"

bool Cartridge::loadRomFromPath(const std::string &path)
{
	std::ifstream rom(path, std::ios::binary);
	if (!rom.is_open())
	{
		std::cerr << "Failed to open rom!" << std::endl;
		return false;
	}

	rom.seekg(0, std::ios::end);
	std::size_t romSize = rom.tellg();
	rom.seekg(0, std::ios::beg);

	m_cartridgeRom.resize(romSize);
	rom.read(reinterpret_cast<char *>(&m_cartridgeRom[0]), romSize);

	fmt::println("Cartridge rom size: {:d} bytes", romSize);

	rom.close();
	return false;
}

uint8_t Cartridge::cartridgeRead(uint16_t position)
{
	return m_cartridgeRom[position];
}
