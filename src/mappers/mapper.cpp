#include "mapper.h"
#include "mappers/MBC1.h"
#include "mappers/MBC2.h"
#include "mappers/noMBC.h"

// Instantiates mapper device
bool Mapper::loadMapper(std::ifstream &romFile, std::unique_ptr<IMapper> &mapper, CartInfo &cartInfo, std::string &errorMsg)
{
	bool status = true;

	switch (cartInfo.MbcType)
	{

	case MBC_TYPES::NO_MBC:
		mapper = std::make_unique<NoMBC>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC_1:
	case MBC_TYPES::MBC_1_RAM:
		mapper = std::make_unique<MBC1>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC_1_RAM_BATTERY:
		cartInfo.BatteryBacked = true;
		mapper                 = std::make_unique<MBC1>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC_2:
		cartInfo.RamSize = MBC2::RAM_SIZE;
		mapper           = std::make_unique<MBC2>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC_2_BATTERY:
		cartInfo.BatteryBacked = true;
		cartInfo.RamSize       = MBC2::RAM_SIZE;
		mapper                 = std::make_unique<MBC2>(romFile, cartInfo);
		break;

	default:
		errorMsg = "Unrecognized MBC!";
		status   = false;
		break;
	}

	return status;
}

const std::string_view Mapper::getMapperString(MBC_TYPES mbcType)
{
	switch (mbcType)
	{
	case Mapper::NO_MBC:
		return "No MBC";

	case Mapper::MBC_1:
		return "MBC1";
	case Mapper::MBC_1_RAM:
		return "MBC1+Ram";
	case Mapper::MBC_1_RAM_BATTERY:
		return "MBC1+Ram+Battery";

	case Mapper::MBC_2:
		return "MBC2";
	case Mapper::MBC_2_BATTERY:
		return "MBC2+BATTERY";

	default:
		return "";
	}
}
