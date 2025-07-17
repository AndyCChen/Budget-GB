#include "mapper.h"
#include "mappers/noMBC.h"
#include "mappers/MBC1.h"

bool Mapper::loadMapper(std::ifstream &romFile, std::unique_ptr<IMapper> &mapper, const CartInfo &cartInfo, std::string &errorMsg)
{
	bool status = true;

	switch (cartInfo.mbcType)
	{

	case MBC_TYPES::NO_MBC:
		mapper = std::make_unique<NoMBC>(romFile, cartInfo);
		break;

	case MBC_TYPES::MBC_1:
		mapper = std::make_unique<MBC1>(romFile, cartInfo);
		break;

	default:
		errorMsg = "Unrecognized MBC!";
		status = false;
		break;

	}

	return status;
}

bool Mapper::isBatteryBacked(Mapper::MBC_TYPES mbcType)
{
	return false;
}
