#include "MBC2.h"

Mapper::MBC2::MBC2(std::ifstream &romFile, const Mapper::CartInfo &cartInfo)
	: IMapper(cartInfo)
{
	m_rom.resize(cartInfo.RomSize);

	romFile.seekg(0);
	romFile.read(reinterpret_cast<char *>(m_rom.data()), cartInfo.RomSize);
	romFile.seekg(0);

	if (m_cartInfo.BatteryBacked)
		loadSaveRam(m_ram.data(), m_ram.size());
}

Mapper::MBC2::~MBC2()
{
	if (m_cartInfo.BatteryBacked)
		dumpBatteryBackedRam(m_ram.data(), m_ram.size());
}

uint8_t Mapper::MBC2::read(uint16_t position)
{
	uint8_t data = 0xFF;

	if (position <= 0x3FFF)
		data = m_rom[position];
	else if (position <= 0x7FFF)
	{
		// BB BB00 00PP PPPP PPPP
		// || ||     ++-++++-++++-- P: position (0-16kb offset within a bank)
		// ++-++------------------- B: bank number (1-15)

		uint32_t address = (m_registers.RomBankSelect << 14) | (position & 0x3FFF);
		data             = m_rom[address];
	}
	else if (position >= 0xA000 && position <= 0xBFFF && m_registers.RamEnable)
		data = m_ram[position & 0x1FF];

	return data;
}

void Mapper::MBC2::write(uint16_t position, uint8_t data)
{
	// bit 8 determines whether to toggle ram enable/disable or so modify rom bank selection
	if (position <= 0x3FFF)
	{
		// set bank select
		if (position & 0x100)
			m_registers.RomBankSelect = (data & 0xF) != 0 ? (data & 0xF) : 1;
		else
			m_registers.RamEnable = (data & 0xF) == 0xA; // enable ram if low nibble is 0xF, else disable ram
	}
	else if (m_registers.RamEnable && position >= 0xA000 && position <= 0xBFFF)
	{
		m_ram[position & 0x1FF] = data;
	}
}

void Mapper::MBC2::reset()
{
	m_registers.RamEnable     = false;
	m_registers.RomBankSelect = 1;
}
