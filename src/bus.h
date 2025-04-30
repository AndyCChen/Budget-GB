#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "cartridge.h"

class Sm83; // forward declare Sm83
class Bus
{
	friend class Sm83JsonTest;

  public:
	// memory sizes

	static constexpr unsigned int DEFAULT_WRAM_SIZE   = 1024 * 8;
	static constexpr unsigned int TEST_MODE_WRAM_SIZE = 1024 * 64;
	static constexpr unsigned int VRAM_SIZE           = 1024 * 8;
	static constexpr unsigned int HRAM_SIZE           = 127;

	// address ranges

	static constexpr uint16_t CARTRIDGE_ROM_END = 0x8000;
	static constexpr uint16_t VRAM_END          = 0xA000;
	static constexpr uint16_t EXTERNAL_RAM_END  = 0xC000;
	static constexpr uint16_t ECHO_RAM_END      = 0xFE00;
	static constexpr uint16_t OAM_END           = 0xFEA0;
	static constexpr uint16_t UNUSABLE_END      = 0xFF00;
	static constexpr uint16_t IO_REGISTERS_END  = 0xFF80;
	static constexpr uint16_t HRAM_END          = 0xFFFF;

	// 

	enum class BusMode
	{
		NONE = 0,
		SM83_TEST, // set up bus with 64kb of unmapped wram
	};

	Bus(Cartridge &cartridge, Sm83 &cpu, BusMode mode = BusMode::NONE);

	void clearWram();

	/**
	 * @brief Same as cpuRead() but does not clock the cpu. Meant to be used by disassembler to read data without
	 * affected cpu state.
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
	void clearBus()
	{
		std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
		std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));
		std::fill(m_hram.begin(), m_hram.end(), static_cast<uint8_t>(0));
	}

  private:
	BusMode    m_mode;
	Cartridge &m_cartridge;
	Sm83      &m_cpu;

	// memory components

	std::vector<uint8_t>                m_wram;
	std::array<uint8_t, Bus::VRAM_SIZE> m_vram;
	std::array<uint8_t, Bus::HRAM_SIZE> m_hram;
};
