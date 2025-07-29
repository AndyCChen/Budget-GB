#include "MBC1.h"

Mapper::MBC1::MBC1(std::ifstream &romFile, const Mapper::CartInfo &cartInfo)
	: IMapper(cartInfo)
{
	m_rom.resize(cartInfo.RomSize);
	m_ram.resize(cartInfo.RamSize);

	romFile.seekg(0);
	romFile.read(reinterpret_cast<char *>(m_rom.data()), cartInfo.RomSize);
	romFile.seekg(0);

	std::fill(m_ram.begin(), m_ram.end(), (uint8_t)0);

	m_registers.RamEnable       = false;
	m_registers.RomBankSelector = 1;
	m_registers.Extra2Bits      = 0;
	m_registers.BankModeSelect  = 0;
}

Mapper::MBC1::~MBC1()
{
	
}

uint8_t Mapper::MBC1::read(uint16_t position)
{
	uint8_t data = 0xFF;

	if (position <= 0x3FFF)
	{
		if (m_registers.BankModeSelect == 0)
			data = m_rom[position];
		else
		{
			uint32_t address = (m_registers.Extra2Bits << 19) | position;
			data             = m_rom[address];
		}
	}
	else if (position <= 0x7FFF)
	{
		// form the bank number from the 5 bit and 2 bit registers
		uint32_t bankNumber = (m_registers.Extra2Bits << 5) | m_registers.RomBankSelector;

		// bit mask with proper rom size
		bankNumber &= (static_cast<uint32_t>(m_cartInfo.RomSize / 16384) - 1);

		uint32_t address = (bankNumber << 14) | (position & 0x3FFF);
		data             = m_rom[address];
	}
	else if (m_registers.RamEnable && m_ram.size() != 0 && position >= 0xA000 && position <= 0xBFFF)
	{
		if (m_registers.BankModeSelect == 0)
			data = m_ram[position & 0x1FFF];
		else
		{
			uint32_t bankNumber = m_registers.Extra2Bits & (static_cast<uint32_t>(m_cartInfo.RamSize / 8192) - 1);
			uint32_t address    = (bankNumber << 13) | (position & 0x1FFF);
			data                = m_ram[address];
		}
	}

	return data;
}

void Mapper::MBC1::write(uint16_t position, uint8_t data)
{
	// ram enable/disable with '0xA' in lower nible
	if (position <= 0x1FFF)
	{
		m_registers.RamEnable = (data & 0xF) == 0xA;
	}
	// set rom bank select
	else if (position <= 0x3FFF)
	{
		if (data == 0x00)
			m_registers.RomBankSelector = 1;
		else
			m_registers.RomBankSelector = data;
	}
	// set ram bank selector or upper 2 bits of rom bank select
	else if (position <= 0x5FFF)
	{
		m_registers.Extra2Bits = data;
	}
	// set banking mode
	else if (position <= 0x7FFF)
	{
		m_registers.BankModeSelect = data;
	}
	else if (m_registers.RamEnable && m_ram.size() != 0 && position >= 0xA000 && position <= 0xBFFF)
	{
		if (m_registers.BankModeSelect == 0)
			m_ram[position & 0x1FFF] = data;
		else
		{
			uint32_t bankNumber = m_registers.Extra2Bits & (static_cast<uint32_t>(m_cartInfo.RamSize / 8192) - 1);
			uint32_t address    = (bankNumber << 13) | (position & 0x1FFF);
			m_ram[address]      = data;
		}
	}
}

void Mapper::MBC1::reset()
{
	m_registers.RamEnable       = false;
	m_registers.RomBankSelector = 1;
	m_registers.Extra2Bits      = 0;
	m_registers.BankModeSelect  = 0;
}
