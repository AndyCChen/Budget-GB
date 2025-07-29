#include "mapper.h"
#include "mappers/MBC1.h"
#include "mappers/MBC2.h"
#include "mappers/MBC3.h"
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

	case MBC_TYPES::MBC1:
	case MBC_TYPES::MBC1_RAM:
		mapper = std::make_unique<MBC1>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC1_RAM_BATTERY:
		cartInfo.BatteryBacked = true;
		mapper                 = std::make_unique<MBC1>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC2:
		cartInfo.RamSize = MBC2::RAM_SIZE;
		mapper           = std::make_unique<MBC2>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC2_BATTERY:
		cartInfo.BatteryBacked = true;
		cartInfo.RamSize       = MBC2::RAM_SIZE;
		mapper                 = std::make_unique<MBC2>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC3_TIMER_BATTERY:
	case MBC_TYPES::MBC3_TIMER_RAM_BATERRY:
		errorMsg = "MBC3 with timer not implemented!";
		status   = false;
		break;

	case MBC_TYPES::MBC3:
	case MBC_TYPES::MBC3_RAM:
		mapper = std::make_unique<MBC3>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC3_RAM_BATTERY:
		cartInfo.BatteryBacked = true;
		mapper                 = std::make_unique<MBC3>(romFile, cartInfo);
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
	case MBC_TYPES::NO_MBC:
		return "No MBC";

	case MBC_TYPES::MBC1:
		return "MBC1";
	case MBC_TYPES::MBC1_RAM:
		return "MBC1+Ram";
	case MBC_TYPES::MBC1_RAM_BATTERY:
		return "MBC1+Ram+Battery";

	case MBC_TYPES::MBC2:
		return "MBC2";
	case MBC_TYPES::MBC2_BATTERY:
		return "MBC2+BATTERY";

	case MBC_TYPES::MBC3_TIMER_BATTERY:
		return "MBC3+TIMER+BATERRY";
	case MBC_TYPES::MBC3_TIMER_RAM_BATERRY:
		return "MBC3+TIMER+RAM+BATERRY";

	case MBC_TYPES::MBC3:
		return "MBC3";
	case MBC_TYPES::MBC3_RAM:
		return "MBC3+RAM";
	case MBC_TYPES::MBC3_RAM_BATTERY:
		return "MBC3+RAM+BATERRY";

	default:
		return "";
	}
}
