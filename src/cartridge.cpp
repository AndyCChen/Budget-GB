#include "cartridge.h"
#include "fmt/base.h"

#include <algorithm>
#include <fstream>

bool Cartridge::loadCartridgeFromPath(const std::string &path, std::vector<std::string> &recentRoms)
{
	std::ifstream romFile(path, std::ios::binary);
	if (!romFile.is_open())
	{
		fmt::println(stderr, "Failed to open rom at: {}", path);
		m_cartridgeLoaded = false;
		return m_cartridgeLoaded;
	}

	romFile.seekg(0, std::ios::end);
	std::size_t romSize = romFile.tellg();
	romFile.seekg(0, std::ios::beg);

	m_cartridgeRom.resize(romSize);
	romFile.read(reinterpret_cast<char *>(&m_cartridgeRom[0]), romSize);
	romFile.close();

	fmt::println("Rom loaded: {}", path);
	fmt::println("Cartridge rom size: {:d} bytes", romSize);

	recentRoms.erase(std::remove(recentRoms.begin(), recentRoms.end(), path), recentRoms.end());

	if (recentRoms.size() == BudgetGbConfig::MAX_RECENT_ROMS)
		recentRoms.pop_back();

	recentRoms.insert(recentRoms.begin(), path);

	m_cartridgeLoaded = true;
	return m_cartridgeLoaded;
}

uint8_t Cartridge::cartridgeRead(uint16_t position)
{
	return m_cartridgeRom[position];
}
