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

	case 0x60:
		setInstructionString("LD H, B");
		LD_r8_r8(m_registerHL.hi, m_registerBC.hi);
		break;

	case 0x61:
		setInstructionString("LD H, C");
		LD_r8_r8(m_registerHL.hi, m_registerBC.lo);
		break;

	case 0x62:
		setInstructionString("LD H, D");
		LD_r8_r8(m_registerHL.hi, m_registerDE.hi);
		break;

	case 0x63:
		setInstructionString("LD H, E");
		LD_r8_r8(m_registerHL.hi, m_registerDE.lo);
		break;
	case 0x64:

		setInstructionString("LD H, H");
		LD_r8_r8(m_registerHL.hi, m_registerHL.hi);
		break;

	case 0x65:
		setInstructionString("LD H, L");
		LD_r8_r8(m_registerHL.hi, m_registerHL.lo);
		break;

	case 0x66:
		setInstructionString("LD H, (HL)");
		LD_r8_indirect_HL(m_registerHL.hi);
		break;

	case 0x67:
		setInstructionString("LD H, A");
		LD_r8_r8(m_registerHL.hi, m_registerAF.accumulator);
		break;

	case 0x68:
		setInstructionString("LD L, B");
		LD_r8_r8(m_registerHL.lo, m_registerBC.hi);
		break;

	case 0x69:
		setInstructionString("LD L, C");
		LD_r8_r8(m_registerHL.lo, m_registerBC.lo);
		break;

	case 0x6A:
		setInstructionString("LD L, D");
		LD_r8_r8(m_registerHL.lo, m_registerDE.hi);
		break;

	case 0x6B:
		setInstructionString("LD L, E");
		LD_r8_r8(m_registerHL.lo, m_registerDE.lo);
		break;

	case 0x6C:
		setInstructionString("LD L, H");
		LD_r8_r8(m_registerHL.lo, m_registerHL.hi);
		break;

	case 0x6D:
		setInstructionString("LD L, L");
		LD_r8_r8(m_registerHL.lo, m_registerHL.lo);
		break;

	case 0x6E:
		setInstructionString("LD L, (HL)");
		LD_r8_indirect_HL(m_registerHL.lo);
		break;

	case 0x6F:
		setInstructionString("LD L, A");
		LD_r8_r8(m_registerHL.lo, m_registerAF.accumulator);
		break;

	case 0x70:
		setInstructionString("LD (HL), B");
		LD_indirect_r16_r8(m_registerHL, m_registerBC.hi);
		break;

	case 0x71:
		setInstructionString("LD (HL), C");
		LD_indirect_r16_r8(m_registerHL, m_registerBC.lo);
		break;

	case 0x72:
		setInstructionString("LD (HL), D");
		LD_indirect_r16_r8(m_registerHL, m_registerDE.hi);
		break;

	case 0x73:
		setInstructionString("LD (HL), E");
		LD_indirect_r16_r8(m_registerHL, m_registerDE.lo);
		break;

	case 0x74:
		setInstructionString("LD (HL), H");
		LD_indirect_r16_r8(m_registerHL, m_registerHL.hi);
		break;

	case 0x75:
		setInstructionString("LD (HL), L");
		LD_indirect_r16_r8(m_registerHL, m_registerHL.lo);
		break;

	case 0x76:
		setInstructionString("HALT");
		break;

	case 0x77:
		setInstructionString("LD (HL), A");
		LD_indirect_r16_r8(m_registerHL, m_registerAF.accumulator);
		break;

	case 0x78:
		setInstructionString("LD A, B");
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.hi);
		break;

	case 0x79:
		setInstructionString("LD A, C");
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.lo);
		break;

	case 0x7A:
		setInstructionString("LD A, D");
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.hi);
		break;

	case 0x7B:
		setInstructionString("LD A, E");
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.lo);
		break;

	case 0x7C:
		setInstructionString("LD A, H");
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.hi);
		break;

	case 0x7D:
		setInstructionString("LD A,L");
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.lo);
		break;

	case 0x7E:
		setInstructionString("LD A, (HL)");
		LD_r8_indirect_HL(m_registerAF.accumulator);
		break;

	case 0x7F:
		setInstructionString("LD A, A");
		LD_r8_r8(m_registerAF.accumulator, m_registerAF.accumulator);
		break;

	case 0x80:
		setInstructionString("ADD A, B");
		ADD_A_r8(m_registerBC.hi);
		break;

	case 0x81:
		setInstructionString("ADD A, C");
		ADD_A_r8(m_registerBC.lo);
		break;

	case 0x82:
		setInstructionString("ADD A, D");
		ADD_A_r8(m_registerDE.hi);
		break;

	case 0x83:
		setInstructionString("ADD A, E");
		ADD_A_r8(m_registerDE.lo);
		break;

	case 0x84:
		setInstructionString("ADD A, H");
		ADD_A_r8(m_registerHL.hi);
		break;

	case 0x85:
		setInstructionString("ADD A, L");
		ADD_A_r8(m_registerHL.lo);
		break;

	case 0x86:
		setInstructionString("ADD A, (HL)");
		ADD_A_indirect_HL();
		break;

	case 0x87:
		setInstructionString("ADD A, A");
		ADD_A_r8(m_registerAF.accumulator);
		break;

	case 0x88:
		setInstructionString("ADC A, B");
		ADC_A_r8(m_registerBC.hi);
		break;

	case 0x89:
		setInstructionString("ADC A, C");
		ADC_A_r8(m_registerBC.lo);
		break;
	case 0x8A:

		setInstructionString("ADC A, D");
		ADC_A_r8(m_registerDE.hi);
		break;

	case 0x8B:
		setInstructionString("ADC A, E");
		ADC_A_r8(m_registerDE.lo);
		break;

	case 0x8C:
		setInstructionString("ADC A, H");
		ADC_A_r8(m_registerHL.hi);
		break;

	case 0x8D:
		setInstructionString("ADC A, L");
		ADC_A_r8(m_registerHL.lo);
		break;

	case 0x8E:
		setInstructionString("ADC A, (HL)");
		ADC_A_indirect_HL();
		break;

	case 0x8F:
		setInstructionString("ADC A, A");
		ADC_A_r8(m_registerAF.accumulator);
		break;

	case 0x90:
		setInstructionString("SUB A, B");
		SUB_A_r8(m_registerBC.hi);
		break;

	case 0x91:
		setInstructionString("SUB A, C");
		SUB_A_r8(m_registerBC.lo);
		break;

	case 0x92:
		setInstructionString("SUB A, D");
		SUB_A_r8(m_registerDE.hi);
		break;
	case 0x93:

		setInstructionString("SUB A, E");
		SUB_A_r8(m_registerDE.lo);
		break;

	case 0x94:
		setInstructionString("SUB A, H");
		SUB_A_r8(m_registerHL.hi);
		break;

	case 0x95:
		setInstructionString("SUB A, L");
		SUB_A_r8(m_registerHL.lo);
		break;

	case 0x96:
		setInstructionString("SUB A, (HL)");
		SUB_A_indirect_HL();
		break;

	case 0x97:
		setInstructionString("SUB A, A");
		SUB_A_r8(m_registerAF.accumulator);
		break;

	case 0x98:
		setInstructionString("SBC A, B");
		SBC_A_r8(m_registerBC.hi);
		break;

	case 0x99:
		setInstructionString("SBC A, C");
		SBC_A_r8(m_registerBC.lo);
		break;

	case 0x9A:
		setInstructionString("SBC A, D");
		SBC_A_r8(m_registerDE.hi);
		break;

	case 0x9B:
		setInstructionString("SBC A, E");
		SBC_A_r8(m_registerDE.lo);
		break;

	case 0x9C:
		setInstructionString("SBC A, H");
		SBC_A_r8(m_registerHL.hi);
		break;

	case 0x9D:
		setInstructionString("SBC A, L");
		SBC_A_r8(m_registerHL.lo);
		break;

	case 0x9E:
		setInstructionString("SBC A, (HL)");
		SBC_A_indirect_HL();
		break;

	case 0x9F:
		setInstructionString("SBC A, A");
		SBC_A_r8(m_registerAF.accumulator);
		break;

	case 0xA0:
		setInstructionString("AND A, B");
		AND_A_r8(m_registerBC.hi);
		break;

	case 0xA1:
		setInstructionString("AND A, C");
		AND_A_r8(m_registerBC.lo);
		break;

	case 0xA2:
		setInstructionString("AND A, D");
		AND_A_r8(m_registerDE.hi);
		break;

	case 0xA3:
		setInstructionString("AND A, E");
		AND_A_r8(m_registerDE.lo);
		break;

	case 0xA4:
		setInstructionString("AND A, H");
		AND_A_r8(m_registerHL.hi);
		break;

	case 0xA5:
		setInstructionString("AND A, L");
		AND_A_r8(m_registerHL.lo);
		break;

	case 0xA6:
		setInstructionString("AND A, (HL)");
		AND_A_indirect_HL();
		break;

	case 0xA7:
		setInstructionString("AND A, A");
		AND_A_r8(m_registerAF.accumulator);
		break;

	case 0xA8:
		setInstructionString("XOR A, B");
		XOR_A_r8(m_registerBC.hi);
		break;

	case 0xA9:
		setInstructionString("XOR A, C");
		XOR_A_r8(m_registerBC.lo);
		break;

	case 0xAA:
		setInstructionString("XOR A, D");
		XOR_A_r8(m_registerDE.hi);
		break;

	case 0xAB:
		setInstructionString("XOR A, E");
		XOR_A_r8(m_registerDE.lo);
		break;

	case 0xAC:
		setInstructionString("XOR A, H");
		XOR_A_r8(m_registerHL.hi);
		break;

	case 0xAD:
		setInstructionString("XOR A, L");
		XOR_A_r8(m_registerHL.lo);
		break;

	case 0xAE:
		setInstructionString("XOR A, (HL)");
		XOR_A_indirect_HL();
		break;

	case 0xAF:
		setInstructionString("XOR A, A");
		XOR_A_r8(m_registerAF.accumulator);
		break;

	case 0xB0:
		setInstructionString("OR A, B");
		OR_A_r8(m_registerBC.hi);
		break;

	case 0xB1:
		setInstructionString("OR A, C");
		OR_A_r8(m_registerBC.lo);
		break;

	case 0xB2:
		setInstructionString("OR A, D");
		OR_A_r8(m_registerDE.hi);
		break;

	case 0xB3:
		setInstructionString("OR A, E");
		OR_A_r8(m_registerDE.lo);
		break;

	case 0xB4:
		setInstructionString("OR A, H");
		OR_A_r8(m_registerHL.hi);
		break;

	case 0xB5:
		setInstructionString("OR A, L");
		OR_A_r8(m_registerHL.lo);
		break;

	case 0xB6:
		setInstructionString("OR A, (HL)");
		OR_A_indirect_HL();
		break;

	case 0xB7:
		setInstructionString("OR A, A");
		OR_A_r8(m_registerAF.accumulator);
		break;

	case 0xB8:
		setInstructionString("CP A, B");
		CP_A_r8(m_registerBC.hi);
		break;

	case 0xB9:
		setInstructionString("CP A, C");
		CP_A_r8(m_registerBC.lo);
		break;

	case 0xBA:
		setInstructionString("CP A, D");
		CP_A_r8(m_registerDE.hi);
		break;

	case 0xBB:
		setInstructionString("CP A, E");
		CP_A_r8(m_registerDE.lo);
		break;

	case 0xBC:
		setInstructionString("CP A, H");
		CP_A_r8(m_registerHL.hi);
		break;

	case 0xBD:
		setInstructionString("CP A, L");
		CP_A_r8(m_registerHL.lo);
		break;

	case 0xBE:
		setInstructionString("CP A, (HL)");
		CP_A_indirect_HL();
		break;

	case 0xBF:
		setInstructionString("CP A, A");
		CP_A_r8(m_registerAF.accumulator);
		break;

	case 0xC0:
		setInstructionString("RET NZ");
		RET_CC(!m_registerAF.flags.Z);
		break;

	case 0xC1:
		setInstructionString("POP BC");
		POP_r16(m_registerBC);
		break;

	case 0xC2:
		setInstructionString("JP NZ, n16");
		JP_CC_n16(!m_registerAF.flags.Z);
		break;

	case 0xC3:
		setInstructionString("JP n16");
		JP_n16();
		break;

	case 0xC4:
		setInstructionString("CALL NZ, n16");
		CALL_CC_n16(!m_registerAF.flags.Z);
		break;

	case 0xC5:
		setInstructionString("PUSH BC");
		PUSH_r16(m_registerBC);
		break;

	case 0xC6:
		setInstructionString("ADD n8");
		ADD_A_n8();
		break;

	case 0xC7:
		setInstructionString("RST 00h");
		RST(RstVector::H00);
		break;

	case 0xC8:
		setInstructionString("RET Z");
		RET_CC(m_registerAF.flags.Z);
		break;

	case 0xC9:
		setInstructionString("RET");
		RET();
		break;

	case 0xCA:
		setInstructionString("JP Z, n16");
		JP_CC_n16(m_registerAF.flags.Z);
		break;

	// Prefix mode, decode opcode with second opcode table
	case 0xCB:
		decodeExecutePrefixedMode(cpuFetch());
		break;

	case 0xCC:
		setInstructionString("CALL Z, n16");
		CALL_CC_n16(m_registerAF.flags.Z);
		break;

	case 0xCD:
		setInstructionString("CALL n16");
		CALL_n16();
		break;

	case 0xCE:
		setInstructionString("ADC A, n8");
		ADC_A_n8();
		break;

	case 0xCF:
		setInstructionString("RST 08h");
		RST(RstVector::H08);
		break;

	case 0xD0:
		setInstructionString("RET NC");
		RET_CC(!m_registerAF.flags.C);
		break;

	case 0xD1:
		setInstructionString("POP DE");
		POP_r16(m_registerDE);
		break;

	case 0xD2:
		setInstructionString("JP NC, n16");
		JP_CC_n16(!m_registerAF.flags.C);
		break;

	case 0xD3:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xD4:
		setInstructionString("CALL NC, n16");
		CALL_CC_n16(!m_registerAF.flags.C);
		break;

	case 0xD5:
		setInstructionString("PUSH DE");
		PUSH_r16(m_registerDE);
		break;

	case 0xD6:
		setInstructionString("SUB A, n8");
		SUB_A_n8();
		break;

	case 0xD7:
		setInstructionString("RST 10h");
		RST(RstVector::H10);
		break;

	case 0xD8:
		setInstructionString("RET C");
		RET_CC(m_registerAF.flags.C);
		break;

	case 0xD9:
		setInstructionString("RETI");
		RETI();
		break;

	case 0xDA:
		setInstructionString("JP C, n16");
		JP_CC_n16(m_registerAF.flags.C);
		break;

	case 0xDB:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xDC:
		setInstructionString("CALL C, n16");
		CALL_CC_n16(m_registerAF.flags.C);
		break;

	case 0xDD:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xDE:
		setInstructionString("SBC A, n8");
		SBC_A_n8();
		break;

	case 0xDF:
		setInstructionString("RST 18h");
		RST(RstVector::H18);
		break;

	case 0xE0:
		setInstructionString("LD (FF00 + n8), A");
		LDH_indirect_n8_A();
		break;

	case 0xE1:
		setInstructionString("POP HL");
		POP_r16(m_registerHL);
		break;

	case 0xE2:
		setInstructionString("LD (FF00 + C), A");
		LDH_indirect_C_A();
		break;

	case 0xE3:
	case 0xE4:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xE5:
		setInstructionString("PUSH HL");
		PUSH_r16(m_registerHL);
		break;

	case 0xE6:
		setInstructionString("AND A, n8");
		AND_A_n8();
		break;

	case 0xE7:
		setInstructionString("RST 20h");
		RST(RstVector::H20);
		break;

	case 0xE8:
		setInstructionString("ADD SP, i8");
		ADD_SP_i8();
		break;

	case 0xE9:
		setInstructionString("JP HL");
		JP_HL();
		break;

	case 0xEA:
		setInstructionString("LD (n16), A");
		LD_indirect_n16_A();
		break;

	case 0xEB:
	case 0xEC:
	case 0xED:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xEE:
		setInstructionString("XOR A, n8");
		XOR_A_n8();
		break;

	case 0xEF:
		setInstructionString("RST 28h");
		RST(RstVector::H28);
		break;
	
	case 0xF0:
		setInstructionString("LD A, (FF00 + n8)");
		LDH_A_indirect_n8();
		break;

	case 0xF1:
		setInstructionString("POP AF");
		POP_AF();
		break;

	case 0xF2:
		setInstructionString("LD A, (FF00 + C)");
		LDH_A_indirect_C();
		break;

	case 0xF3:
		setInstructionString("DI");
		DI();
		break;

	case 0xF4:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xF5:
		setInstructionString("PUSH AF");
		PUSH_AF();
		break;

	case 0xF6:
		setInstructionString("OR A, n8");
		OR_A_n8();
		break;

	case 0xF7:
		setInstructionString("RST 30h");
		RST(RstVector::H30);
		break;

	case 0xF8:
		setInstructionString("LD HL, SP + i8");
		LD_HL_SP_i8();
		break;

	case 0xF9:
		setInstructionString("LD SP, HL");
		LD_SP_HL();
		break;
	
	case 0xFA:
		setInstructionString("LD A, (n16)");
		LD_A_indirect_n16();
		break;

	case 0xFB:
		setInstructionString("EI");
		EI();
		break;

	case 0xFC:
	case 0xFD:
		setInstructionString("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xFE:
		setInstructionString("CP A, n8");
		CP_A_n8();
		break;
	
	case 0xFF:
		setInstructionString("RST 38h");
		RST(RstVector::H38);
		break;

	// No intruction
	default:
		break;
	}
}

void Sm83::decodeExecutePrefixedMode(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x00:
		setInstructionString("RLC B");
		RLC_r8(m_registerBC.hi);
		break;

	case 0x01:
		setInstructionString("RLC C");
		RLC_r8(m_registerBC.lo);
		break;

	case 0x02:
		setInstructionString("RLC D");
		RLC_r8(m_registerDE.hi);
		break;

	case 0x03:
		setInstructionString("RLC E");
		RLC_r8(m_registerDE.lo);
		break;

	case 0x04:
		setInstructionString("RLC H");
		RLC_r8(m_registerHL.hi);
		break;
	case 0x05:

		setInstructionString("RLC L");
		RLC_r8(m_registerHL.lo);
		break;

	case 0x06:
		setInstructionString("RLC (HL)");
		RLC_indirect_HL();
		break;

	case 0x07:
		setInstructionString("RLC A");
		RLC_r8(m_registerAF.accumulator);
		break;

	case 0x08:
		setInstructionString("RRC B");
		RRC_r8(m_registerBC.hi);
		break;

	case 0x09:
		setInstructionString("RRC C");
		RRC_r8(m_registerBC.lo);
		break;

	case 0x0A:
		setInstructionString("RRC D");
		RRC_r8(m_registerDE.hi);
		break;

	case 0x0B:
		setInstructionString("RRC E");
		RRC_r8(m_registerDE.lo);
		break;

	case 0x0C:
		setInstructionString("RRC H");
		RRC_r8(m_registerHL.hi);
		break;

	case 0x0D:
		setInstructionString("RRC L");
		RRC_r8(m_registerHL.lo);
		break;

	case 0x0E:
		setInstructionString("RRC (HL)");
		RRC_indirect_HL();
		break;

	case 0x0F:
		setInstructionString("RRC A");
		RRC_r8(m_registerAF.accumulator);
		break;

	case 0x10:
		setInstructionString("RL B");
		RL_r8(m_registerBC.hi);
		break;

	case 0x11:
		setInstructionString("RL C");
		RL_r8(m_registerBC.lo);
		break;

	case 0x12:
		setInstructionString("RL D");
		RL_r8(m_registerDE.hi);
		break;

	case 0x13:
		setInstructionString("RL E");
		RL_r8(m_registerDE.lo);
		break;
	case 0x14:

		setInstructionString("RL H");
		RL_r8(m_registerHL.hi);
		break;
	case 0x15:

		setInstructionString("RL L");
		RL_r8(m_registerHL.lo);
		break;

	case 0x16:
		setInstructionString("RL (HL)");
		RL_indirect_HL();
		break;

	case 0x17:
		setInstructionString("RL A");
		RL_r8(m_registerAF.accumulator);
		break;

	case 0x18:
		setInstructionString("RR B");
		RR_r8(m_registerBC.hi);
		break;

	case 0x19:
		setInstructionString("RR C");
		RR_r8(m_registerBC.lo);
		break;

	case 0x1A:
		setInstructionString("RR D");
		RR_r8(m_registerDE.hi);
		break;

	case 0x1B:
		setInstructionString("RR E");
		RR_r8(m_registerDE.lo);
		break;

	case 0x1C:
		setInstructionString("RR H");
		RR_r8(m_registerHL.hi);
		break;

	case 0x1D:
		setInstructionString("RR L");
		RR_r8(m_registerHL.lo);
		break;

	case 0x1E:
		setInstructionString("RR (HL)");
		RR_indirect_HL();
		break;

	case 0x1F:
		setInstructionString("RR A");
		RR_r8(m_registerAF.accumulator);
		break;

	case 0x20:
		setInstructionString("SLA B");
		SLA_r8(m_registerBC.hi);
		break;

	case 0x21:
		setInstructionString("SLA C");
		SLA_r8(m_registerBC.lo);
		break;

	case 0x22:
		setInstructionString("SLA D");
		SLA_r8(m_registerDE.hi);
		break;

	case 0x23:
		setInstructionString("SLA E");
		SLA_r8(m_registerDE.lo);
		break;

	case 0x24:
		setInstructionString("SLA H");
		SLA_r8(m_registerHL.hi);
		break;

	case 0x25:
		setInstructionString("SLA L");
		SLA_r8(m_registerHL.lo);
		break;

	case 0x26:
		setInstructionString("SLA (HL)");
		SLA_indirect_HL();
		break;

	case 0x27:
		setInstructionString("SLA A");
		SLA_r8(m_registerAF.accumulator);
		break;

	case 0x28:
		setInstructionString("SRA B");
		SRA_r8(m_registerBC.hi);
		break;

	case 0x29:
		setInstructionString("SRA C");
		SRA_r8(m_registerBC.lo);
		break;

	case 0x2A:
		setInstructionString("SRA D");
		SRA_r8(m_registerDE.hi);
		break;

	case 0x2B:
		setInstructionString("SRA E");
		SRA_r8(m_registerDE.lo);
		break;

	case 0x2C:
		setInstructionString("SRA H");
		SRA_r8(m_registerHL.hi);
		break;

	case 0x2D:
		setInstructionString("SRA L");
		SRA_r8(m_registerHL.lo);
		break;

	case 0x2E:
		setInstructionString("SRA (HL)");
		SRA_indirect_HL();
		break;

	case 0x2F:
		setInstructionString("SRA A");
		SRA_r8(m_registerAF.accumulator);
		break;

	case 0x30:
		setInstructionString("SWAP B");
		SWAP_r8(m_registerBC.hi);
		break;

	case 0x31:
		setInstructionString("SWAP C");
		SWAP_r8(m_registerBC.lo);
		break;

	case 0x32:
		setInstructionString("SWAP D");
		SWAP_r8(m_registerDE.hi);
		break;

	case 0x33:
		setInstructionString("SWAP E");
		SWAP_r8(m_registerDE.lo);
		break;

	case 0x34:
		setInstructionString("SWAP H");
		SWAP_r8(m_registerHL.hi);
		break;

	case 0x35:
		setInstructionString("SWAP L");
		SWAP_r8(m_registerHL.lo);
		break;

	case 0x36:
		setInstructionString("SWAP (HL)");
		SWAP_indirect_HL();
		break;

	case 0x37:
		setInstructionString("SWAP A");
		SWAP_r8(m_registerAF.accumulator);
		break;

	case 0x38:
		setInstructionString("SRL B");
		SRL_r8(m_registerBC.hi);
		break;

	case 0x39:
		setInstructionString("SRL C");
		SRL_r8(m_registerBC.lo);
		break;

	case 0x3A:
		setInstructionString("SRL D");
		SRL_r8(m_registerDE.hi);
		break;

	case 0x3B:
		setInstructionString("SRL E");
		SRL_r8(m_registerDE.lo);
		break;

	case 0x3C:
		setInstructionString("SRL H");
		SRL_r8(m_registerHL.hi);
		break;

	case 0x3D:
		setInstructionString("SRL L");
		SRL_r8(m_registerHL.lo);
		break;

	case 0x3E:
		setInstructionString("SRL (HL)");
		SRL_indirect_HL();
		break;

	case 0x3F:
		setInstructionString("SRL A");
		SRL_r8(m_registerAF.accumulator);
		break;

	case 0x40:
		setInstructionString("BIT 0, B");
		BIT_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0x41:
		setInstructionString("BIT 0, C");
		BIT_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0x42:
		setInstructionString("BIT 0, D");
		BIT_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0x43:
		setInstructionString("BIT 0, E");
		BIT_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0x44:
		setInstructionString("BIT 0, H");
		BIT_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0x45:
		setInstructionString("BIT 0, L");
		BIT_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0x46:
		setInstructionString("BIT 0, (HL)");
		BIT_indirect_HL(BitSelect::B0);
		break;

	case 0x47:
		setInstructionString("BIT 0, A");
		BIT_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0x48:
		setInstructionString("BIT 1, B");
		BIT_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0x49:
		setInstructionString("BIT 1, C");
		BIT_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0x4A:
		setInstructionString("BIT 1, D");
		BIT_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0x4B:
		setInstructionString("BIT 1, E");
		BIT_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0x4C:
		setInstructionString("BIT 1, H");
		BIT_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0x4D:
		setInstructionString("BIT 1, L");
		BIT_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0x4E:
		setInstructionString("BIT 1, (HL)");
		BIT_indirect_HL(BitSelect::B1);
		break;

	case 0x4F:
		setInstructionString("BIT 1, A");
		BIT_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0x50:
		setInstructionString("BIT 2, B");
		BIT_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0x51:
		setInstructionString("BIT 2, C");
		BIT_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0x52:
		setInstructionString("BIT 2, D");
		BIT_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0x53:
		setInstructionString("BIT 2, E");
		BIT_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0x54:
		setInstructionString("BIT 2, H");
		BIT_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0x55:
		setInstructionString("BIT 2, L");
		BIT_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0x56:
		setInstructionString("BIT 2, (HL)");
		BIT_indirect_HL(BitSelect::B2);
		break;

	case 0x57:
		setInstructionString("BIT 2, A");
		BIT_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0x58:
		setInstructionString("BIT 3, B");
		BIT_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0x59:
		setInstructionString("BIT 3, C");
		BIT_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0x5A:
		setInstructionString("BIT 3, D");
		BIT_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0x5B:
		setInstructionString("BIT 3, E");
		BIT_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0x5C:
		setInstructionString("BIT 3, H");
		BIT_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0x5D:
		setInstructionString("BIT 3, L");
		BIT_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0x5E:
		setInstructionString("BIT 3, (HL)");
		BIT_indirect_HL(BitSelect::B3);
		break;

	case 0x5F:
		setInstructionString("BIT 3, A");
		BIT_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0x60:
		setInstructionString("BIT 4, B");
		BIT_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0x61:
		setInstructionString("BIT 4, C");
		BIT_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0x62:
		setInstructionString("BIT 4, D");
		BIT_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0x63:
		setInstructionString("BIT 4, E");
		BIT_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0x64:
		setInstructionString("BIT 4, H");
		BIT_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0x65:
		setInstructionString("BIT 4, L");
		BIT_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0x66:
		setInstructionString("BIT 4, (HL)");
		BIT_indirect_HL(BitSelect::B4);
		break;

	case 0x67:
		setInstructionString("BIT 4, A");
		BIT_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0x68:
		setInstructionString("BIT 5, B");
		BIT_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0x69:
		setInstructionString("BIT 5, C");
		BIT_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0x6A:
		setInstructionString("BIT 5, D");
		BIT_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0x6B:
		setInstructionString("BIT 5, E");
		BIT_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0x6C:
		setInstructionString("BIT 5, H");
		BIT_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0x6D:
		setInstructionString("BIT 5, L");
		BIT_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0x6E:
		setInstructionString("BIT 5, (HL)");
		BIT_indirect_HL(BitSelect::B5);
		break;

	case 0x6F:
		setInstructionString("BIT 5, A");
		BIT_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0x70:
		setInstructionString("BIT 6, B");
		BIT_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0x71:
		setInstructionString("BIT 6, C");
		BIT_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0x72:
		setInstructionString("BIT 6, D");
		BIT_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0x73:
		setInstructionString("BIT 6, E");
		BIT_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0x74:
		setInstructionString("BIT 6, H");
		BIT_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0x75:
		setInstructionString("BIT 6, L");
		BIT_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0x76:
		setInstructionString("BIT 6, (HL)");
		BIT_indirect_HL(BitSelect::B6);
		break;

	case 0x77:
		setInstructionString("BIT 6, A");
		BIT_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0x78:
		setInstructionString("BIT 7, B");
		BIT_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0x79:
		setInstructionString("BIT 7, C");
		BIT_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0x7A:
		setInstructionString("BIT 7, D");
		BIT_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0x7B:
		setInstructionString("BIT 7, E");
		BIT_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0x7C:
		setInstructionString("BIT 7, H");
		BIT_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0x7D:
		setInstructionString("BIT 7, L");
		BIT_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0x7E:
		setInstructionString("BIT 7, (HL)");
		BIT_indirect_HL(BitSelect::B7);
		break;

	case 0x7F:
		setInstructionString("BIT 7, A");
		BIT_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	case 0x80:
		setInstructionString("RES 0, B");
		RES_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0x81:
		setInstructionString("RES 0, C");
		RES_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0x82:
		setInstructionString("RES 0, D");
		RES_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0x83:
		setInstructionString("RES 0, E");
		RES_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0x84:
		setInstructionString("RES 0, H");
		RES_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0x85:
		setInstructionString("RES 0, L");
		RES_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0x86:
		setInstructionString("RES 0, (HL)");
		RES_indirect_HL(BitSelect::B0);
		break;

	case 0x87:
		setInstructionString("RES 0, A");
		RES_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0x88:
		setInstructionString("RES 1, B");
		RES_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0x89:
		setInstructionString("RES 1, C");
		RES_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0x8A:
		setInstructionString("RES 1, D");
		RES_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0x8B:
		setInstructionString("RES 1, E");
		RES_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0x8C:
		setInstructionString("RES 1, H");
		RES_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0x8D:
		setInstructionString("RES 1, L");
		RES_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0x8E:
		setInstructionString("RES 1, (HL)");
		RES_indirect_HL(BitSelect::B1);
		break;

	case 0x8F:
		setInstructionString("RES 1, A");
		RES_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0x90:
		setInstructionString("RES 2, B");
		RES_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0x91:
		setInstructionString("RES 2, C");
		RES_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0x92:
		setInstructionString("RES 2, D");
		RES_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0x93:
		setInstructionString("RES 2, E");
		RES_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0x94:
		setInstructionString("RES 2, H");
		RES_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0x95:
		setInstructionString("RES 2, L");
		RES_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0x96:
		setInstructionString("RES 2, (HL)");
		RES_indirect_HL(BitSelect::B2);
		break;

	case 0x97:
		setInstructionString("RES 2, A");
		RES_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0x98:
		setInstructionString("RES 3, B");
		RES_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0x99:
		setInstructionString("RES 3, C");
		RES_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0x9A:
		setInstructionString("RES 3, D");
		RES_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0x9B:
		setInstructionString("RES 3, E");
		RES_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0x9C:
		setInstructionString("RES 3, H");
		RES_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0x9D:
		setInstructionString("RES 3, L");
		RES_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0x9E:
		setInstructionString("RES 3, (HL)");
		RES_indirect_HL(BitSelect::B3);
		break;

	case 0x9F:
		setInstructionString("RES 3, A");
		RES_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0xA0:
		setInstructionString("RES 4, B");
		RES_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0xA1:
		setInstructionString("RES 4, C");
		RES_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0xA2:
		setInstructionString("RES 4, D");
		RES_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0xA3:
		setInstructionString("RES 4, E");
		RES_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0xA4:
		setInstructionString("RES 4, H");
		RES_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0xA5:
		setInstructionString("RES 4, L");
		RES_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0xA6:
		setInstructionString("RES 4, (HL)");
		RES_indirect_HL(BitSelect::B4);
		break;

	case 0xA7:
		setInstructionString("RES 4, A");
		RES_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0xA8:
		setInstructionString("RES 5, B");
		RES_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0xA9:
		setInstructionString("RES 5, C");
		RES_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0xAA:
		setInstructionString("RES 5, D");
		RES_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0xAB:
		setInstructionString("RES 5, E");
		RES_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0xAC:
		setInstructionString("RES 5, H");
		RES_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0xAD:
		setInstructionString("RES 5, L");
		RES_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0xAE:
		setInstructionString("RES 5, (HL)");
		RES_indirect_HL(BitSelect::B5);
		break;

	case 0xAF:
		setInstructionString("RES 5, A");
		RES_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0xB0:
		setInstructionString("RES 6, B");
		RES_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0xB1:
		setInstructionString("RES 6, C");
		RES_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0xB2:
		setInstructionString("RES 6, D");
		RES_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0xB3:
		setInstructionString("RES 6, E");
		RES_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0xB4:
		setInstructionString("RES 6, H");
		RES_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0xB5:
		setInstructionString("RES 6, L");
		RES_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0xB6:
		setInstructionString("RES 6, (HL)");
		RES_indirect_HL(BitSelect::B6);
		break;

	case 0xB7:
		setInstructionString("RES 6, A");
		RES_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0xB8:
		setInstructionString("RES 7, B");
		RES_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0xB9:
		setInstructionString("RES 7, C");
		RES_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0xBA:
		setInstructionString("RES 7, D");
		RES_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0xBB:
		setInstructionString("RES 7, E");
		RES_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0xBC:
		setInstructionString("RES 7, H");
		RES_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0xBD:
		setInstructionString("RES 7, L");
		RES_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0xBE:
		setInstructionString("RES 7, (HL)");
		RES_indirect_HL(BitSelect::B7);
		break;

	case 0xBF:
		setInstructionString("RES 7, A");
		RES_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	case 0xC0:
		setInstructionString("SET 0, B");
		SET_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0xC1:
		setInstructionString("SET 6, C");
		SET_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0xC2:
		setInstructionString("SET 6, D");
		SET_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0xC3:
		setInstructionString("SET 6, E");
		SET_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0xC4:
		setInstructionString("SET 6, H");
		SET_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0xC5:
		setInstructionString("SET 6, L");
		SET_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0xC6:
		setInstructionString("SET 6, (HL)");
		SET_indirect_HL(BitSelect::B0);
		break;

	case 0xC7:
		setInstructionString("SET 6, A");
		SET_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0xC8:
		setInstructionString("SET 1, B");
		SET_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0xC9:
		setInstructionString("SET 1, C");
		SET_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0xCA:
		setInstructionString("SET 1, D");
		SET_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0xCB:
		setInstructionString("SET 1, E");
		SET_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0xCC:
		setInstructionString("SET 1, H");
		SET_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0xCD:
		setInstructionString("SET 1, L");
		SET_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0xCE:
		setInstructionString("SET 1, (HL)");
		SET_indirect_HL(BitSelect::B1);
		break;

	case 0xCF:
		setInstructionString("SET 1, A");
		SET_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0xD0:
		setInstructionString("SET 0, B");
		SET_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0xD1:
		setInstructionString("SET 2, C");
		SET_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0xD2:
		setInstructionString("SET 2, D");
		SET_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0xD3:
		setInstructionString("SET 2, E");
		SET_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0xD4:
		setInstructionString("SET 2, H");
		SET_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0xD5:
		setInstructionString("SET 2, L");
		SET_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0xD6:
		setInstructionString("SET 2, (HL)");
		SET_indirect_HL(BitSelect::B2);
		break;

	case 0xD7:
		setInstructionString("SET 2, A");
		SET_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0xD8:
		setInstructionString("SET 3, B");
		SET_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0xD9:
		setInstructionString("SET 3, C");
		SET_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0xDA:
		setInstructionString("SET 3, D");
		SET_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0xDB:
		setInstructionString("SET 3, E");
		SET_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0xDC:
		setInstructionString("SET 3, H");
		SET_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0xDD:
		setInstructionString("SET 3, L");
		SET_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0xDE:
		setInstructionString("SET 3, (HL)");
		SET_indirect_HL(BitSelect::B3);
		break;

	case 0xDF:
		setInstructionString("SET 3, A");
		SET_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0xE0:
		setInstructionString("SET 0, B");
		SET_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0xE1:
		setInstructionString("SET 4, C");
		SET_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0xE2:
		setInstructionString("SET 4, D");
		SET_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0xE3:
		setInstructionString("SET 4, E");
		SET_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0xE4:
		setInstructionString("SET 4, H");
		SET_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0xE5:
		setInstructionString("SET 4, L");
		SET_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0xE6:
		setInstructionString("SET 4, (HL)");
		SET_indirect_HL(BitSelect::B4);
		break;

	case 0xE7:
		setInstructionString("SET 4, A");
		SET_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0xE8:
		setInstructionString("SET 5, B");
		SET_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0xE9:
		setInstructionString("SET 5, C");
		SET_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0xEA:
		setInstructionString("SET 5, D");
		SET_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0xEB:
		setInstructionString("SET 5, E");
		SET_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0xEC:
		setInstructionString("SET 5, H");
		SET_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0xED:
		setInstructionString("SET 5, L");
		SET_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0xEE:
		setInstructionString("SET 5, (HL)");
		SET_indirect_HL(BitSelect::B5);
		break;

	case 0xEF:
		setInstructionString("SET 5, A");
		SET_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0xF0:
		setInstructionString("SET 0, B");
		SET_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0xF1:
		setInstructionString("SET 6, C");
		SET_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0xF2:
		setInstructionString("SET 6, D");
		SET_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0xF3:
		setInstructionString("SET 6, E");
		SET_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0xF4:
		setInstructionString("SET 6, H");
		SET_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0xF5:
		setInstructionString("SET 6, L");
		SET_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0xF6:
		setInstructionString("SET 6, (HL)");
		SET_indirect_HL(BitSelect::B6);
		break;

	case 0xF7:
		setInstructionString("SET 6, A");
		SET_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0xF8:
		setInstructionString("SET 7, B");
		SET_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0xF9:
		setInstructionString("SET 7, C");
		SET_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0xFA:
		setInstructionString("SET 7, D");
		SET_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0xFB:
		setInstructionString("SET 7, E");
		SET_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0xFC:
		setInstructionString("SET 7, H");
		SET_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0xFD:
		setInstructionString("SET 7, L");
		SET_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0xFE:
		setInstructionString("SET 7, (HL)");
		SET_indirect_HL(BitSelect::B7);
		break;

	case 0xFF:
		setInstructionString("SET 7, A");
		SET_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	default:
		break;
	}
}

void Sm83::LDH_indirect_n8_A()
{
	uint16_t address = 0xFF00 + cpuFetch();
	m_bus->cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LDH_indirect_C_A()
{
	uint16_t address = 0xFF00 + m_registerBC.lo;
	m_bus->cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LDH_A_indirect_n8()
{
	uint16_t address = 0xFF00 + cpuFetch();
	m_registerAF.accumulator = m_bus->cpuRead(address);
}

void Sm83::LDH_A_indirect_C()
{
	uint16_t address = 0xFF00 + m_registerBC.lo;
	m_registerAF.accumulator = m_bus->cpuRead(address);
}

void Sm83::LD_SP_HL()
{
	mTick();
	m_stackPointer = (m_registerHL.hi << 8) | m_registerHL.lo;
}

void Sm83::LD_HL_SP_i8()
{
	uint8_t offset = cpuFetch();
	uint16_t sum = static_cast<uint16_t>(m_stackPointer + static_cast<int8_t>(offset));

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = ((m_stackPointer & 0xF) + (offset & 0xF)) >> 4;
	m_registerAF.flags.C = ((m_stackPointer & 0xFF) + offset) >> 8;

	mTick();
	m_registerHL.hi = sum >> 8;
	m_registerHL.lo = sum & 0xFF;
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
	m_bus->cpuWrite(address, src);
}

void Sm83::LD_indirect_n16_A()
{
	uint16_t address = cpuFetch() | (cpuFetch() << 8);
	m_bus->cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LD_A_indirect_n16()
{
	uint16_t address = cpuFetch() | (cpuFetch() << 8);
	m_registerAF.accumulator = m_bus->cpuRead(address);
}

void Sm83::LD_A_indirect_r16(Sm83Register &src)
{
	uint16_t address = (src.hi << 8) | src.lo;
	m_registerAF.accumulator = m_bus->cpuRead(address);
}

void Sm83::LD_indirect_HL_n8()
{
	uint8_t value = cpuFetch();
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus->cpuWrite(address, value);
}

void Sm83::LD_r8_indirect_HL(uint8_t &dest)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	dest = m_bus->cpuRead(address);
}

void Sm83::LD_indirect_HLI_A()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus->cpuWrite(address, m_registerAF.accumulator);
	address += 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_indirect_HLD_A()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus->cpuWrite(address, m_registerAF.accumulator);
	address -= 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_A_indirect_HLI()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_registerAF.accumulator = m_bus->cpuRead(address);
	address += 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_A_indirect_HLD()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_registerAF.accumulator = m_bus->cpuRead(address);
	address -= 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_indirect_n16_SP()
{
	// fetch lo byte then hi byte
	uint16_t address = cpuFetch() | (cpuFetch() << 8);

	// store lo byte the hi byte at next address
	m_bus->cpuWrite(address, m_stackPointer & 0xFF);
	m_bus->cpuWrite(address + 1, (m_stackPointer & 0xFF00) >> 8);
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
	uint8_t value = m_bus->cpuRead(address);
	INC_r8(value);
	m_bus->cpuWrite(address, value);
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
	uint8_t value = m_bus->cpuRead(address);
	DEC_r8(value);
	m_bus->cpuWrite(address, value);
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

void Sm83::RLC_r8(uint8_t &dest)
{
	m_registerAF.flags.C = (dest & 0x80) >> 7;
	dest = dest << 1;
	dest |= m_registerAF.flags.C;

	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RLC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	RLC_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::RRC_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest & 0x1;
	dest = dest >> 1;
	dest |= m_registerAF.flags.C << 7;

	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RRC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	RRC_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::RL_r8(uint8_t &dest)
{
	uint8_t shiftedValue = static_cast<uint8_t>((dest << 1) | m_registerAF.flags.C);

	m_registerAF.flags.C = (dest & 0x80) >> 7;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.Z = shiftedValue == 0;
	dest = shiftedValue;
}

void Sm83::RL_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	RL_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::RR_r8(uint8_t &dest)
{
	uint8_t shiftedValue = static_cast<uint8_t>((m_registerAF.flags.C << 7) | (dest >> 1));

	m_registerAF.flags.C = dest & 0x1;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.Z = shiftedValue == 0;
	dest = shiftedValue;
}

void Sm83::RR_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	RR_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::SLA_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest >> 7;
	dest = dest << 1;
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::SLA_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	SLA_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::SRA_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest & 0x1;
	dest = (dest & 0x80) | (dest >> 1);
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::SRA_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	SRA_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::SRL_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest & 0x1;
	dest = dest >> 1;
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::SRL_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	SRL_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::SWAP_r8(uint8_t &dest)
{
	dest = (dest >> 4) | (dest << 4);
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.C = 0;
}

void Sm83::SWAP_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	SWAP_r8(dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::ADD_SP_i8()
{
	uint8_t operand = cpuFetch();

	mTick();
	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = ((m_stackPointer & 0xF) + (operand & 0xF)) >> 4;
	m_registerAF.flags.C = ((m_stackPointer & 0xFF) + (operand)) >> 8;

	mTick();
	m_stackPointer = static_cast<uint16_t>(m_stackPointer + static_cast<int8_t>(operand));
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

void Sm83::ADD_A_r8(uint8_t &operand)
{
	uint16_t sum = m_registerAF.accumulator + operand;

	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = (((m_registerAF.accumulator & 0xF) + (operand & 0xF)) & 0x10) >> 4;
	m_registerAF.flags.C = (sum & 0x100) >> 8;
	m_registerAF.flags.Z = static_cast<uint8_t>(sum) == 0;

	m_registerAF.accumulator = static_cast<uint8_t>(sum);
}

void Sm83::ADD_A_n8()
{
	uint8_t operand = cpuFetch();
	ADD_A_r8(operand);
}

void Sm83::ADD_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	ADD_A_r8(operand);
}

void Sm83::ADC_A_r8(uint8_t &operand)
{
	uint16_t sum = static_cast<uint16_t>(m_registerAF.accumulator + operand + m_registerAF.flags.C);

	m_registerAF.flags.H = (((m_registerAF.accumulator & 0xF) + (operand & 0xF) + m_registerAF.flags.C) & 0x10) >> 4;
	m_registerAF.flags.C = (sum & 0x100) >> 8;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.Z = static_cast<uint8_t>(sum) == 0;
	m_registerAF.accumulator = static_cast<uint8_t>(sum);
}

void Sm83::ADC_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	ADC_A_r8(operand);
}

void Sm83::ADC_A_n8()
{
	uint8_t operand = cpuFetch();
	ADC_A_r8(operand);
}

void Sm83::SUB_A_r8(uint8_t &operand)
{
	// using 2's complement for subtraction, cuz why not
	uint16_t difference = m_registerAF.accumulator + static_cast<uint8_t>(~operand) + 1;

	m_registerAF.flags.H = (((m_registerAF.accumulator & 0xF) + ((~operand & 0xF) + 1)) & 0x10) >> 4;
	m_registerAF.flags.H = ~m_registerAF.flags.H;

	// carry out is inverted for subtraction when using 2's complement
	m_registerAF.flags.C = ~((difference >> 8) & 0x1);
	m_registerAF.flags.N = 1;
	m_registerAF.flags.Z = static_cast<uint8_t>(difference) == 0;

	m_registerAF.accumulator = static_cast<uint8_t>(difference);
}

void Sm83::SUB_A_n8()
{
	uint8_t operand = cpuFetch();
	SUB_A_r8(operand);
}

void Sm83::SUB_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	SUB_A_r8(operand);
}

void Sm83::SBC_A_r8(uint8_t &operand)
{
	// Interestingly the incomming carry flag is inverted on the sm83 for the SBC instruction.
	// This is in contrast to the SBC instruction on the 6502 where the programmer must
	// explicitly set the carry flag prior to starting muli-byte subtractions with SBC.
	// On the gameboy's sm83, this is done for you!
	uint16_t difference = m_registerAF.accumulator + (static_cast<uint8_t>(~operand) + !m_registerAF.flags.C);

	m_registerAF.flags.H =
		(((m_registerAF.accumulator & 0xF) + ((~operand & 0xF) + !m_registerAF.flags.C)) & 0x10) >> 4;
	m_registerAF.flags.H = ~m_registerAF.flags.H;

	m_registerAF.flags.C = !((difference >> 8) & 0x1);
	m_registerAF.flags.N = 1;
	m_registerAF.flags.Z = static_cast<uint8_t>(difference) == 0;

	m_registerAF.accumulator = static_cast<uint8_t>(difference);
}

void Sm83::SBC_A_n8()
{
	uint8_t operand = cpuFetch();
	SBC_A_r8(operand);
}

void Sm83::SBC_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	SBC_A_r8(operand);
}

void Sm83::JR_i8()
{
	int8_t offset = static_cast<int8_t>(cpuFetch());
	mTick();
	m_programCounter += offset;
}

void Sm83::JR_CC_i8(bool condition)
{
	int8_t offset = static_cast<int8_t>(cpuFetch());

	// 1 extra m-cycle on jump taken
	// 1 m-cycle == 4 t-cycles
	if (condition)
	{
		mTick();
		m_programCounter += offset;
	}
}

void Sm83::JP_CC_n16(bool condition)
{
	uint16_t address = cpuFetch() | (cpuFetch() << 8);

	if (condition)
	{
		mTick();
		m_programCounter = address;
	}
}

void Sm83::JP_n16()
{
	uint16_t address = cpuFetch() | (cpuFetch() << 8);
	mTick();
	m_programCounter = address;
}

void Sm83::JP_HL()
{
	m_programCounter = (m_registerHL.hi << 8) | m_registerHL.lo;
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

		m_registerAF.accumulator = static_cast<uint8_t>(m_registerAF.accumulator - adjustment);
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
		m_registerAF.accumulator = static_cast<uint8_t>(m_registerAF.accumulator + adjustment);
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
	m_registerAF.flags.C = ~m_registerAF.flags.C;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::AND_A_r8(uint8_t &operand)
{
	m_registerAF.accumulator &= operand;

	m_registerAF.flags.Z = m_registerAF.accumulator == 0;
	m_registerAF.flags.H = 1;
	m_registerAF.flags.C = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::AND_A_n8()
{
	uint8_t operand = cpuFetch();
	AND_A_r8(operand);
}

void Sm83::AND_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	AND_A_r8(operand);
}

void Sm83::XOR_A_r8(uint8_t &operand)
{
	m_registerAF.accumulator ^= operand;

	m_registerAF.flags.Z = m_registerAF.accumulator == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.C = 0;
}

void Sm83::XOR_A_n8()
{
	uint8_t operand = cpuFetch();
	XOR_A_r8(operand);
}

void Sm83::XOR_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	XOR_A_r8(operand);
}

void Sm83::OR_A_r8(uint8_t &operand)
{
	m_registerAF.accumulator |= operand;

	m_registerAF.flags.Z = m_registerAF.accumulator == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.C = 0;
}

void Sm83::OR_A_n8()
{
	uint8_t operand = cpuFetch();
	OR_A_r8(operand);
}

void Sm83::OR_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	OR_A_r8(operand);
}

void Sm83::CP_A_r8(uint8_t &operand)
{
	uint16_t difference = m_registerAF.accumulator + (static_cast<uint8_t>(~operand) + 1);

	m_registerAF.flags.Z = static_cast<uint8_t>(difference) == 0;
	m_registerAF.flags.N = 1;
	m_registerAF.flags.H = (((m_registerAF.accumulator & 0xF) + ((~operand & 0xF) + 1)) & 0x10) >> 4;
	m_registerAF.flags.H = ~m_registerAF.flags.H;
	m_registerAF.flags.C = !((difference & 0x100) >> 8);
}

void Sm83::CP_A_n8()
{
	uint8_t operand = cpuFetch();
	CP_A_r8(operand);
}

void Sm83::CP_A_indirect_HL()
{
	uint8_t operand = m_bus->cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	CP_A_r8(operand);
}

void Sm83::RET()
{
	uint8_t lo = m_bus->cpuRead(m_stackPointer++);
	uint8_t hi = m_bus->cpuRead(m_stackPointer++);

	mTick();
	m_programCounter = (hi << 8) | lo;
}

void Sm83::RET_CC(bool condition)
{
	mTick();
	if (condition)
		RET();
}

void Sm83::RETI()
{
	RET();
	m_ime = true;
}

void Sm83::POP_r16(Sm83Register &dest)
{
	uint8_t lo = m_bus->cpuRead(m_stackPointer++);
	uint8_t hi = m_bus->cpuRead(m_stackPointer++);
	dest.lo = lo;
	dest.hi = hi;
}

void Sm83::POP_AF()
{
	uint8_t lo = m_bus->cpuRead(m_stackPointer++);
	uint8_t hi = m_bus->cpuRead(m_stackPointer++);

	m_registerAF.accumulator = hi;
	m_registerAF.flags.setFlagsU8(lo);
}

void Sm83::PUSH_r16(Sm83Register &dest)
{
	mTick();
	m_bus->cpuWrite(--m_stackPointer, dest.hi);
	m_bus->cpuWrite(--m_stackPointer, dest.lo);
}

void Sm83::PUSH_AF()
{
	mTick();
	m_bus->cpuWrite(--m_stackPointer, m_registerAF.accumulator);
	m_bus->cpuWrite(--m_stackPointer, m_registerAF.flags.getFlagsU8());
}

void Sm83::CALL_n16()
{
	uint16_t callAddress = cpuFetch() | (cpuFetch() << 8);

	mTick();

	// save hi byte of pc to stack followed by the lo byte
	m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
	m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

	m_programCounter = callAddress;
}

void Sm83::CALL_CC_n16(bool condition)
{
	uint16_t callAddress = cpuFetch() | (cpuFetch() << 8);

	if (condition)
	{
		mTick();

		// save hi byte of pc to stack followed by the lo byte
		m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
		m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

		m_programCounter = callAddress;
	}
}

void Sm83::RST(RstVector vec)
{
	mTick();
	m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
	m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

	m_programCounter = static_cast<uint16_t>(vec);
}

void Sm83::BIT_r8(BitSelect b, uint8_t dest)
{
	m_registerAF.flags.Z = (dest & static_cast<uint8_t>(b)) == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 1;
}

void Sm83::BIT_indirect_HL(BitSelect b)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	BIT_r8(b, dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::RES_r8(BitSelect b, uint8_t &dest)
{
	dest = dest & ~static_cast<uint8_t>(b);
}

void Sm83::RES_indirect_HL(BitSelect b)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	RES_r8(b, dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::SET_r8(BitSelect b, uint8_t &dest)
{
	dest = dest | static_cast<uint8_t>(b);
}

void Sm83::SET_indirect_HL(BitSelect b)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t dest = m_bus->cpuRead(address);
	SET_r8(b, dest);
	m_bus->cpuWrite(address, dest);
}

void Sm83::DI()
{
	m_ime = false;
}

void Sm83::EI()
{

}
