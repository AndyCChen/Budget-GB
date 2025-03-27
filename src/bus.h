#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <cartridge.h>

namespace BusConstants
{
// memory sizes

inline constexpr unsigned int DEFAULT_WRAM_SIZE = 1024 * 2;
inline constexpr unsigned int TEST_MODE_WRAM_SIZE = 1024 * 64;
inline constexpr unsigned int VRAM_SIZE = 1024 * 8;

// address ranges

inline constexpr uint16_t CARTRIDGE_ROM_END = 0x8000;
inline constexpr uint16_t VRAM_END = 0xA000;
inline constexpr uint16_t EXTERNAL_RAM_END = 0xC000;
inline constexpr uint16_t ECHO_RAM_END = 0xFE00;
inline constexpr uint16_t OAM_END = 0xFEA0;
inline constexpr uint16_t UNUSABLE_END = 0xFF00;
inline constexpr uint16_t IO_REGISTERS_END = 0xFF80;
inline constexpr uint16_t HRAM_END = 0xFFFF;

} // namespace BusConstants

class Bus
{
	friend class Sm83JsonTest;

  public:
	enum class BusMode
	{
		NONE = 0,
		SM83_TEST, // set up bus with 64kb of unmapped wram
	};

	Bus(Cartridge *cartridge, BusMode mode = BusMode::NONE);

	void clearWram();
	uint8_t cpuRead(uint16_t position);
	void cpuWrite(uint16_t position, uint8_t data);

  private:
	BusMode m_mode;

	Cartridge *m_cartridge;

	// memory components

	std::vector<uint8_t> m_wram;
	std::array<uint8_t, BusConstants::VRAM_SIZE> m_vram;
};
