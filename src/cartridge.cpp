#include "cartridge.h"

#include "fmt/base.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>

using namespace Mapper;

/**
 * @brief Fill out cartinfo with cartridge header, cartInfo need to be passed into loadMapper for the full mapper info to be determined.
 * @param romFile File object that is already opened, will not be closed once this function returns
 * @param cartInfo Cart info that will be filled out
 * @param errorMsg contains error msg if function return false
 * @return true success, false fail
 */
static bool readCartridgeHeader(std::ifstream &romFile, Mapper::CartInfo &cartInfo, std::string errorMsg)
{
	constexpr int CARTRIDGE_TYPE_POS = 0x0147;
	constexpr int ROM_SIZE_POS       = 0x0148;
	constexpr int RAM_SIZE_POS       = 0x0149;

	// set mbc type

	romFile.seekg(CARTRIDGE_TYPE_POS, std::ios::beg);
	romFile.read(reinterpret_cast<char *>(&cartInfo.MbcType), 1);

	romFile.seekg(ROM_SIZE_POS, std::ios::beg);
	romFile.read(reinterpret_cast<char *>(&cartInfo.RomSize), 1);

	romFile.seekg(RAM_SIZE_POS, std::ios::beg);
	romFile.read(reinterpret_cast<char *>(&cartInfo.RamSize), 1);

	// set rom size https://gbdev.io/pandocs/The_Cartridge_Header.html#0148--rom-size

	// rom size values above 0x08 are not well documented and possibly incorrect, they are not used by official games anyways
	if (cartInfo.RomSize > 0x08)
	{
		errorMsg = "Unrecognized rom size!";
		return false;
	}
	else
	{
		romFile.seekg(0, std::ios::end);
		auto fileSize = romFile.tellg();
		romFile.seekg(0, std::ios::beg);

		cartInfo.RomSize = 0x8000 * (1 << cartInfo.RomSize);

		if (cartInfo.RomSize != fileSize)
		{
			errorMsg = "Cartridge header rom size and actual file rom size miss-match!";
			return false;
		}
	}

	// set ram size

	switch (cartInfo.RamSize)
	{
	case 0x00:
		cartInfo.RamSize = 0;
		break;

	case 0x02:
		cartInfo.RamSize = 1024 * 8;
		break;

	case 0x03:
		cartInfo.RamSize = 1024 * 32;
		break;

	case 0x04:
		cartInfo.RamSize = 1024 * 128;
		break;

	case 0x05:
		cartInfo.RamSize = 1024 * 64;
		break;

	default:
		errorMsg = "Unrecognized ram size!";
		return false;
	}

	return true;
}

bool Cartridge::loadCartridgeFromPath(const std::string &path, std::vector<std::string> &recentRoms)
{
	std::ifstream romFile(path, std::ios::binary);
	recentRoms.erase(std::remove(recentRoms.begin(), recentRoms.end(), path), recentRoms.end());

	if (!romFile.is_open())
	{
		fmt::println(stderr, "Failed to open rom at: {}", path);
		return m_cartridgeLoaded = false;
	}

	Mapper::CartInfo cartInfo{};
	cartInfo.CartFilePath = path;
	if (!readCartridgeHeader(romFile, cartInfo, m_errorMsg))
	{
		fmt::println("{}", m_errorMsg);
		return m_cartridgeLoaded = false;
	}

	if (!Mapper::loadMapper(romFile, m_mapper, cartInfo, m_errorMsg))
	{
		fmt::println("{}", m_errorMsg);
		return m_cartridgeLoaded = false;
	}

	if (recentRoms.size() == BudgetGbConfig::MAX_RECENT_ROMS)
		recentRoms.pop_back();

	recentRoms.insert(recentRoms.begin(), path);

	return m_cartridgeLoaded = true;
}
