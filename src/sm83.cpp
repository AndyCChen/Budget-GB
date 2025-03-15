#include "sm83.h"
#include <cstdint>

Sm83::Sm83(Bus *bus)
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

	// LD BC, n16
	case 0x01:
		LD_r16_n16(m_registerBC);
		break;

	// LD (BC), A
	case 0x02:
		LD_indirect_r16_r8(m_registerBC, m_registerAF.accumulator);
		break;

	// INC BC
	case 0x03:
		INC_r16(m_registerBC);
		break;

	// INC B
	case 0x04:
		INC_r8(m_registerBC.hi);
		break;

	// DEC B
	case 0x05:
		DEC_r8(m_registerBC.hi);
		break;

	// LD B n8
	case 0x06:
		LD_r8_n8(m_registerBC.hi);
		break;

	// RLCA
	case 0x07:
		RLCA();
		break;

	// LD (n16), SP
	case 0x08:
		LD_indirect_n16_SP();
		break;

	// ADD HL, BC
	case 0x09:
		ADD_HL_r16(m_registerBC);
		break;

	// LD A, (BC)
	case 0x0A:
		LD_A_indirect_r16(m_registerBC);
		break;

	// DEC BC
	case 0x0B:
		DEC_r16(m_registerBC);
		break;

	// INC C
	case 0x0C:
		INC_r8(m_registerBC.lo);
		break;

	// DEC C
	case 0x0D:
		DEC_r8(m_registerBC.lo);
		break;

	// LD C, n8
	case 0x0E:
		LD_r8_n8(m_registerBC.lo);
		break;

	// RRCA
	case 0x0F:
		RRCA();
		break;

	// STOP
	case 0x10:
		break;

	// LD DE, n16
	case 0x11:
		LD_r16_n16(m_registerDE);
		break;

	// LD (DE), A
	case 0x12:
		LD_indirect_r16_r8(m_registerDE, m_registerAF.accumulator);
		break;

	// INC DE
	case 0x13:
		INC_r16(m_registerDE);
		break;

	// INC D
	case 0x14:
		INC_r8(m_registerDE.hi);
		break;

	// DEC D
	case 0x15:
		DEC_r8(m_registerDE.hi);
		break;

	// LD, D, n8
	case 0x16:
		LD_r8_n8(m_registerDE.hi);
		break;

	// RLA
	case 0x17:
		RLA();
		break;

	// JR i8
	case 0x18:
		JR_i8();
		break;

	// ADD HL, DE
	case 0x19:
		ADD_HL_r16(m_registerDE);
		break;

	// LD A, (DE)
	case 0x1A:
		LD_A_indirect_r16(m_registerDE);
		break;

	// DEC DE
	case 0x1B:
		DEC_r16(m_registerDE);
		break;

	// INC E
	case 0x1C:
		INC_r8(m_registerDE.lo);
		break;

	// DEC E
	case 0x1D:
		DEC_r8(m_registerDE.lo);
		break;

	// LD E, n8
	case 0x1E:
		LD_r8_n8(m_registerDE.lo);
		break;

	// RRA
	case 0x1F:
		RRA();
		break;

	// JR NZ, i8
	case 0x20:
		JR_CC_i8(!m_registerAF.flags.Z);
		break;

	// No intruction
	default:
		break;
	}
}

void Sm83::LD_r8_n8(uint8_t &dest)
{
	dest = cpuFetch();
}

void Sm83::LD_r16_n16(Sm83Register &dest)
{
	dest.lo = cpuFetch();
	dest.hi = cpuFetch();
}

void Sm83::LD_indirect_r16_r8(Sm83Register &dest, uint8_t &src)
{
	uint16_t address = (dest.hi << 8) | dest.lo;
	m_bus->cpu_write(address, src);
}

void Sm83::LD_A_indirect_r16(Sm83Register &src)
{
	uint16_t address = (src.hi << 8) | src.lo;
	m_registerAF.accumulator = m_bus->cpu_read(address);
}

void Sm83::LD_indirect_n16_SP()
{
	// fetch lo byte then hi byte
	uint16_t address = cpuFetch() | (cpuFetch() << 8);

	// store lo byte the hi byte at next address
	m_bus->cpu_write(address, m_stackPointer & 0xFF);
	m_bus->cpu_write(address + 1, (m_stackPointer & 0xFF00) >> 8);
}

void Sm83::INC_r8(uint8_t &dest)
{
	// set half carry if lower 4 bits overflow from increment
	m_registerAF.flags.H = ((dest >> 4) & 0x1) ^ (((dest + 1) >> 4) & 0x1);
	dest += 1;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.Z = dest ? 0 : 1;
}

void Sm83::INC_r16(Sm83Register &dest)
{
	uint16_t value = ((dest.hi << 8) | dest.lo) + 1;
	dest.lo = value & 0xFF;
	dest.hi = (value >> 8) & 0xFF;
}

void Sm83::DEC_r8(uint8_t &dest)
{
	// if the upper nibble changes after the decrement,
	// that means a borrow occured on bit 4
	m_registerAF.flags.H = (dest & 0xF0) != ((dest - 1) & 0xF0);
	dest -= 1;
	m_registerAF.flags.N = 1;
	m_registerAF.flags.Z = dest ? 0 : 1;
}

void Sm83::DEC_r16(Sm83Register &dest)
{
	uint16_t value = (dest.hi << 8) | dest.lo;
	value -= 1;
	dest.hi = (value & 0xFF00) >> 8;
	dest.lo = value & 0xFF;
}

void Sm83::RLCA()
{
	uint16_t shiftedVal = m_registerAF.accumulator << 1;

	// carry flag set to bit that is rotated out
	m_registerAF.flags.C = (shiftedVal & 0x100) >> 8;

	// bit zero set to value in carry flag
	shiftedVal |= m_registerAF.flags.C;

	m_registerAF.accumulator = static_cast<uint8_t>(shiftedVal);

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RRCA()
{
	// carry flag set to bit that will be shifted out
	m_registerAF.flags.C = m_registerAF.accumulator & 0x1;
	m_registerAF.accumulator >>= 1;
	m_registerAF.accumulator |= m_registerAF.flags.C << 7;

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RLA()
{
	uint16_t shiftedVal = m_registerAF.accumulator << 1;
	shiftedVal |= m_registerAF.flags.C;
	m_registerAF.flags.C = (0x100 & shiftedVal) >> 8;
	m_registerAF.accumulator = static_cast<uint8_t>(shiftedVal);

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RRA()
{
	uint16_t shiftedVal = m_registerAF.accumulator >> 1;
	shiftedVal |= m_registerAF.flags.C << 7;
	m_registerAF.flags.C = m_registerAF.accumulator & 0x1;
	m_registerAF.accumulator = static_cast<uint8_t>(shiftedVal);

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::ADD_HL_r16(Sm83Register &operand)
{
	uint16_t hl = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint16_t op = (operand.hi << 8) | operand.lo;
	uint32_t sum = hl + op;

	// set half carry when lower 12 bit sum is greater than 0xFFF (aka overflowed)
	m_registerAF.flags.H = ((hl & 0xFFF) + (op & 0xFFF)) > 0xFFF;
	m_registerAF.flags.C = (sum & 0x10000) >> 16;
	m_registerAF.flags.N = 0;

	m_registerHL.hi = (sum & 0xFF00) >> 8;
	m_registerHL.lo = sum & 0x00FF;
}

void Sm83::JR_i8()
{
	int8_t offset = static_cast<int8_t>(cpuFetch());
	m_programCounter += offset;
}

void Sm83::JR_CC_i8(bool condition)
{
	int8_t offset = static_cast<int8_t>(cpuFetch());

	// 1 extra m-cycle on jump taken
	// 1 m-cycle == 4 t-cycles
	if (condition)
		m_programCounter += offset;
}
