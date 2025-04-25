#include "fmt/base.h"
#include <fstream>

#include "cartridge.h"

bool Cartridge::loadCartridgeFromPath(const std::string &path)
{
	std::ifstream romFile(path, std::ios::binary);
	if (!romFile.is_open())
	{
		fmt::println(stderr, "Failed to open rom at: {}", path);
		m_cartridgeLoaded = false;
		return false;
	}

	romFile.seekg(0, std::ios::end);
	std::size_t romSize = romFile.tellg();
	romFile.seekg(0, std::ios::beg);

	m_cartridgeRom.resize(romSize);
	romFile.read(reinterpret_cast<char *>(&m_cartridgeRom[0]), romSize);
	romFile.close();

	fmt::println("Rom loaded: {}", path);
	fmt::println("Cartridge rom size: {:d} bytes", romSize);

	m_cartridgeLoaded = true;
	return true;
}

uint8_t Cartridge::cartridgeRead(uint16_t position)
{
	return m_cartridgeRom[position];
}
