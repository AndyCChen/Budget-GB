#include "MBC3.h"
#include "fmt/core.h"

Mapper::MBC3::MBC3(std::ifstream &romFile, const Mapper::CartInfo &cartInfo)
	: IMapper(cartInfo)
{
	m_rom.resize(cartInfo.RomSize);
	m_ram.resize(cartInfo.RamSize);

	romFile.seekg(0);
	romFile.read(reinterpret_cast<char *>(m_rom.data()), cartInfo.RomSize);
	romFile.seekg(0);
}

Mapper::MBC3::~MBC3()
{
}

uint8_t Mapper::MBC3::read(uint16_t position)
{
	uint8_t data = 0xFF;

	if (position <= 0x3FFF)
		data = m_rom[position];
	else if (position <= 0x7FFF)
	{
		// B BBBB BB00 00PP PPPP PPPP
		// | |||| ||     ++-++++-++++-- P: position (0-16kb offset within a bank)
		// +-++++-++------------------- B: bank number (1-127)

		uint32_t address = (m_registers.RomBankSelector << 14) | (position & 0x3FFF);
		data             = m_rom[address];
	}
	else if (m_registers.RamAndRtcEnable && position >= 0xA000 && position <= 0xBFFF)
	{
		if (m_registers.RamOrRtcSelector <= 0x07)
		{
			uint32_t address = (m_registers.RamOrRtcSelector << 13) | (position & 0x1FFF);
			data             = m_ram[address];
		}
		else
		{
			switch (m_registers.RamOrRtcSelector)
			{
			case RTC_S:
				data = m_rtc.RtcSeconds;
				break;
			case RTC_M:
				data = m_rtc.RtcMinutes;
				break;
			case RTC_H:
				data = m_rtc.RtcHours;
				break;
			case RTC_DL:
				data = m_rtc.RtcDaysLower8Bits;
				break;
			case RTC_DH:
				data = m_rtc.RtcDH;
				break;
			default:
				break;
			}
		}
	}

	return data;
}

void Mapper::MBC3::write(uint16_t position, uint8_t data)
{
	if (position <= 0x1FFF)
	{
		if (data == 0xA)
			m_registers.RamAndRtcEnable = true;
		else if (data == 0)
			m_registers.RamAndRtcEnable = false;
	}
	else if (position <= 0x3FFF)
		m_registers.RomBankSelector = (data & 0x7F) != 0 ? (data & 0x7F) : 1;
	else if (position <= 0x5FFF)
		m_registers.RamOrRtcSelector = data;
	else if (position <= 0x7FFF)
	{
		if (m_registers.LatchState == 0 && data == 1)
		{
			// latch rtc time
			m_rtcLatch = m_rtc;
		}

		m_registers.LatchState = data;
	}
	else if (m_registers.RamAndRtcEnable && position >= 0xA000 && position <= 0xBFFF)
	{
		if (m_registers.RamOrRtcSelector <= 0x07)
		{
			uint32_t address = (m_registers.RamOrRtcSelector << 13) | (position & 0x1FFF);
			m_ram[address] = data;
		}
		else
		{
			switch (m_registers.RamOrRtcSelector)
			{
			case RTC_S:
				m_rtc.RtcSeconds = data;
				break;
			case RTC_M:
				m_rtc.RtcMinutes = data;
				break;
			case RTC_H:
				m_rtc.RtcHours = data;
				break;
			case RTC_DL:
				m_rtc.RtcDaysLower8Bits = data;
				break;
			case RTC_DH:
				m_rtc.RtcDH = data;
				break;
			default:
				break;
			}
		}
	}
}

void Mapper::MBC3::reset()
{
	m_registers.reset();
	m_rtc.reset();
}
