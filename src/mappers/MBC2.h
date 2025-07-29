#pragma once

#include "mapper.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

namespace Mapper
{

// https://gbdev.io/pandocs/MBC2.html
class MBC2 : public IMapper
{
  public:
	MBC2(std::ifstream &romFile, const Mapper::CartInfo &cartInfo);
	~MBC2();

	virtual uint8_t read(uint16_t position) override;
	virtual void    write(uint16_t position, uint8_t data) override;
	virtual void    reset() override;

	static constexpr uint16_t RAM_SIZE = 512; // mbc2 has a fixed 512 bytes of internal ram only

  private:
	std::vector<uint8_t>     m_rom;
	std::array<uint8_t, RAM_SIZE> m_ram{}; // 512 4 bit values as ram. Lower four bits treated as are undefined.

	struct Registers
	{
		uint8_t RomBankSelect = 1;
		bool    RamEnable     = false;
	} m_registers;
};

} // namespace Mapper