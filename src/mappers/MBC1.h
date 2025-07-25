#pragma once

#include "fmt/core.h"
#include "mapper.h"

#include <cstdint>
#include <fstream>
#include <vector>

namespace Mapper
{

class MBC1 : public Mapper::IMapper
{
  public:
	MBC1(std::ifstream &romFile, const Mapper::CartInfo &cartInfo);

	~MBC1() override
	{
	}

	uint8_t read(uint16_t position) override;
	void    write(uint16_t position, uint8_t data) override;

  private:
	std::vector<uint8_t> m_rom;
	std::vector<uint8_t> m_ram;

	uint32_t m_romBank = 0;
	uint32_t m_ramBank = 0;

	// registers

	struct Registers
	{
		bool    RamEnable = false;
		uint8_t RomBankSelector : 5; // 5 bit rom bank number
		uint8_t Extra2Bits : 2;      // extra 2 bits for more rom banks in large cartridges (+512kb rom size) or acts as ram bank selector
		uint8_t BankModeSelect : 1; // controls behavior of the 2 bit register
	} m_registers;
};

} // namespace Mapper