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
		setInstructionString("NOP");
		break;

	// LD BC, n16
	case 0x01:
		setInstructionString("LD BC, n16");
		LD_r16_n16(m_registerBC);
		break;

	// LD (BC), A
	case 0x02:
		setInstructionString("LD (BC), A");
		LD_indirect_r16_r8(m_registerBC, m_registerAF.accumulator);
		break;

	// INC BC
	case 0x03:
		setInstructionString("INC BC");
		INC_r16(m_registerBC);
		break;

	// INC B
	case 0x04:
		setInstructionString("INC B");
		INC_r8(m_registerBC.hi);
		break;

	// DEC B
	case 0x05:
		setInstructionString("DEC B");
		DEC_r8(m_registerBC.hi);
		break;

	// LD B n8
	case 0x06:
		setInstructionString("LD B n8");
		LD_r8_n8(m_registerBC.hi);
		break;

	// RLCA
	case 0x07:
		setInstructionString("RLCA");
		RLCA();
		break;

	// LD (n16), SP
	case 0x08:
		setInstructionString("LD (n16), SP");
		LD_indirect_n16_SP();
		break;

	// ADD HL, BC
	case 0x09:
		setInstructionString("ADD HL, BC");
		ADD_HL_r16(m_registerBC);
		break;

	// LD A, (BC)
	case 0x0A:
		setInstructionString("LD A, (BC)");
		LD_A_indirect_r16(m_registerBC);
		break;

	// DEC BC
	case 0x0B:
		setInstructionString("DEC BC");
		DEC_r16(m_registerBC);
		break;

	// INC C
	case 0x0C:
		setInstructionString("INC C");
		INC_r8(m_registerBC.lo);
		break;

	// DEC C
	case 0x0D:
		setInstructionString("DEC C");
		DEC_r8(m_registerBC.lo);
		break;

	// LD C, n8
	case 0x0E:
		setInstructionString("LD C, n8");
		LD_r8_n8(m_registerBC.lo);
		break;

	// RRCA
	case 0x0F:
		setInstructionString("RRCA");
		RRCA();
		break;

	// STOP
	case 0x10:
		setInstructionString("STOP");
		break;

	// LD DE, n16
	case 0x11:
		LD_r16_n16(m_registerDE);
		setInstructionString("LD DE, n16");
		break;

	// LD (DE), A
	case 0x12:
		LD_indirect_r16_r8(m_registerDE, m_registerAF.accumulator);
		setInstructionString("LD (DE), A");
		break;

	// INC DE
	case 0x13:
		INC_r16(m_registerDE);
		setInstructionString("INC DE");
		break;

	// INC D
	case 0x14:
		INC_r8(m_registerDE.hi);
		setInstructionString("INC D");
		break;

	// DEC D
	case 0x15:
		DEC_r8(m_registerDE.hi);
		setInstructionString("DEC D");
		break;

	// LD, D, n8
	case 0x16:
		LD_r8_n8(m_registerDE.hi);
		setInstructionString("LD, D, n8");
		break;

	// RLA
	case 0x17:
		RLA();
		setInstructionString("RLA");
		break;

	// JR i8
	case 0x18:
		JR_i8();
		setInstructionString("JR i8");
		break;

	// ADD HL, DE
	case 0x19:
		ADD_HL_r16(m_registerDE);
		setInstructionString("ADD HL, DE");
		break;

	// LD A, (DE)
	case 0x1A:
		LD_A_indirect_r16(m_registerDE);
		setInstructionString("LD A, (DE)");
		break;

	// DEC DE
	case 0x1B:
		DEC_r16(m_registerDE);
		setInstructionString("DEC DE");
		break;

	// INC E
	case 0x1C:
		INC_r8(m_registerDE.lo);
		setInstructionString("INC E");
		break;

	// DEC E
	case 0x1D:
		DEC_r8(m_registerDE.lo);
		setInstructionString("DEC E");
		break;

	// LD E, n8
	case 0x1E:
		LD_r8_n8(m_registerDE.lo);
		setInstructionString("LD E, n8");
		break;

	// RRA
	case 0x1F:
		RRA();
		setInstructionString("RRA");
		break;

	// JR NZ, i8
	case 0x20:
		JR_CC_i8(!m_registerAF.flags.Z);
		setInstructionString("JR NZ, i8");
		break;

	// LD HL, n16
	case 0x21:
		LD_r16_n16(m_registerHL);
		setInstructionString("LD HL, n16");
		break;

	// LD (HL+), A
	case 0x22:
		LD_indirect_HLI_A();
		setInstructionString("LD (HL+), A");
		break;

	// INC HL
	case 0x23:
		setInstructionString("INC HL");
		INC_r16(m_registerHL);
		break;

	// INC H
	case 0x24:
		setInstructionString("INC H");
		INC_r8(m_registerHL.hi);
		break;

	// DEC H
	case 0x25:
		setInstructionString("DEC H");
		DEC_r8(m_registerHL.hi);
		break;

	// LD H, n8
	case 0x26:
		setInstructionString("LD H, n8");
		LD_r8_n8(m_registerHL.hi);
		break;

	// DAA
	case 0x27:
		setInstructionString("DAA");
		DAA();
		break;

	// JR Z, i8
	case 0x28:
		setInstructionString("JR Z, i8");
		JR_CC_i8(m_registerAF.flags.Z);
		break;

	// ADD HL, HL
	case 0x29:
		setInstructionString("ADD HL, HL");
		ADD_HL_r16(m_registerHL);
		break;

	// LD A, (HL+)
	case 0x2A:
		setInstructionString("LD A, (HL+)");
		LD_A_indirect_HLI();
		break;

	// DEC HL
	case 0x2B:
		setInstructionString("DEC HL");
		DEC_r16(m_registerHL);
		break;

	// INC L
	case 0x2C:
		setInstructionString("INC L");
		INC_r8(m_registerHL.lo);
		break;

	// DEC L
	case 0x2D:
		setInstructionString("DEC L");
		DEC_r8(m_registerHL.lo);
		break;

	// LD L, n8
	case 0x2E:
		setInstructionString("LD L, n8");
		LD_r8_n8(m_registerHL.lo);
		break;

	// CPL
	case 0x2F:
		setInstructionString("CPL");
		CPL();
		break;

	// JR NC, i8
	case 0x30:
		setInstructionString("JR NC, i8");
		JR_CC_i8(!m_registerAF.flags.C);
		break;

	// LD SP, n16
	case 0x31:
		setInstructionString("LD SP, n16");
		LD_SP_n16();
		break;

	// LD (HL-), A
	case 0x32:
		setInstructionString("LD (HL-), A");
		LD_indirect_HLD_A();
		break;

	// INC SP
	case 0x33:
		setInstructionString("INC SP");
		INC_SP();
		break;

	// INC (HL)
	case 0x34:
		setInstructionString("INC (HL)");
		INC_indirect_HL();
		break;

	// DEC (HL)
	case 0x35:
		setInstructionString("DEC (HL)");
		DEC_indirect_HL();
		break;

	// LD (HL), n8
	case 0x36:
		setInstructionString("LD (HL), n8");
		LD_indirect_HL_n8();
		break;

	// SCF
	case 0x37:
		setInstructionString("SCF");
		SCF();
		break;

	// JR C, i8
	case 0x38:
		setInstructionString("JR C, i8");
		JR_CC_i8(m_registerAF.flags.C);
		break;

	// ADD HL, SP
	case 0x39:
		setInstructionString("ADD HL, SP");
		ADD_HL_SP();
		break;

	// LD A, HLD
	case 0x3A:
		setInstructionString("LD A, HLD");
		LD_A_indirect_HLD();
		break;

	// DEC SP
	case 0x3B:
		setInstructionString("DEC SP");
		DEC_SP();
		break;

	// INC A
	case 0x3C:
		setInstructionString("INC A");
		INC_r8(m_registerAF.accumulator);
		break;

	// DEC A
	case 0x3D:
		setInstructionString("DEC A");
		DEC_r8(m_registerAF.accumulator);
		break;

	// LD A, n8
	case 0x3E:
		setInstructionString("LD A, n8");
		LD_r8_n8(m_registerAF.accumulator);
		break;

	// CCF
	case 0x3F:
		setInstructionString("CCF");
		CCF();
		break;

	// LD B, B
	case 0x40:
		setInstructionString("LD B, B");
		LD_r8_r8(m_registerBC.hi, m_registerBC.hi);
		break;

	case 0x41:
		setInstructionString("LD B, C");
		LD_r8_r8(m_registerBC.hi, m_registerBC.lo);
		break;

	case 0x42:
		setInstructionString("LD B, D");
		LD_r8_r8(m_registerBC.hi, m_registerDE.hi);
		break;

	case 0x43:
		setInstructionString("LD B, E");
		LD_r8_r8(m_registerBC.hi, m_registerDE.lo);
		break;

	case 0x44:
		setInstructionString("LD B, H");
		LD_r8_r8(m_registerBC.hi, m_registerHL.hi);
		break;

	case 0x45:
		setInstructionString("LD B, L");
		LD_r8_r8(m_registerBC.hi, m_registerHL.lo);
		break;

	case 0x46:
		setInstructionString("LD B, (HL)");
		LD_r8_indirect_HL(m_registerBC.hi);
		break;

	case 0x47:
		setInstructionString("LD B, A");
		LD_r8_r8(m_registerBC.hi, m_registerAF.accumulator);
		break;

	case 0x48:
		setInstructionString("LD C, B");
		LD_r8_r8(m_registerBC.lo, m_registerBC.hi);
		break;

	case 0x49:
		setInstructionString("LD C, C");
		LD_r8_r8(m_registerBC.lo, m_registerBC.lo);
		break;

	case 0x4A:
		setInstructionString("LD C, D");
		LD_r8_r8(m_registerBC.lo, m_registerDE.hi);
		break;

	case 0x4B:
		setInstructionString("LD C, E");
		LD_r8_r8(m_registerBC.lo, m_registerDE.lo);
		break;

	case 0x4C:
		setInstructionString("LD C, H");
		LD_r8_r8(m_registerBC.lo, m_registerHL.hi);
		break;

	case 0x4D:
		setInstructionString("LD C, L");
		LD_r8_r8(m_registerBC.lo, m_registerHL.lo);
		break;

	case 0x4E:
		setInstructionString("LD C, (HL)");
		LD_r8_indirect_HL(m_registerBC.lo);
		break;

	case 0x4F:
		setInstructionString("LD C, A");
		LD_r8_r8(m_registerBC.lo, m_registerAF.accumulator);
		break;
	
	case 0x50:
		setInstructionString("LD D, B");
		LD_r8_r8(m_registerDE.hi, m_registerBC.hi);
		break;

	case 0x51:
		setInstructionString("LD D, C");
		LD_r8_r8(m_registerDE.hi, m_registerBC.lo);
		break;
	case 0x52:
		setInstructionString("LD D, D");
		LD_r8_r8(m_registerDE.hi, m_registerDE.hi);
		break;

	case 0x53:
		setInstructionString("LD D, E");
		LD_r8_r8(m_registerDE.hi, m_registerDE.lo);
		break;

	case 0x54:
		setInstructionString("LD D, H");
		LD_r8_r8(m_registerDE.hi, m_registerHL.hi);
		break;

	case 0x55:
		setInstructionString("LD D, L");
		LD_r8_r8(m_registerDE.hi, m_registerHL.lo);
		break;
	case 0x56:
		setInstructionString("LD D, (HL)");
		LD_r8_indirect_HL(m_registerDE.hi);
		break;

	case 0x57:
		setInstructionString("LD D, A");
		LD_r8_r8(m_registerDE.hi, m_registerAF.accumulator);
		break;

	case 0x58:
		setInstructionString("LD E, B");
		LD_r8_r8(m_registerDE.lo, m_registerBC.hi);
		break;
	case 0x59:
		setInstructionString("LD E, C");
		LD_r8_r8(m_registerDE.lo, m_registerBC.lo);
		break;

	case 0x5A:
		setInstructionString("LD E, D");
		LD_r8_r8(m_registerDE.lo, m_registerDE.hi);
		break;
	case 0x5B:
		setInstructionString("LD E, E");
		LD_r8_r8(m_registerDE.lo, m_registerDE.lo);
		break;

	case 0x5C:
		setInstructionString("LD E, H");
		LD_r8_r8(m_registerDE.lo, m_registerHL.hi);
		break;

	case 0x5D:
		setInstructionString("LD E, L");
		LD_r8_r8(m_registerDE.lo, m_registerHL.lo);
		break;

	case 0x5E:
		setInstructionString("LD E, (HL)");
		LD_r8_indirect_HL(m_registerDE.lo);
		break;

	case 0x5F:
		setInstructionString("LD E, A");
		LD_r8_r8(m_registerDE.lo, m_registerAF.accumulator);
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

void Sm83::LD_r8_r8(uint8_t &dest, uint8_t &src)
{
	dest = src;
}

void Sm83::LD_r16_n16(Sm83Register &dest)
{
	dest.lo = cpuFetch();
	dest.hi = cpuFetch();
}

void Sm83::LD_SP_n16()
{
	m_stackPointer = cpuFetch() | (cpuFetch() << 8);
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

void Sm83::LD_indirect_HL_n8()
{
	uint8_t value = cpuFetch();
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus->cpu_write(address, value);
}

void Sm83::LD_r8_indirect_HL(uint8_t &dest)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	dest = m_bus->cpu_read(address);
}

void Sm83::LD_indirect_HLI_A()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus->cpu_write(address, m_registerAF.accumulator);
	address += 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_indirect_HLD_A()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus->cpu_write(address, m_registerAF.accumulator);
	address -= 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_A_indirect_HLI()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_registerAF.accumulator = m_bus->cpu_read(address);
	address += 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_A_indirect_HLD()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_registerAF.accumulator = m_bus->cpu_read(address);
	address -= 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
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

void Sm83::INC_SP()
{
	m_stackPointer += 1;
}

void Sm83::INC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t value = m_bus->cpu_read(address);
	m_registerAF.flags.H = ((value >> 4) & 0x1) ^ (((value + 1) >> 4) & 0x1);
	value += 1;
	m_registerAF.flags.Z = value ? 0 : 1;
	m_registerAF.flags.N = 0;
	m_bus->cpu_write(address, value);
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

void Sm83::DEC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t value = m_bus->cpu_read(address);
	m_registerAF.flags.H = (value & 0xF0) != ((value - 1) & 0xF0);
	value -= 1;
	m_registerAF.flags.N = 1;
	m_registerAF.flags.Z = value ? 0 : 1;
	m_bus->cpu_write(address, value);
}

void Sm83::DEC_SP()
{
	m_stackPointer -= 1;
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

void Sm83::ADD_HL_SP()
{
	uint16_t hl = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint32_t sum = hl + m_stackPointer;

	// set half carry when lower 12 bit sum is greater than 0xFFF (aka overflowed)
	m_registerAF.flags.H = ((hl & 0xFFF) + (m_stackPointer & 0xFFF)) > 0xFFF;
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

void Sm83::DAA()
{
	uint16_t adjustment = 0;

	// previous arithmetic instruction was a subtraction operation
	if (m_registerAF.flags.N)
	{
		if (m_registerAF.flags.H)
			adjustment += 0x06;
		if (m_registerAF.flags.C)
			adjustment += 0x60;

		m_registerAF.accumulator -= adjustment;
	}
	else
	{
		if (m_registerAF.flags.H || ((m_registerAF.accumulator & 0xF) > 0x9))
			adjustment += 0x06;
		if (m_registerAF.flags.C || (m_registerAF.accumulator > 0x99))
		{
			adjustment += 0x60;
			m_registerAF.flags.C = 1;
		}
		m_registerAF.accumulator += adjustment;
	}

	m_registerAF.flags.Z = m_registerAF.accumulator == 0;
	m_registerAF.flags.H = 0;
}

void Sm83::CPL()
{
	m_registerAF.accumulator = ~m_registerAF.accumulator;
	m_registerAF.flags.N = 1;
	m_registerAF.flags.H = 1;
}

void Sm83::SCF()
{
	m_registerAF.flags.C = 1;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::CCF()
{
	m_registerAF.flags.C = !m_registerAF.flags.C;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}
