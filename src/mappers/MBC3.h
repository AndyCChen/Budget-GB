#pragma once

#include "mapper.h"

#include <cstdint>
#include <fstream>
#include <vector>

namespace Mapper
{

// https://gbdev.io/pandocs/MBC3.html
class MBC3 : public IMapper
{
  public:
	MBC3(std::ifstream &romFile, const Mapper::CartInfo &cartInfo);
	~MBC3() override;

	virtual uint8_t read(uint16_t position) override;
	virtual void    write(uint16_t position, uint8_t data) override;
	virtual void    reset() override;

  private:
	std::vector<uint8_t> m_rom;
	std::vector<uint8_t> m_ram;

	struct Registers
	{
		uint8_t RomBankSelector  = 1;     // select 128 16kb rom banks, total 2mb rom
		uint8_t RamOrRtcSelector = 0;     // values 0x00 - 0x07 select a ram bank, 0x08 - 0x0C select a RTC register
		uint8_t LatchState       = 0;     // a transition from 0 -> 1 from a data write latches the current RTC time, repeat 0 -> 1 sequence to update latched time
		bool    RamAndRtcEnable  = false; // enable access to ram and rtc registers

		void reset()
		{
			RomBankSelector  = 1;
			RamOrRtcSelector = 0;
			LatchState       = 0;
			RamAndRtcEnable  = false;
		}
	} m_registers;

	enum RTC_Registers
	{
		RTC_S  = 0x08,
		RTC_M  = 0x09,
		RTC_H  = 0x0A,
		RTC_DL = 0x0B,
		RTC_DH = 0x0C,
	};

	struct RTC
	{
		uint8_t RtcSeconds        = 0;
		uint8_t RtcMinutes        = 0;
		uint8_t RtcHours          = 0;
		uint8_t RtcDaysLower8Bits = 0;
		uint8_t RtcDH             = 0; // Bit 0: bit 8 of day counter, Bit 6: halt rtc if set, Bit 7: Day counter overflow bit

		void reset()
		{
			RtcSeconds        = 0;
			RtcMinutes        = 0;
			RtcHours          = 0;
			RtcDaysLower8Bits = 0;
			RtcDH             = 0;
		}
	} m_rtc;

	RTC m_rtcLatch;
};

} // namespace Mapper