#include "disassembler.h"

void Disassembler::instructionStep()
{
	m_bufferPosition = (m_bufferPosition + 1) % m_buffer.size();
	m_buffer[m_bufferPosition].clear();
	
	m_buffer[m_bufferPosition].m_instructionAddress = m_programCounter;
	uint8_t opcode = fetch();
	disassembleOpcode(opcode);
}

void Disassembler::disassembleOpcode(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x00:
		formatToInstructionString("NOP");
		break;
	case 0x01:
		formatToInstructionString("LD BC, ");
		break;
	case 0xC3: {
		formatToInstructionString("JP ");
		uint8_t lo = fetch();
		uint8_t hi = fetch();
		formatToInstructionString("${:04X}", (hi << 8) | lo);
		break;
	}
	default:
		break;
	}


}

void Disassembler::disassemblePrefixedOpcode(uint8_t opcode)
{
}

void Disassembler::logToConsole()
{
	DisassembledInstruction &current = m_buffer[m_bufferPosition];
	fmt::println("${:04X} {:11s}{:s}", current.m_instructionAddress, current.m_instructionBytes, current.m_instructionString);
}
