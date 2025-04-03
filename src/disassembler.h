#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <iterator>

#include "bus.h"
#include "fmt/base.h"

struct DisassembledInstruction
{
	uint16_t m_instructionAddress;
	std::string m_instructionBytes;
	std::string m_instructionString;

	void clear()
	{
		m_instructionBytes.clear();
		m_instructionString.clear();
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

	uint8_t fetch()
	{
		uint8_t value = m_bus.cpuReadNoTick(m_programCounter++);
		formatToInstructionBytes("{:02X} ", value);
		return value;
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

	template <typename... T> void formatToInstructionBytes(fmt::format_string<T...> format, T&&... args)
	{
		fmt::format_to(std::back_inserter(m_buffer[m_bufferPosition].m_instructionBytes), format, std::forward<T>(args)...);
	}

	template <typename... T> void formatToInstructionString(fmt::format_string<T...> format, T &&...args)
	{
		fmt::format_to(std::back_inserter(m_buffer[m_bufferPosition].m_instructionString), format, std::forward<T>(args)...);
	}

};