#include "sm83.h"

Sm83::Sm83(Bus* bus)
{
	m_bus = bus;

	m_programCounter = 0x0100;
	m_stackPointer = 0xFFFE;

	m_registerAF.accumulator = 0x1;
	m_registerAF.flags.Z = 1;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.C = 0;
	
	m_registerBC.hi = 0x00;
	m_registerBC.lo = 0x13;

	m_registerDE.hi = 0x00;
	m_registerDE.lo = 0xD8;

	m_registerHL.hi = 0x01;
	m_registerHL.lo = 0x4D;
}

void Sm83::runInstruction()
{
	uint8_t opcode = cpuFetch();
	decodeExecute(opcode);
}

void Sm83::decodeExecute(uint8_t opcode)
{
	switch (opcode)
	{
		// NOP
		case 0x00:
			break;

		// LD BC, u16
		case 0x01:
			LD_r16_n16(m_registerBC);
			break;

		// LD (BC), A
		case 0x02:
			LD_indirect_r16_A(m_registerBC);
			break;

		// INC BC
		case 0x03:
			INC_r16(m_registerBC);
			break;

		// INC B
		case 0x04:
			INC_r8(m_registerBC.hi);
			break;

		// No intruction
		default:
			break;
	}
}

void Sm83::LD_r16_n16(Sm83Register& dest)
{
	dest.lo = cpuFetch();
	dest.hi = cpuFetch();
}

void Sm83::LD_indirect_r16_A(Sm83Register& dest)
{
	uint16_t address = (dest.hi << 8) | dest.lo;
	m_bus->cpu_write(address, m_registerAF.accumulator);
}

void Sm83::INC_r8(uint8_t& dest)
{
	// set half carry if lower 4 bits overflow from increment
	m_registerAF.flags.H = ((dest >> 4) & 0x1) ^ ( ((dest + 1) >> 4) & 0x1 );
	m_registerAF.flags.N = 0;
	dest += 1;
	m_registerAF.flags.Z = dest ? 0 : 1;
}

void Sm83::INC_r16(Sm83Register& dest)
{
	uint16_t value = ((dest.hi << 8) | dest.lo) + 1;
	dest.lo = value & 0xFF;
	dest.hi = (value >> 8) & 0xFF;
}



