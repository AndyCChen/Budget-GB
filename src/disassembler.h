#pragma once

#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#include "bus.h"
#include "fmt/base.h"
#include "fmt/format.h"

struct DisassembledInstruction
{
	uint16_t m_opcodeAddress;
	std::string m_opcodeBytes;
	std::string m_opcodeString;

	void clear()
	{
		m_opcodeBytes.clear();
		m_opcodeString.clear();
	}
};

class Disassembler
{
public:
	Disassembler(Bus &bus) : m_bus(bus)
	{
		m_programCounter = 0;
		m_bufferPosition = 0;
		m_buffer.resize(10);
	}

	/**
	 * @brief Disassemble the next instruction.
	 */
	void instructionStep();

	void setProgramCounter(uint16_t pc)
	{
		m_programCounter = pc;
	}

	void logToConsole();

private:
	Bus &m_bus;
	uint16_t m_programCounter;

	std::size_t m_bufferPosition;
	std::vector<DisassembledInstruction> m_buffer;

	uint8_t fetch_n8()
	{
		uint8_t value = m_bus.cpuReadNoTick(m_programCounter++);
		formatToOpcodeBytes("{:02X} ", value);
		return value;
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

	template <typename... T> void formatToOpcodeBytes(fmt::format_string<T...> format, T &&...args)
	{
		fmt::format_to(std::back_inserter(m_buffer[m_bufferPosition].m_opcodeBytes), format,
					   std::forward<T>(args)...);
	}

	template <typename... T> void formatToOpcodeString(fmt::format_string<T...> format, T &&...args)
	{
		fmt::format_to(std::back_inserter(m_buffer[m_bufferPosition].m_opcodeString), format,
					   std::forward<T>(args)...);
	}
};
