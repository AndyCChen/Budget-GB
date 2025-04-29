#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include <string>

#include "bus.h"
#include "fmt/base.h"
#include "fmt/format.h"

struct NextInstruction
{
	std::optional<uint16_t> m_opcodeAddress;
	char                    m_opcodeString[32];
	char                    m_buffer[64];

	NextInstruction()
	{
		m_opcodeAddress = std::nullopt;
		std::memset(m_opcodeString, 0, sizeof(m_opcodeString));
		std::memset(m_buffer, 0, sizeof(m_buffer));
	}
};

class Disassembler
{
  public:
	Disassembler(Bus &bus)
		: m_bus(bus)
	{
		m_programCounter = 0;
		m_bufferPosition = 0;
	}

	/**
	 * @brief Disassemble the several future opcodes.
	 */
	void step();

	void setProgramCounter(uint16_t pc)
	{
		m_programCounter = pc;
	}

	const char *getDisassemblyAt(std::size_t index);

  private:
	Bus     &m_bus;
	uint16_t m_programCounter;

	uint32_t                       m_bufferPosition;
	std::array<NextInstruction, 6> m_buffer;

	uint8_t fetch_n8()
	{
		return m_bus.cpuReadNoTick(m_programCounter++);
	}

	/**
	 * @brief Fetch 2 byte values stored in little endian order.
	 *
	 * @return
	 */
	uint16_t fetch_n16()
	{
		uint8_t lo = fetch_n8();
		uint8_t hi = fetch_n8();

		return (hi << 8) | lo;
	}

	/**
	 * @brief Gets the relative jump address for JR instructions
	 *
	 * @return
	 */
	uint16_t computeRelativeJump()
	{
		int8_t offset = static_cast<int8_t>(fetch_n8());
		return m_programCounter + static_cast<int8_t>(offset);
	}

	/**
	 * @brief Handle disassembly of opcode.
	 * @param cartridgeRom
	 */
	void disassembleOpcode(uint8_t opcode);

	/**
	 * @brief Handle disassembly of the second opcode table of prefixed instructions.
	 * @param cartridgeRom
	 */
	void disassemblePrefixedOpcode(uint8_t opcode);

	template <typename... T>
	void formatToOpcodeString(fmt::format_string<T...> format, T &&...args)
	{
		NextInstruction &intr = m_buffer[m_bufferPosition];
		std::memset(intr.m_opcodeString, 0, sizeof(intr.m_opcodeString));
		fmt::format_to_n(intr.m_opcodeString, sizeof(intr.m_opcodeString), format, std::forward<T>(args)...);
	}
};
