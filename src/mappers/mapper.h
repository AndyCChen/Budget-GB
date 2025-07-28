#pragma once

#include <array>
#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

namespace Mapper
{

// https://gbdev.io/pandocs/The_Cartridge_Header.html#0147--cartridge-type
enum MBC_TYPES
{
	NO_MBC            = 0x00,
	MBC_1             = 0x01,
	MBC_1_RAM         = 0x02,
	MBC_1_RAM_BATTERY = 0x03,
};

struct CartInfo
{
	MBC_TYPES MbcType       = MBC_TYPES::NO_MBC;
	uint32_t  RomSize       = 0;
	uint32_t  RamSize       = 0;
	bool      BatteryBacked = false;
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

	const CartInfo m_cartInfo{};
};

bool loadMapper(std::ifstream &romFile, std::unique_ptr<IMapper> &mapper, CartInfo &cartInfo, std::string &errorMsg);

} // namespace Mapper
