#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace Mapper
{

// https://gbdev.io/pandocs/The_Cartridge_Header.html#0147--cartridge-type
enum class MBC_TYPES
{
	NO_MBC = 0x00,

	MBC1             = 0x01,
	MBC1_RAM         = 0x02,
	MBC1_RAM_BATTERY = 0x03,

	MBC2         = 0x05,
	MBC2_BATTERY = 0x06,

	MBC3_TIMER_BATTERY     = 0x0F,
	MBC3_TIMER_RAM_BATERRY = 0x10,

	MBC3             = 0x11,
	MBC3_RAM         = 0x12,
	MBC3_RAM_BATTERY = 0x13,
};

struct CartInfo
{
	MBC_TYPES             MbcType       = MBC_TYPES::NO_MBC;
	uint32_t              RomSize       = 0;
	uint32_t              RamSize       = 0;
	bool                  BatteryBacked = false;
	std::filesystem::path CartFilePath;
};

class IMapper
{
  public:
	IMapper(const CartInfo &cartInfo)
		: m_cartInfo(cartInfo)
	{
	}

	virtual ~IMapper() = default;

	virtual uint8_t read(uint16_t position)                = 0;
	virtual void    write(uint16_t position, uint8_t data) = 0;
	virtual void    reset()                                = 0;

	void dumpBatteryBackedRam(const std::vector<uint8_t> &ram) const;
	void dumpBatteryBackedRam(const uint8_t *ram, std::size_t size) const;

	void loadSaveRam( std::vector<uint8_t> &ram);
	void loadSaveRam(uint8_t *ram, std::size_t size);

	const CartInfo m_cartInfo{};
};

bool loadMapper(std::ifstream &romFile, std::unique_ptr<IMapper> &mapper, CartInfo &cartInfo, std::string &errorMsg);

const std::string_view getMapperString(MBC_TYPES mbcType);

} // namespace Mapper
