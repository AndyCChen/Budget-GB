#include "noMBC.h"

using namespace Mapper;

NoMBC::NoMBC(std::ifstream &romFile, const CartInfo &cartInfo)
	: IMapper(cartInfo)
{
	romFile.seekg(0);
	romFile.read(reinterpret_cast<char *>(m_rom.data()), cartInfo.romSize);
	romFile.seekg(0);
}

uint8_t NoMBC::read(uint16_t position)
{
	if (position < m_rom.size())
		return m_rom[position];
	else
		return 0xFF;
}

void NoMBC::write(uint16_t position, uint8_t data)
{
	return;	
}
