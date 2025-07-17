#pragma once

#include <cstdint>
#include <fstream>
#include <memory>
#include <string>

namespace Mapper
{

// https://gbdev.io/pandocs/The_Cartridge_Header.html#0147--cartridge-type
enum MBC_TYPES
{
	NO_MBC = 0x00,
	MBC_1  = 0x01,
};

struct CartInfo
{
	MBC_TYPES mbcType;
	uint32_t  romSize;
	uint32_t  ramSize;
	bool      batteryBacked;
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

bool loadMapper(std::ifstream &romFile, std::unique_ptr<IMapper> &mapper, const CartInfo &cartInfo, std::string &errorMsg);

// check if given mbc type has battery backed ram
bool isBatteryBacked(Mapper::MBC_TYPES mbcType);

} // namespace Mapper
