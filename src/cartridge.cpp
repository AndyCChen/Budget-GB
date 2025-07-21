#include "cartridge.h"

#include "fmt/base.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>

using namespace Mapper;

/**
 * @brief File out cartinfo with cartridge header
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
	romFile.read(reinterpret_cast<char *>(&cartInfo.mbcType), 1);

	romFile.seekg(ROM_SIZE_POS, std::ios::beg);
	romFile.read(reinterpret_cast<char *>(&cartInfo.romSize), 1);

	romFile.seekg(RAM_SIZE_POS, std::ios::beg);
	romFile.read(reinterpret_cast<char *>(&cartInfo.ramSize), 1);

	// set rom size https://gbdev.io/pandocs/The_Cartridge_Header.html#0148--rom-size

	if (cartInfo.romSize > 0x08)
	{
		errorMsg = "Unrecognized rom size!";
		return false;
	}
	else
	{
		romFile.seekg(0, std::ios::end);
		auto fileSize = romFile.tellg();
		romFile.seekg(0, std::ios::beg);

		cartInfo.romSize = 0x8000 * (1 << cartInfo.romSize);

		if (cartInfo.romSize != fileSize)
		{
			errorMsg = "Cartridge header rom size and actual file rom size miss-match!";
			return false;
		}
	}

	// set ram size

	switch (cartInfo.ramSize)
	{
	case 0x00:
		cartInfo.ramSize = 0;
		break;

	case 0x02:
		cartInfo.ramSize = 1024 * 8;
		break;

	case 0x03:
		cartInfo.ramSize = 1024 * 32;
		break;

	case 0x04:
		cartInfo.ramSize = 1024 * 128;
		break;

	case 0x05:
		cartInfo.ramSize = 1024 * 64;
		break;

	default:
		errorMsg = "Unrecognized ram size!";
		return false;
	}

	cartInfo.batteryBacked = Mapper::isBatteryBacked(cartInfo.mbcType);

	return true;
}

bool Cartridge::loadCartridgeFromPath(const std::string &path, std::vector<std::string> &recentRoms)
{
	bool status = true;

	std::ifstream romFile(path, std::ios::binary);
	if (!romFile.is_open())
	{
		fmt::println(stderr, "Failed to open rom at: {}", path);
		status = false;
		goto EXIT;
	}

	Mapper::CartInfo cartInfo{};
	if (!readCartridgeHeader(romFile, cartInfo, m_errorMsg))
	{
		fmt::println("{}", m_errorMsg);
		status = false;
		goto EXIT;
	}

	if (!Mapper::loadMapper(romFile, m_mapper, cartInfo, m_errorMsg))
	{
		fmt::println("{}", m_errorMsg);
		status = false;
		goto EXIT;
	}

EXIT:
	m_cartridgeLoaded = status;

	recentRoms.erase(std::remove(recentRoms.begin(), recentRoms.end(), path), recentRoms.end());
	if (m_cartridgeLoaded)
	{
		if (recentRoms.size() == BudgetGbConfig::MAX_RECENT_ROMS)
			recentRoms.pop_back();

		recentRoms.insert(recentRoms.begin(), path);
	}

	return m_cartridgeLoaded;
}
