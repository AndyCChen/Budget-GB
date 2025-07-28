#include "mapper.h"
#include "mappers/noMBC.h"
#include "mappers/MBC1.h"

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
		mapper = std::make_unique<MBC1>(romFile, cartInfo);
		break;

	default:
		errorMsg = "Unrecognized MBC!";
		status = false;
		break;

	}

	return status;
}

