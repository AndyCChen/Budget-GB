#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "cartridge.h"
#include "ppu.h"
#include "utils/vec.h"

class Sm83; // forward declare Sm83
class Bus
{
	friend class Sm83JsonTest;

  public:
	// memory sizes

	static constexpr unsigned int WRAM_SIZE = 1024 * 8;
	static constexpr unsigned int HRAM_SIZE = 127;

	// address ranges

	static constexpr uint16_t CARTRIDGE_ROM_END = 0x8000;
	static constexpr uint16_t VRAM_END          = 0xA000;
	static constexpr uint16_t EXTERNAL_RAM_END  = 0xC000;
	static constexpr uint16_t ECHO_RAM_END      = 0xFE00;
	static constexpr uint16_t OAM_END           = 0xFEA0;
	static constexpr uint16_t UNUSABLE_END      = 0xFF00;
	static constexpr uint16_t IO_REGISTERS_END  = 0xFF80;
	static constexpr uint16_t HRAM_END          = 0xFFFF;

	Bus(Cartridge &cartridge, Sm83 &cpu, std::vector<Utils::array_u8Vec4> &lcdPixelBuffer);

	void clearWram();

	/**
	 * @brief Same as cpuRead() but does not clock the cpu. Meant to be used by instruction logger to read data without
	 * affecting cpu state.
	 * @param position
	 * @return
	 */
	uint8_t cpuReadNoTick(uint16_t position);

	/**
	 * @brief Reads contents off bus based on cpu memory map, clocks cpu for 1 M-cycle.
	 * @param position
	 * @return
	 */
	uint8_t cpuRead(uint16_t position);

	/**
	 * @brief Write content to the bus based on cpu memory map, clocks for 1 M-cycle.
	 * @param position
	 * @param data
	 */
	void cpuWrite(uint16_t position, uint8_t data);

	/**
	 * @brief Reset all memory components to zero.
	 */
	void resetBus()
	{
		m_tCyclePerFrame = 0;
		m_tCycles        = 0;

		std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
		std::fill(m_hram.begin(), m_hram.end(), static_cast<uint8_t>(0));
		m_ppu.reset();
	}

	// one m-cycle clock
	void tickM();

	/**
	 * @brief Run cpu for
	 */
	void onUpdate();

  private:
	Cartridge &m_cartridge;
	Sm83      &m_cpu;
	PPU        m_ppu;

	std::size_t m_tCycles        = 0; // track total elapsed gameboy cycles
	std::size_t m_tCyclePerFrame = 0; // track elasped cycles in a frame (1/60 of a second), decremented after frame ends

	// memory components

	std::array<uint8_t, Bus::WRAM_SIZE> m_wram;
	std::array<uint8_t, Bus::HRAM_SIZE> m_hram;

	enum IORegisters
	{
		SERIAL_SB = 0xFF01, // Serial transfer data

		TIMER_DIV  = 0xFF04, // divider register
		TIMER_TIMA = 0xFF05, // timer counter
		TIMER_TMA  = 0xFF06, // timer modulo (reload value for timer counter)
		TIMER_TAC  = 0xFF07, // timer control

		INTERRUPT_IF = 0xFF0F, // interrupt flag

		// PPU registers - control & status
		LCD_CONTROL = 0xFF40,
		LCD_STAT    = 0xFF41,
		LCD_LY      = 0xFF44,
		LCD_LYC     = 0xFF45,

		BGP = 0xFF47, // bg color palette

		// PPU registers - scrolling
		LCD_SCY = 0xFF42,
		LCD_SCX = 0xFF43,
		LCD_WY  = 0xFF4A,
		LCD_WX  = 0xFF4B,
	};

	void    writeIO(uint16_t position, uint8_t data);
	uint8_t readIO(uint16_t position);
};
