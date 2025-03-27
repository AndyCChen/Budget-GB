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

	m_ime = false;

	m_logEnable = false;
}

void Sm83::runInstruction()
{
	uint8_t opcode = cpuFetch();
	decodeExecute(opcode);

	if (m_logEnable)
		fmt::print("{}\n", m_instructionString);

	m_instructionString.clear();
}

void Sm83::decodeExecute(uint8_t opcode)
{
	switch (opcode)
	{

	case 0x00:
		formatToOpcode("NOP");
		break;

	case 0x01:
		formatToOpcode("LD BC, ");
		LD_r16_n16(m_registerBC);
		break;

	case 0x02:
		formatToOpcode("LD (BC), A");
		LD_indirect_r16_r8(m_registerBC, m_registerAF.accumulator);
		break;

	case 0x03:
		formatToOpcode("INC BC");
		INC_r16(m_registerBC);
		break;

	case 0x04:
		formatToOpcode("INC B");
		INC_r8(m_registerBC.hi);
		break;

	case 0x05:
		formatToOpcode("DEC B");
		DEC_r8(m_registerBC.hi);
		break;

	case 0x06:
		formatToOpcode("LD B, ");
		LD_r8_n8(m_registerBC.hi);
		break;

	case 0x07:
		formatToOpcode("RLCA");
		RLCA();
		break;

	case 0x08:
		formatToOpcode("LD ");
		LD_indirect_n16_SP();
		break;

	case 0x09:
		formatToOpcode("ADD HL, BC");
		ADD_HL_r16(m_registerBC);
		break;

	case 0x0A:
		formatToOpcode("LD A, (BC)");
		LD_A_indirect_r16(m_registerBC);
		break;

	case 0x0B:
		formatToOpcode("DEC BC");
		DEC_r16(m_registerBC);
		break;

	case 0x0C:
		formatToOpcode("INC C");
		INC_r8(m_registerBC.lo);
		break;

	case 0x0D:
		formatToOpcode("DEC C");
		DEC_r8(m_registerBC.lo);
		break;

	case 0x0E:
		formatToOpcode("LD C, ");
		LD_r8_n8(m_registerBC.lo);
		break;

	case 0x0F:
		formatToOpcode("RRCA");
		RRCA();
		break;

	case 0x10:
		formatToOpcode("STOP");
		break;

	case 0x11:
		formatToOpcode("LD DE, ");
		LD_r16_n16(m_registerDE);
		break;

	case 0x12:
		formatToOpcode("LD (DE), A");
		LD_indirect_r16_r8(m_registerDE, m_registerAF.accumulator);
		break;

	case 0x13:
		formatToOpcode("INC DE");
		INC_r16(m_registerDE);
		break;

	case 0x14:
		formatToOpcode("INC D");
		INC_r8(m_registerDE.hi);
		break;

	case 0x15:
		formatToOpcode("DEC D");
		DEC_r8(m_registerDE.hi);
		break;

	case 0x16:
		formatToOpcode("LD, D, ");
		LD_r8_n8(m_registerDE.hi);
		break;

	case 0x17:
		formatToOpcode("RLA");
		RLA();
		break;

	case 0x18:
		formatToOpcode("JR ");
		JR_i8();
		break;

	case 0x19:
		formatToOpcode("ADD HL, DE");
		ADD_HL_r16(m_registerDE);
		break;

	case 0x1A:
		formatToOpcode("LD A, (DE)");
		LD_A_indirect_r16(m_registerDE);
		break;

	case 0x1B:
		formatToOpcode("DEC DE");
		DEC_r16(m_registerDE);
		break;

	case 0x1C:
		formatToOpcode("INC E");
		INC_r8(m_registerDE.lo);
		break;

	case 0x1D:
		formatToOpcode("DEC E");
		DEC_r8(m_registerDE.lo);
		break;

	case 0x1E:
		formatToOpcode("LD E, ");
		LD_r8_n8(m_registerDE.lo);
		break;

	case 0x1F:
		formatToOpcode("RRA");
		RRA();
		break;

	case 0x20:
		formatToOpcode("JR NZ, ");
		JR_CC_i8(!m_registerAF.flags.Z);
		break;

	case 0x21:
		formatToOpcode("LD HL, ");
		LD_r16_n16(m_registerHL);
		break;

	case 0x22:
		formatToOpcode("LD (HL+), A");
		LD_indirect_HLI_A();
		break;

	case 0x23:
		formatToOpcode("INC HL");
		INC_r16(m_registerHL);
		break;

	case 0x24:
		formatToOpcode("INC H");
		INC_r8(m_registerHL.hi);
		break;

	case 0x25:
		formatToOpcode("DEC H");
		DEC_r8(m_registerHL.hi);
		break;

	case 0x26:
		formatToOpcode("LD H, ");
		LD_r8_n8(m_registerHL.hi);
		break;

	case 0x27:
		formatToOpcode("DAA");
		DAA();
		break;

	case 0x28:
		formatToOpcode("JR Z, ");
		JR_CC_i8(m_registerAF.flags.Z);
		break;

	case 0x29:
		formatToOpcode("ADD HL, HL");
		ADD_HL_r16(m_registerHL);
		break;

	case 0x2A:
		formatToOpcode("LD A, (HL+)");
		LD_A_indirect_HLI();
		break;

	case 0x2B:
		formatToOpcode("DEC HL");
		DEC_r16(m_registerHL);
		break;

	case 0x2C:
		formatToOpcode("INC L");
		INC_r8(m_registerHL.lo);
		break;

	case 0x2D:
		formatToOpcode("DEC L");
		DEC_r8(m_registerHL.lo);
		break;

	case 0x2E:
		formatToOpcode("LD L, ");
		LD_r8_n8(m_registerHL.lo);
		break;

	case 0x2F:
		formatToOpcode("CPL");
		CPL();
		break;

	case 0x30:
		formatToOpcode("JR NC, ");
		JR_CC_i8(!m_registerAF.flags.C);
		break;

	case 0x31:
		formatToOpcode("LD SP, ");
		LD_SP_n16();
		break;

	case 0x32:
		formatToOpcode("LD (HL-), A");
		LD_indirect_HLD_A();
		break;

	case 0x33:
		formatToOpcode("INC SP");
		INC_SP();
		break;

	case 0x34:
		formatToOpcode("INC (HL)");
		INC_indirect_HL();
		break;

	case 0x35:
		formatToOpcode("DEC (HL)");
		DEC_indirect_HL();
		break;

	case 0x36:
		formatToOpcode("LD (HL), ");
		LD_indirect_HL_n8();
		break;

	case 0x37:
		formatToOpcode("SCF");
		SCF();
		break;

	case 0x38:
		formatToOpcode("JR C, ");
		JR_CC_i8(m_registerAF.flags.C);
		break;

	case 0x39:
		formatToOpcode("ADD HL, SP");
		ADD_HL_SP();
		break;

	case 0x3A:
		formatToOpcode("LD A, HLD");
		LD_A_indirect_HLD();
		break;

	case 0x3B:
		formatToOpcode("DEC SP");
		DEC_SP();
		break;

	case 0x3C:
		formatToOpcode("INC A");
		INC_r8(m_registerAF.accumulator);
		break;

	case 0x3D:
		formatToOpcode("DEC A");
		DEC_r8(m_registerAF.accumulator);
		break;

	case 0x3E:
		formatToOpcode("LD A, ");
		LD_r8_n8(m_registerAF.accumulator);
		break;

	case 0x3F:
		formatToOpcode("CCF");
		CCF();
		break;

	case 0x40:
		formatToOpcode("LD B, B");
		LD_r8_r8(m_registerBC.hi, m_registerBC.hi);
		break;

	case 0x41:
		formatToOpcode("LD B, C");
		LD_r8_r8(m_registerBC.hi, m_registerBC.lo);
		break;

	case 0x42:
		formatToOpcode("LD B, D");
		LD_r8_r8(m_registerBC.hi, m_registerDE.hi);
		break;

	case 0x43:
		formatToOpcode("LD B, E");
		LD_r8_r8(m_registerBC.hi, m_registerDE.lo);
		break;

	case 0x44:
		formatToOpcode("LD B, H");
		LD_r8_r8(m_registerBC.hi, m_registerHL.hi);
		break;

	case 0x45:
		formatToOpcode("LD B, L");
		LD_r8_r8(m_registerBC.hi, m_registerHL.lo);
		break;

	case 0x46:
		formatToOpcode("LD B, (HL)");
		LD_r8_indirect_HL(m_registerBC.hi);
		break;

	case 0x47:
		formatToOpcode("LD B, A");
		LD_r8_r8(m_registerBC.hi, m_registerAF.accumulator);
		break;

	case 0x48:
		formatToOpcode("LD C, B");
		LD_r8_r8(m_registerBC.lo, m_registerBC.hi);
		break;

	case 0x49:
		formatToOpcode("LD C, C");
		LD_r8_r8(m_registerBC.lo, m_registerBC.lo);
		break;

	case 0x4A:
		formatToOpcode("LD C, D");
		LD_r8_r8(m_registerBC.lo, m_registerDE.hi);
		break;

	case 0x4B:
		formatToOpcode("LD C, E");
		LD_r8_r8(m_registerBC.lo, m_registerDE.lo);
		break;

	case 0x4C:
		formatToOpcode("LD C, H");
		LD_r8_r8(m_registerBC.lo, m_registerHL.hi);
		break;

	case 0x4D:
		formatToOpcode("LD C, L");
		LD_r8_r8(m_registerBC.lo, m_registerHL.lo);
		break;

	case 0x4E:
		formatToOpcode("LD C, (HL)");
		LD_r8_indirect_HL(m_registerBC.lo);
		break;

	case 0x4F:
		formatToOpcode("LD C, A");
		LD_r8_r8(m_registerBC.lo, m_registerAF.accumulator);
		break;

	case 0x50:
		formatToOpcode("LD D, B");
		LD_r8_r8(m_registerDE.hi, m_registerBC.hi);
		break;

	case 0x51:
		formatToOpcode("LD D, C");
		LD_r8_r8(m_registerDE.hi, m_registerBC.lo);
		break;
	case 0x52:
		formatToOpcode("LD D, D");
		LD_r8_r8(m_registerDE.hi, m_registerDE.hi);
		break;

	case 0x53:
		formatToOpcode("LD D, E");
		LD_r8_r8(m_registerDE.hi, m_registerDE.lo);
		break;

	case 0x54:
		formatToOpcode("LD D, H");
		LD_r8_r8(m_registerDE.hi, m_registerHL.hi);
		break;

	case 0x55:
		formatToOpcode("LD D, L");
		LD_r8_r8(m_registerDE.hi, m_registerHL.lo);
		break;
	case 0x56:
		formatToOpcode("LD D, (HL)");
		LD_r8_indirect_HL(m_registerDE.hi);
		break;

	case 0x57:
		formatToOpcode("LD D, A");
		LD_r8_r8(m_registerDE.hi, m_registerAF.accumulator);
		break;

	case 0x58:
		formatToOpcode("LD E, B");
		LD_r8_r8(m_registerDE.lo, m_registerBC.hi);
		break;
	case 0x59:
		formatToOpcode("LD E, C");
		LD_r8_r8(m_registerDE.lo, m_registerBC.lo);
		break;

	case 0x5A:
		formatToOpcode("LD E, D");
		LD_r8_r8(m_registerDE.lo, m_registerDE.hi);
		break;
	case 0x5B:
		formatToOpcode("LD E, E");
		LD_r8_r8(m_registerDE.lo, m_registerDE.lo);
		break;

	case 0x5C:
		formatToOpcode("LD E, H");
		LD_r8_r8(m_registerDE.lo, m_registerHL.hi);
		break;

	case 0x5D:
		formatToOpcode("LD E, L");
		LD_r8_r8(m_registerDE.lo, m_registerHL.lo);
		break;

	case 0x5E:
		formatToOpcode("LD E, (HL)");
		LD_r8_indirect_HL(m_registerDE.lo);
		break;

	case 0x5F:
		formatToOpcode("LD E, A");
		LD_r8_r8(m_registerDE.lo, m_registerAF.accumulator);
		break;

	case 0x60:
		formatToOpcode("LD H, B");
		LD_r8_r8(m_registerHL.hi, m_registerBC.hi);
		break;

	case 0x61:
		formatToOpcode("LD H, C");
		LD_r8_r8(m_registerHL.hi, m_registerBC.lo);
		break;

	case 0x62:
		formatToOpcode("LD H, D");
		LD_r8_r8(m_registerHL.hi, m_registerDE.hi);
		break;

	case 0x63:
		formatToOpcode("LD H, E");
		LD_r8_r8(m_registerHL.hi, m_registerDE.lo);
		break;
	case 0x64:

		formatToOpcode("LD H, H");
		LD_r8_r8(m_registerHL.hi, m_registerHL.hi);
		break;

	case 0x65:
		formatToOpcode("LD H, L");
		LD_r8_r8(m_registerHL.hi, m_registerHL.lo);
		break;

	case 0x66:
		formatToOpcode("LD H, (HL)");
		LD_r8_indirect_HL(m_registerHL.hi);
		break;

	case 0x67:
		formatToOpcode("LD H, A");
		LD_r8_r8(m_registerHL.hi, m_registerAF.accumulator);
		break;

	case 0x68:
		formatToOpcode("LD L, B");
		LD_r8_r8(m_registerHL.lo, m_registerBC.hi);
		break;

	case 0x69:
		formatToOpcode("LD L, C");
		LD_r8_r8(m_registerHL.lo, m_registerBC.lo);
		break;

	case 0x6A:
		formatToOpcode("LD L, D");
		LD_r8_r8(m_registerHL.lo, m_registerDE.hi);
		break;

	case 0x6B:
		formatToOpcode("LD L, E");
		LD_r8_r8(m_registerHL.lo, m_registerDE.lo);
		break;

	case 0x6C:
		formatToOpcode("LD L, H");
		LD_r8_r8(m_registerHL.lo, m_registerHL.hi);
		break;

	case 0x6D:
		formatToOpcode("LD L, L");
		LD_r8_r8(m_registerHL.lo, m_registerHL.lo);
		break;

	case 0x6E:
		formatToOpcode("LD L, (HL)");
		LD_r8_indirect_HL(m_registerHL.lo);
		break;

	case 0x6F:
		formatToOpcode("LD L, A");
		LD_r8_r8(m_registerHL.lo, m_registerAF.accumulator);
		break;

	case 0x70:
		formatToOpcode("LD (HL), B");
		LD_indirect_r16_r8(m_registerHL, m_registerBC.hi);
		break;

	case 0x71:
		formatToOpcode("LD (HL), C");
		LD_indirect_r16_r8(m_registerHL, m_registerBC.lo);
		break;

	case 0x72:
		formatToOpcode("LD (HL), D");
		LD_indirect_r16_r8(m_registerHL, m_registerDE.hi);
		break;

	case 0x73:
		formatToOpcode("LD (HL), E");
		LD_indirect_r16_r8(m_registerHL, m_registerDE.lo);
		break;

	case 0x74:
		formatToOpcode("LD (HL), H");
		LD_indirect_r16_r8(m_registerHL, m_registerHL.hi);
		break;

	case 0x75:
		formatToOpcode("LD (HL), L");
		LD_indirect_r16_r8(m_registerHL, m_registerHL.lo);
		break;

	case 0x76:
		formatToOpcode("HALT");
		break;

	case 0x77:
		formatToOpcode("LD (HL), A");
		LD_indirect_r16_r8(m_registerHL, m_registerAF.accumulator);
		break;

	case 0x78:
		formatToOpcode("LD A, B");
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.hi);
		break;

	case 0x79:
		formatToOpcode("LD A, C");
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.lo);
		break;

	case 0x7A:
		formatToOpcode("LD A, D");
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.hi);
		break;

	case 0x7B:
		formatToOpcode("LD A, E");
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.lo);
		break;

	case 0x7C:
		formatToOpcode("LD A, H");
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.hi);
		break;

	case 0x7D:
		formatToOpcode("LD A,L");
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.lo);
		break;

	case 0x7E:
		formatToOpcode("LD A, (HL)");
		LD_r8_indirect_HL(m_registerAF.accumulator);
		break;

	case 0x7F:
		formatToOpcode("LD A, A");
		LD_r8_r8(m_registerAF.accumulator, m_registerAF.accumulator);
		break;

	case 0x80:
		formatToOpcode("ADD A, B");
		ADD_A_r8(m_registerBC.hi);
		break;

	case 0x81:
		formatToOpcode("ADD A, C");
		ADD_A_r8(m_registerBC.lo);
		break;

	case 0x82:
		formatToOpcode("ADD A, D");
		ADD_A_r8(m_registerDE.hi);
		break;

	case 0x83:
		formatToOpcode("ADD A, E");
		ADD_A_r8(m_registerDE.lo);
		break;

	case 0x84:
		formatToOpcode("ADD A, H");
		ADD_A_r8(m_registerHL.hi);
		break;

	case 0x85:
		formatToOpcode("ADD A, L");
		ADD_A_r8(m_registerHL.lo);
		break;

	case 0x86:
		formatToOpcode("ADD A, (HL)");
		ADD_A_indirect_HL();
		break;

	case 0x87:
		formatToOpcode("ADD A, A");
		ADD_A_r8(m_registerAF.accumulator);
		break;

	case 0x88:
		formatToOpcode("ADC A, B");
		ADC_A_r8(m_registerBC.hi);
		break;

	case 0x89:
		formatToOpcode("ADC A, C");
		ADC_A_r8(m_registerBC.lo);
		break;
	case 0x8A:

		formatToOpcode("ADC A, D");
		ADC_A_r8(m_registerDE.hi);
		break;

	case 0x8B:
		formatToOpcode("ADC A, E");
		ADC_A_r8(m_registerDE.lo);
		break;

	case 0x8C:
		formatToOpcode("ADC A, H");
		ADC_A_r8(m_registerHL.hi);
		break;

	case 0x8D:
		formatToOpcode("ADC A, L");
		ADC_A_r8(m_registerHL.lo);
		break;

	case 0x8E:
		formatToOpcode("ADC A, (HL)");
		ADC_A_indirect_HL();
		break;

	case 0x8F:
		formatToOpcode("ADC A, A");
		ADC_A_r8(m_registerAF.accumulator);
		break;

	case 0x90:
		formatToOpcode("SUB A, B");
		SUB_A_r8(m_registerBC.hi);
		break;

	case 0x91:
		formatToOpcode("SUB A, C");
		SUB_A_r8(m_registerBC.lo);
		break;

	case 0x92:
		formatToOpcode("SUB A, D");
		SUB_A_r8(m_registerDE.hi);
		break;
	case 0x93:

		formatToOpcode("SUB A, E");
		SUB_A_r8(m_registerDE.lo);
		break;

	case 0x94:
		formatToOpcode("SUB A, H");
		SUB_A_r8(m_registerHL.hi);
		break;

	case 0x95:
		formatToOpcode("SUB A, L");
		SUB_A_r8(m_registerHL.lo);
		break;

	case 0x96:
		formatToOpcode("SUB A, (HL)");
		SUB_A_indirect_HL();
		break;

	case 0x97:
		formatToOpcode("SUB A, A");
		SUB_A_r8(m_registerAF.accumulator);
		break;

	case 0x98:
		formatToOpcode("SBC A, B");
		SBC_A_r8(m_registerBC.hi);
		break;

	case 0x99:
		formatToOpcode("SBC A, C");
		SBC_A_r8(m_registerBC.lo);
		break;

	case 0x9A:
		formatToOpcode("SBC A, D");
		SBC_A_r8(m_registerDE.hi);
		break;

	case 0x9B:
		formatToOpcode("SBC A, E");
		SBC_A_r8(m_registerDE.lo);
		break;

	case 0x9C:
		formatToOpcode("SBC A, H");
		SBC_A_r8(m_registerHL.hi);
		break;

	case 0x9D:
		formatToOpcode("SBC A, L");
		SBC_A_r8(m_registerHL.lo);
		break;

	case 0x9E:
		formatToOpcode("SBC A, (HL)");
		SBC_A_indirect_HL();
		break;

	case 0x9F:
		formatToOpcode("SBC A, A");
		SBC_A_r8(m_registerAF.accumulator);
		break;

	case 0xA0:
		formatToOpcode("AND A, B");
		AND_A_r8(m_registerBC.hi);
		break;

	case 0xA1:
		formatToOpcode("AND A, C");
		AND_A_r8(m_registerBC.lo);
		break;

	case 0xA2:
		formatToOpcode("AND A, D");
		AND_A_r8(m_registerDE.hi);
		break;

	case 0xA3:
		formatToOpcode("AND A, E");
		AND_A_r8(m_registerDE.lo);
		break;

	case 0xA4:
		formatToOpcode("AND A, H");
		AND_A_r8(m_registerHL.hi);
		break;

	case 0xA5:
		formatToOpcode("AND A, L");
		AND_A_r8(m_registerHL.lo);
		break;

	case 0xA6:
		formatToOpcode("AND A, (HL)");
		AND_A_indirect_HL();
		break;

	case 0xA7:
		formatToOpcode("AND A, A");
		AND_A_r8(m_registerAF.accumulator);
		break;

	case 0xA8:
		formatToOpcode("XOR A, B");
		XOR_A_r8(m_registerBC.hi);
		break;

	case 0xA9:
		formatToOpcode("XOR A, C");
		XOR_A_r8(m_registerBC.lo);
		break;

	case 0xAA:
		formatToOpcode("XOR A, D");
		XOR_A_r8(m_registerDE.hi);
		break;

	case 0xAB:
		formatToOpcode("XOR A, E");
		XOR_A_r8(m_registerDE.lo);
		break;

	case 0xAC:
		formatToOpcode("XOR A, H");
		XOR_A_r8(m_registerHL.hi);
		break;

	case 0xAD:
		formatToOpcode("XOR A, L");
		XOR_A_r8(m_registerHL.lo);
		break;

	case 0xAE:
		formatToOpcode("XOR A, (HL)");
		XOR_A_indirect_HL();
		break;

	case 0xAF:
		formatToOpcode("XOR A, A");
		XOR_A_r8(m_registerAF.accumulator);
		break;

	case 0xB0:
		formatToOpcode("OR A, B");
		OR_A_r8(m_registerBC.hi);
		break;

	case 0xB1:
		formatToOpcode("OR A, C");
		OR_A_r8(m_registerBC.lo);
		break;

	case 0xB2:
		formatToOpcode("OR A, D");
		OR_A_r8(m_registerDE.hi);
		break;

	case 0xB3:
		formatToOpcode("OR A, E");
		OR_A_r8(m_registerDE.lo);
		break;

	case 0xB4:
		formatToOpcode("OR A, H");
		OR_A_r8(m_registerHL.hi);
		break;

	case 0xB5:
		formatToOpcode("OR A, L");
		OR_A_r8(m_registerHL.lo);
		break;

	case 0xB6:
		formatToOpcode("OR A, (HL)");
		OR_A_indirect_HL();
		break;

	case 0xB7:
		formatToOpcode("OR A, A");
		OR_A_r8(m_registerAF.accumulator);
		break;

	case 0xB8:
		formatToOpcode("CP A, B");
		CP_A_r8(m_registerBC.hi);
		break;

	case 0xB9:
		formatToOpcode("CP A, C");
		CP_A_r8(m_registerBC.lo);
		break;

	case 0xBA:
		formatToOpcode("CP A, D");
		CP_A_r8(m_registerDE.hi);
		break;

	case 0xBB:
		formatToOpcode("CP A, E");
		CP_A_r8(m_registerDE.lo);
		break;

	case 0xBC:
		formatToOpcode("CP A, H");
		CP_A_r8(m_registerHL.hi);
		break;

	case 0xBD:
		formatToOpcode("CP A, L");
		CP_A_r8(m_registerHL.lo);
		break;

	case 0xBE:
		formatToOpcode("CP A, (HL)");
		CP_A_indirect_HL();
		break;

	case 0xBF:
		formatToOpcode("CP A, A");
		CP_A_r8(m_registerAF.accumulator);
		break;

	case 0xC0:
		formatToOpcode("RET NZ");
		RET_CC(!m_registerAF.flags.Z);
		break;

	case 0xC1:
		formatToOpcode("POP BC");
		POP_r16(m_registerBC);
		break;

	case 0xC2:
		formatToOpcode("JP NZ, ");
		JP_CC_n16(!m_registerAF.flags.Z);
		break;

	case 0xC3:
		formatToOpcode("JP ");
		JP_n16();
		break;

	case 0xC4:
		formatToOpcode("CALL NZ, ");
		CALL_CC_n16(!m_registerAF.flags.Z);
		break;

	case 0xC5:
		formatToOpcode("PUSH BC");
		PUSH_r16(m_registerBC);
		break;

	case 0xC6:
		formatToOpcode("ADD A ");
		ADD_A_n8();
		break;

	case 0xC7:
		formatToOpcode("RST 00h");
		RST(RstVector::H00);
		break;

	case 0xC8:
		formatToOpcode("RET Z");
		RET_CC(m_registerAF.flags.Z);
		break;

	case 0xC9:
		formatToOpcode("RET");
		RET();
		break;

	case 0xCA:
		formatToOpcode("JP Z, ");
		JP_CC_n16(m_registerAF.flags.Z);
		break;

	// Prefix mode, decode opcode with second opcode table
	case 0xCB:
		decodeExecutePrefixedMode(cpuFetch());
		break;

	case 0xCC:
		formatToOpcode("CALL Z, ");
		CALL_CC_n16(m_registerAF.flags.Z);
		break;

	case 0xCD:
		formatToOpcode("CALL ");
		CALL_n16();
		break;

	case 0xCE:
		formatToOpcode("ADC A, ");
		ADC_A_n8();
		break;

	case 0xCF:
		formatToOpcode("RST 08h");
		RST(RstVector::H08);
		break;

	case 0xD0:
		formatToOpcode("RET NC");
		RET_CC(!m_registerAF.flags.C);
		break;

	case 0xD1:
		formatToOpcode("POP DE");
		POP_r16(m_registerDE);
		break;

	case 0xD2:
		formatToOpcode("JP NC, ");
		JP_CC_n16(!m_registerAF.flags.C);
		break;

	case 0xD3:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xD4:
		formatToOpcode("CALL NC, ");
		CALL_CC_n16(!m_registerAF.flags.C);
		break;

	case 0xD5:
		formatToOpcode("PUSH DE");
		PUSH_r16(m_registerDE);
		break;

	case 0xD6:
		formatToOpcode("SUB A, ");
		SUB_A_n8();
		break;

	case 0xD7:
		formatToOpcode("RST 10h");
		RST(RstVector::H10);
		break;

	case 0xD8:
		formatToOpcode("RET C");
		RET_CC(m_registerAF.flags.C);
		break;

	case 0xD9:
		formatToOpcode("RETI");
		RETI();
		break;

	case 0xDA:
		formatToOpcode("JP C, ");
		JP_CC_n16(m_registerAF.flags.C);
		break;

	case 0xDB:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xDC:
		formatToOpcode("CALL C, ");
		CALL_CC_n16(m_registerAF.flags.C);
		break;

	case 0xDD:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xDE:
		formatToOpcode("SBC A, ");
		SBC_A_n8();
		break;

	case 0xDF:
		formatToOpcode("RST 18h");
		RST(RstVector::H18);
		break;

	case 0xE0:
		formatToOpcode("LD ");
		LDH_indirect_n8_A();
		break;

	case 0xE1:
		formatToOpcode("POP HL");
		POP_r16(m_registerHL);
		break;

	case 0xE2:
		formatToOpcode("LD $(FF00 + C), A");
		LDH_indirect_C_A();
		break;

	case 0xE3:
	case 0xE4:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xE5:
		formatToOpcode("PUSH HL");
		PUSH_r16(m_registerHL);
		break;

	case 0xE6:
		formatToOpcode("AND A, ");
		AND_A_n8();
		break;

	case 0xE7:
		formatToOpcode("RST 20h");
		RST(RstVector::H20);
		break;

	case 0xE8:
		formatToOpcode("ADD SP, ");
		ADD_SP_i8();
		break;

	case 0xE9:
		formatToOpcode("JP HL");
		JP_HL();
		break;

	case 0xEA:
		formatToOpcode("LD ");
		LD_indirect_n16_A();
		break;

	case 0xEB:
	case 0xEC:
	case 0xED:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xEE:
		formatToOpcode("XOR A, ");
		XOR_A_n8();
		break;

	case 0xEF:
		formatToOpcode("RST 28h");
		RST(RstVector::H28);
		break;

	case 0xF0:
		formatToOpcode("LD A, ");
		LDH_A_indirect_n8();
		break;

	case 0xF1:
		formatToOpcode("POP AF");
		POP_AF();
		break;

	case 0xF2:
		formatToOpcode("LD A, (FF00 + C)");
		LDH_A_indirect_C();
		break;

	case 0xF3:
		formatToOpcode("DI");
		DI();
		break;

	case 0xF4:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xF5:
		formatToOpcode("PUSH AF");
		PUSH_AF();
		break;

	case 0xF6:
		formatToOpcode("OR A, ");
		OR_A_n8();
		break;

	case 0xF7:
		formatToOpcode("RST 30h");
		RST(RstVector::H30);
		break;

	case 0xF8:
		formatToOpcode("LD HL, SP + i8");
		LD_HL_SP_i8();
		break;

	case 0xF9:
		formatToOpcode("LD SP, HL");
		LD_SP_HL();
		break;

	case 0xFA:
		formatToOpcode("LD A, ");
		LD_A_indirect_n16();
		break;

	case 0xFB:
		formatToOpcode("EI");
		EI();
		break;

	case 0xFC:
	case 0xFD:
		formatToOpcode("Illegal opcode!");
		m_programCounter -= 1;
		break;

	case 0xFE:
		formatToOpcode("CP A, ");
		CP_A_n8();
		break;

	case 0xFF:
		formatToOpcode("RST 38h");
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
		formatToOpcode("RLC B");
		RLC_r8(m_registerBC.hi);
		break;

	case 0x01:
		formatToOpcode("RLC C");
		RLC_r8(m_registerBC.lo);
		break;

	case 0x02:
		formatToOpcode("RLC D");
		RLC_r8(m_registerDE.hi);
		break;

	case 0x03:
		formatToOpcode("RLC E");
		RLC_r8(m_registerDE.lo);
		break;

	case 0x04:
		formatToOpcode("RLC H");
		RLC_r8(m_registerHL.hi);
		break;
	case 0x05:

		formatToOpcode("RLC L");
		RLC_r8(m_registerHL.lo);
		break;

	case 0x06:
		formatToOpcode("RLC (HL)");
		RLC_indirect_HL();
		break;

	case 0x07:
		formatToOpcode("RLC A");
		RLC_r8(m_registerAF.accumulator);
		break;

	case 0x08:
		formatToOpcode("RRC B");
		RRC_r8(m_registerBC.hi);
		break;

	case 0x09:
		formatToOpcode("RRC C");
		RRC_r8(m_registerBC.lo);
		break;

	case 0x0A:
		formatToOpcode("RRC D");
		RRC_r8(m_registerDE.hi);
		break;

	case 0x0B:
		formatToOpcode("RRC E");
		RRC_r8(m_registerDE.lo);
		break;

	case 0x0C:
		formatToOpcode("RRC H");
		RRC_r8(m_registerHL.hi);
		break;

	case 0x0D:
		formatToOpcode("RRC L");
		RRC_r8(m_registerHL.lo);
		break;

	case 0x0E:
		formatToOpcode("RRC (HL)");
		RRC_indirect_HL();
		break;

	case 0x0F:
		formatToOpcode("RRC A");
		RRC_r8(m_registerAF.accumulator);
		break;

	case 0x10:
		formatToOpcode("RL B");
		RL_r8(m_registerBC.hi);
		break;

	case 0x11:
		formatToOpcode("RL C");
		RL_r8(m_registerBC.lo);
		break;

	case 0x12:
		formatToOpcode("RL D");
		RL_r8(m_registerDE.hi);
		break;

	case 0x13:
		formatToOpcode("RL E");
		RL_r8(m_registerDE.lo);
		break;
	case 0x14:

		formatToOpcode("RL H");
		RL_r8(m_registerHL.hi);
		break;
	case 0x15:

		formatToOpcode("RL L");
		RL_r8(m_registerHL.lo);
		break;

	case 0x16:
		formatToOpcode("RL (HL)");
		RL_indirect_HL();
		break;

	case 0x17:
		formatToOpcode("RL A");
		RL_r8(m_registerAF.accumulator);
		break;

	case 0x18:
		formatToOpcode("RR B");
		RR_r8(m_registerBC.hi);
		break;

	case 0x19:
		formatToOpcode("RR C");
		RR_r8(m_registerBC.lo);
		break;

	case 0x1A:
		formatToOpcode("RR D");
		RR_r8(m_registerDE.hi);
		break;

	case 0x1B:
		formatToOpcode("RR E");
		RR_r8(m_registerDE.lo);
		break;

	case 0x1C:
		formatToOpcode("RR H");
		RR_r8(m_registerHL.hi);
		break;

	case 0x1D:
		formatToOpcode("RR L");
		RR_r8(m_registerHL.lo);
		break;

	case 0x1E:
		formatToOpcode("RR (HL)");
		RR_indirect_HL();
		break;

	case 0x1F:
		formatToOpcode("RR A");
		RR_r8(m_registerAF.accumulator);
		break;

	case 0x20:
		formatToOpcode("SLA B");
		SLA_r8(m_registerBC.hi);
		break;

	case 0x21:
		formatToOpcode("SLA C");
		SLA_r8(m_registerBC.lo);
		break;

	case 0x22:
		formatToOpcode("SLA D");
		SLA_r8(m_registerDE.hi);
		break;

	case 0x23:
		formatToOpcode("SLA E");
		SLA_r8(m_registerDE.lo);
		break;

	case 0x24:
		formatToOpcode("SLA H");
		SLA_r8(m_registerHL.hi);
		break;

	case 0x25:
		formatToOpcode("SLA L");
		SLA_r8(m_registerHL.lo);
		break;

	case 0x26:
		formatToOpcode("SLA (HL)");
		SLA_indirect_HL();
		break;

	case 0x27:
		formatToOpcode("SLA A");
		SLA_r8(m_registerAF.accumulator);
		break;

	case 0x28:
		formatToOpcode("SRA B");
		SRA_r8(m_registerBC.hi);
		break;

	case 0x29:
		formatToOpcode("SRA C");
		SRA_r8(m_registerBC.lo);
		break;

	case 0x2A:
		formatToOpcode("SRA D");
		SRA_r8(m_registerDE.hi);
		break;

	case 0x2B:
		formatToOpcode("SRA E");
		SRA_r8(m_registerDE.lo);
		break;

	case 0x2C:
		formatToOpcode("SRA H");
		SRA_r8(m_registerHL.hi);
		break;

	case 0x2D:
		formatToOpcode("SRA L");
		SRA_r8(m_registerHL.lo);
		break;

	case 0x2E:
		formatToOpcode("SRA (HL)");
		SRA_indirect_HL();
		break;

	case 0x2F:
		formatToOpcode("SRA A");
		SRA_r8(m_registerAF.accumulator);
		break;

	case 0x30:
		formatToOpcode("SWAP B");
		SWAP_r8(m_registerBC.hi);
		break;

	case 0x31:
		formatToOpcode("SWAP C");
		SWAP_r8(m_registerBC.lo);
		break;

	case 0x32:
		formatToOpcode("SWAP D");
		SWAP_r8(m_registerDE.hi);
		break;

	case 0x33:
		formatToOpcode("SWAP E");
		SWAP_r8(m_registerDE.lo);
		break;

	case 0x34:
		formatToOpcode("SWAP H");
		SWAP_r8(m_registerHL.hi);
		break;

	case 0x35:
		formatToOpcode("SWAP L");
		SWAP_r8(m_registerHL.lo);
		break;

	case 0x36:
		formatToOpcode("SWAP (HL)");
		SWAP_indirect_HL();
		break;

	case 0x37:
		formatToOpcode("SWAP A");
		SWAP_r8(m_registerAF.accumulator);
		break;

	case 0x38:
		formatToOpcode("SRL B");
		SRL_r8(m_registerBC.hi);
		break;

	case 0x39:
		formatToOpcode("SRL C");
		SRL_r8(m_registerBC.lo);
		break;

	case 0x3A:
		formatToOpcode("SRL D");
		SRL_r8(m_registerDE.hi);
		break;

	case 0x3B:
		formatToOpcode("SRL E");
		SRL_r8(m_registerDE.lo);
		break;

	case 0x3C:
		formatToOpcode("SRL H");
		SRL_r8(m_registerHL.hi);
		break;

	case 0x3D:
		formatToOpcode("SRL L");
		SRL_r8(m_registerHL.lo);
		break;

	case 0x3E:
		formatToOpcode("SRL (HL)");
		SRL_indirect_HL();
		break;

	case 0x3F:
		formatToOpcode("SRL A");
		SRL_r8(m_registerAF.accumulator);
		break;

	case 0x40:
		formatToOpcode("BIT 0, B");
		BIT_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0x41:
		formatToOpcode("BIT 0, C");
		BIT_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0x42:
		formatToOpcode("BIT 0, D");
		BIT_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0x43:
		formatToOpcode("BIT 0, E");
		BIT_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0x44:
		formatToOpcode("BIT 0, H");
		BIT_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0x45:
		formatToOpcode("BIT 0, L");
		BIT_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0x46:
		formatToOpcode("BIT 0, (HL)");
		BIT_indirect_HL(BitSelect::B0);
		break;

	case 0x47:
		formatToOpcode("BIT 0, A");
		BIT_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0x48:
		formatToOpcode("BIT 1, B");
		BIT_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0x49:
		formatToOpcode("BIT 1, C");
		BIT_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0x4A:
		formatToOpcode("BIT 1, D");
		BIT_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0x4B:
		formatToOpcode("BIT 1, E");
		BIT_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0x4C:
		formatToOpcode("BIT 1, H");
		BIT_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0x4D:
		formatToOpcode("BIT 1, L");
		BIT_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0x4E:
		formatToOpcode("BIT 1, (HL)");
		BIT_indirect_HL(BitSelect::B1);
		break;

	case 0x4F:
		formatToOpcode("BIT 1, A");
		BIT_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0x50:
		formatToOpcode("BIT 2, B");
		BIT_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0x51:
		formatToOpcode("BIT 2, C");
		BIT_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0x52:
		formatToOpcode("BIT 2, D");
		BIT_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0x53:
		formatToOpcode("BIT 2, E");
		BIT_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0x54:
		formatToOpcode("BIT 2, H");
		BIT_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0x55:
		formatToOpcode("BIT 2, L");
		BIT_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0x56:
		formatToOpcode("BIT 2, (HL)");
		BIT_indirect_HL(BitSelect::B2);
		break;

	case 0x57:
		formatToOpcode("BIT 2, A");
		BIT_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0x58:
		formatToOpcode("BIT 3, B");
		BIT_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0x59:
		formatToOpcode("BIT 3, C");
		BIT_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0x5A:
		formatToOpcode("BIT 3, D");
		BIT_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0x5B:
		formatToOpcode("BIT 3, E");
		BIT_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0x5C:
		formatToOpcode("BIT 3, H");
		BIT_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0x5D:
		formatToOpcode("BIT 3, L");
		BIT_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0x5E:
		formatToOpcode("BIT 3, (HL)");
		BIT_indirect_HL(BitSelect::B3);
		break;

	case 0x5F:
		formatToOpcode("BIT 3, A");
		BIT_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0x60:
		formatToOpcode("BIT 4, B");
		BIT_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0x61:
		formatToOpcode("BIT 4, C");
		BIT_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0x62:
		formatToOpcode("BIT 4, D");
		BIT_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0x63:
		formatToOpcode("BIT 4, E");
		BIT_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0x64:
		formatToOpcode("BIT 4, H");
		BIT_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0x65:
		formatToOpcode("BIT 4, L");
		BIT_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0x66:
		formatToOpcode("BIT 4, (HL)");
		BIT_indirect_HL(BitSelect::B4);
		break;

	case 0x67:
		formatToOpcode("BIT 4, A");
		BIT_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0x68:
		formatToOpcode("BIT 5, B");
		BIT_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0x69:
		formatToOpcode("BIT 5, C");
		BIT_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0x6A:
		formatToOpcode("BIT 5, D");
		BIT_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0x6B:
		formatToOpcode("BIT 5, E");
		BIT_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0x6C:
		formatToOpcode("BIT 5, H");
		BIT_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0x6D:
		formatToOpcode("BIT 5, L");
		BIT_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0x6E:
		formatToOpcode("BIT 5, (HL)");
		BIT_indirect_HL(BitSelect::B5);
		break;

	case 0x6F:
		formatToOpcode("BIT 5, A");
		BIT_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0x70:
		formatToOpcode("BIT 6, B");
		BIT_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0x71:
		formatToOpcode("BIT 6, C");
		BIT_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0x72:
		formatToOpcode("BIT 6, D");
		BIT_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0x73:
		formatToOpcode("BIT 6, E");
		BIT_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0x74:
		formatToOpcode("BIT 6, H");
		BIT_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0x75:
		formatToOpcode("BIT 6, L");
		BIT_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0x76:
		formatToOpcode("BIT 6, (HL)");
		BIT_indirect_HL(BitSelect::B6);
		break;

	case 0x77:
		formatToOpcode("BIT 6, A");
		BIT_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0x78:
		formatToOpcode("BIT 7, B");
		BIT_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0x79:
		formatToOpcode("BIT 7, C");
		BIT_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0x7A:
		formatToOpcode("BIT 7, D");
		BIT_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0x7B:
		formatToOpcode("BIT 7, E");
		BIT_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0x7C:
		formatToOpcode("BIT 7, H");
		BIT_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0x7D:
		formatToOpcode("BIT 7, L");
		BIT_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0x7E:
		formatToOpcode("BIT 7, (HL)");
		BIT_indirect_HL(BitSelect::B7);
		break;

	case 0x7F:
		formatToOpcode("BIT 7, A");
		BIT_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	case 0x80:
		formatToOpcode("RES 0, B");
		RES_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0x81:
		formatToOpcode("RES 0, C");
		RES_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0x82:
		formatToOpcode("RES 0, D");
		RES_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0x83:
		formatToOpcode("RES 0, E");
		RES_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0x84:
		formatToOpcode("RES 0, H");
		RES_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0x85:
		formatToOpcode("RES 0, L");
		RES_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0x86:
		formatToOpcode("RES 0, (HL)");
		RES_indirect_HL(BitSelect::B0);
		break;

	case 0x87:
		formatToOpcode("RES 0, A");
		RES_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0x88:
		formatToOpcode("RES 1, B");
		RES_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0x89:
		formatToOpcode("RES 1, C");
		RES_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0x8A:
		formatToOpcode("RES 1, D");
		RES_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0x8B:
		formatToOpcode("RES 1, E");
		RES_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0x8C:
		formatToOpcode("RES 1, H");
		RES_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0x8D:
		formatToOpcode("RES 1, L");
		RES_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0x8E:
		formatToOpcode("RES 1, (HL)");
		RES_indirect_HL(BitSelect::B1);
		break;

	case 0x8F:
		formatToOpcode("RES 1, A");
		RES_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0x90:
		formatToOpcode("RES 2, B");
		RES_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0x91:
		formatToOpcode("RES 2, C");
		RES_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0x92:
		formatToOpcode("RES 2, D");
		RES_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0x93:
		formatToOpcode("RES 2, E");
		RES_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0x94:
		formatToOpcode("RES 2, H");
		RES_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0x95:
		formatToOpcode("RES 2, L");
		RES_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0x96:
		formatToOpcode("RES 2, (HL)");
		RES_indirect_HL(BitSelect::B2);
		break;

	case 0x97:
		formatToOpcode("RES 2, A");
		RES_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0x98:
		formatToOpcode("RES 3, B");
		RES_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0x99:
		formatToOpcode("RES 3, C");
		RES_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0x9A:
		formatToOpcode("RES 3, D");
		RES_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0x9B:
		formatToOpcode("RES 3, E");
		RES_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0x9C:
		formatToOpcode("RES 3, H");
		RES_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0x9D:
		formatToOpcode("RES 3, L");
		RES_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0x9E:
		formatToOpcode("RES 3, (HL)");
		RES_indirect_HL(BitSelect::B3);
		break;

	case 0x9F:
		formatToOpcode("RES 3, A");
		RES_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0xA0:
		formatToOpcode("RES 4, B");
		RES_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0xA1:
		formatToOpcode("RES 4, C");
		RES_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0xA2:
		formatToOpcode("RES 4, D");
		RES_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0xA3:
		formatToOpcode("RES 4, E");
		RES_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0xA4:
		formatToOpcode("RES 4, H");
		RES_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0xA5:
		formatToOpcode("RES 4, L");
		RES_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0xA6:
		formatToOpcode("RES 4, (HL)");
		RES_indirect_HL(BitSelect::B4);
		break;

	case 0xA7:
		formatToOpcode("RES 4, A");
		RES_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0xA8:
		formatToOpcode("RES 5, B");
		RES_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0xA9:
		formatToOpcode("RES 5, C");
		RES_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0xAA:
		formatToOpcode("RES 5, D");
		RES_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0xAB:
		formatToOpcode("RES 5, E");
		RES_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0xAC:
		formatToOpcode("RES 5, H");
		RES_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0xAD:
		formatToOpcode("RES 5, L");
		RES_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0xAE:
		formatToOpcode("RES 5, (HL)");
		RES_indirect_HL(BitSelect::B5);
		break;

	case 0xAF:
		formatToOpcode("RES 5, A");
		RES_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0xB0:
		formatToOpcode("RES 6, B");
		RES_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0xB1:
		formatToOpcode("RES 6, C");
		RES_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0xB2:
		formatToOpcode("RES 6, D");
		RES_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0xB3:
		formatToOpcode("RES 6, E");
		RES_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0xB4:
		formatToOpcode("RES 6, H");
		RES_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0xB5:
		formatToOpcode("RES 6, L");
		RES_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0xB6:
		formatToOpcode("RES 6, (HL)");
		RES_indirect_HL(BitSelect::B6);
		break;

	case 0xB7:
		formatToOpcode("RES 6, A");
		RES_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0xB8:
		formatToOpcode("RES 7, B");
		RES_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0xB9:
		formatToOpcode("RES 7, C");
		RES_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0xBA:
		formatToOpcode("RES 7, D");
		RES_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0xBB:
		formatToOpcode("RES 7, E");
		RES_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0xBC:
		formatToOpcode("RES 7, H");
		RES_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0xBD:
		formatToOpcode("RES 7, L");
		RES_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0xBE:
		formatToOpcode("RES 7, (HL)");
		RES_indirect_HL(BitSelect::B7);
		break;

	case 0xBF:
		formatToOpcode("RES 7, A");
		RES_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	case 0xC0:
		formatToOpcode("SET 0, B");
		SET_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0xC1:
		formatToOpcode("SET 0, C");
		SET_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0xC2:
		formatToOpcode("SET 0, D");
		SET_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0xC3:
		formatToOpcode("SET 0, E");
		SET_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0xC4:
		formatToOpcode("SET 0, H");
		SET_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0xC5:
		formatToOpcode("SET 0, L");
		SET_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0xC6:
		formatToOpcode("SET 0, (HL)");
		SET_indirect_HL(BitSelect::B0);
		break;

	case 0xC7:
		formatToOpcode("SET 0, A");
		SET_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0xC8:
		formatToOpcode("SET 1, B");
		SET_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0xC9:
		formatToOpcode("SET 1, C");
		SET_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0xCA:
		formatToOpcode("SET 1, D");
		SET_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0xCB:
		formatToOpcode("SET 1, E");
		SET_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0xCC:
		formatToOpcode("SET 1, H");
		SET_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0xCD:
		formatToOpcode("SET 1, L");
		SET_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0xCE:
		formatToOpcode("SET 1, (HL)");
		SET_indirect_HL(BitSelect::B1);
		break;

	case 0xCF:
		formatToOpcode("SET 1, A");
		SET_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0xD0:
		formatToOpcode("SET 2, B");
		SET_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0xD1:
		formatToOpcode("SET 2, C");
		SET_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0xD2:
		formatToOpcode("SET 2, D");
		SET_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0xD3:
		formatToOpcode("SET 2, E");
		SET_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0xD4:
		formatToOpcode("SET 2, H");
		SET_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0xD5:
		formatToOpcode("SET 2, L");
		SET_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0xD6:
		formatToOpcode("SET 2, (HL)");
		SET_indirect_HL(BitSelect::B2);
		break;

	case 0xD7:
		formatToOpcode("SET 2, A");
		SET_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0xD8:
		formatToOpcode("SET 3, B");
		SET_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0xD9:
		formatToOpcode("SET 3, C");
		SET_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0xDA:
		formatToOpcode("SET 3, D");
		SET_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0xDB:
		formatToOpcode("SET 3, E");
		SET_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0xDC:
		formatToOpcode("SET 3, H");
		SET_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0xDD:
		formatToOpcode("SET 3, L");
		SET_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0xDE:
		formatToOpcode("SET 3, (HL)");
		SET_indirect_HL(BitSelect::B3);
		break;

	case 0xDF:
		formatToOpcode("SET 3, A");
		SET_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0xE0:
		formatToOpcode("SET 4, B");
		SET_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0xE1:
		formatToOpcode("SET 4, C");
		SET_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0xE2:
		formatToOpcode("SET 4, D");
		SET_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0xE3:
		formatToOpcode("SET 4, E");
		SET_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0xE4:
		formatToOpcode("SET 4, H");
		SET_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0xE5:
		formatToOpcode("SET 4, L");
		SET_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0xE6:
		formatToOpcode("SET 4, (HL)");
		SET_indirect_HL(BitSelect::B4);
		break;

	case 0xE7:
		formatToOpcode("SET 4, A");
		SET_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0xE8:
		formatToOpcode("SET 5, B");
		SET_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0xE9:
		formatToOpcode("SET 5, C");
		SET_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0xEA:
		formatToOpcode("SET 5, D");
		SET_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0xEB:
		formatToOpcode("SET 5, E");
		SET_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0xEC:
		formatToOpcode("SET 5, H");
		SET_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0xED:
		formatToOpcode("SET 5, L");
		SET_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0xEE:
		formatToOpcode("SET 5, (HL)");
		SET_indirect_HL(BitSelect::B5);
		break;

	case 0xEF:
		formatToOpcode("SET 5, A");
		SET_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0xF0:
		formatToOpcode("SET 6, B");
		SET_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0xF1:
		formatToOpcode("SET 6, C");
		SET_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0xF2:
		formatToOpcode("SET 6, D");
		SET_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0xF3:
		formatToOpcode("SET 6, E");
		SET_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0xF4:
		formatToOpcode("SET 6, H");
		SET_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0xF5:
		formatToOpcode("SET 6, L");
		SET_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0xF6:
		formatToOpcode("SET 6, (HL)");
		SET_indirect_HL(BitSelect::B6);
		break;

	case 0xF7:
		formatToOpcode("SET 6, A");
		SET_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0xF8:
		formatToOpcode("SET 7, B");
		SET_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0xF9:
		formatToOpcode("SET 7, C");
		SET_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0xFA:
		formatToOpcode("SET 7, D");
		SET_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0xFB:
		formatToOpcode("SET 7, E");
		SET_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0xFC:
		formatToOpcode("SET 7, H");
		SET_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0xFD:
		formatToOpcode("SET 7, L");
		SET_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0xFE:
		formatToOpcode("SET 7, (HL)");
		SET_indirect_HL(BitSelect::B7);
		break;

	case 0xFF:
		formatToOpcode("SET 7, A");
		SET_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	default:
		break;
	}
}

void Sm83::LDH_indirect_n8_A()
{
	uint8_t offset = cpuFetch();
	uint16_t address = 0xFF00 + offset;
	m_bus->cpuWrite(address, m_registerAF.accumulator);
	formatToOpcode("$(FF00 + {:02X}), A", offset);
}

void Sm83::LDH_indirect_C_A()
{
	uint16_t address = 0xFF00 + m_registerBC.lo;
	m_bus->cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LDH_A_indirect_n8()
{
	uint8_t offset = cpuFetch();
	uint16_t address = 0xFF00 + offset;
	m_registerAF.accumulator = m_bus->cpuRead(address);
	formatToOpcode("$(FF00 + {:02X})", offset);
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
	formatToOpcode("${:02X}", dest);
}

void Sm83::LD_r8_r8(uint8_t &dest, uint8_t &src)
{
	dest = src;
}

void Sm83::LD_r16_n16(Sm83Register &dest)
{
	dest.lo = cpuFetch();
	dest.hi = cpuFetch();
	formatToOpcode("${:04X}", (dest.hi << 8) | dest.lo);
}

void Sm83::LD_SP_n16()
{
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	m_stackPointer = lo | (hi << 8);
	formatToOpcode("${:04X}", m_stackPointer);
}
void Sm83::LD_indirect_r16_r8(Sm83Register &dest, uint8_t &src)
{
	uint16_t address = (dest.hi << 8) | dest.lo;
	m_bus->cpuWrite(address, src);
}

void Sm83::LD_indirect_n16_A()
{
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t address = lo | (hi << 8);
	m_bus->cpuWrite(address, m_registerAF.accumulator);
	formatToOpcode("$({:04X}), A", address);
}

void Sm83::LD_A_indirect_n16()
{
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t address = lo | (hi << 8);
	m_registerAF.accumulator = m_bus->cpuRead(address);
	formatToOpcode("$({:04X})", address);
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
	formatToOpcode("${:02X}", value);
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
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t address = lo | (hi << 8);

	// store lo byte the hi byte at next address
	m_bus->cpuWrite(address, m_stackPointer & 0xFF);
	m_bus->cpuWrite(address + 1, (m_stackPointer & 0xFF00) >> 8);
	formatToOpcode("(${:04X}), SP", address);
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
	formatToOpcode("+- ${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:04X}", m_programCounter);
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

	formatToOpcode("${:04X}", m_programCounter);
}

void Sm83::JP_CC_n16(bool condition)
{
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t address = lo | (hi << 8);

	if (condition)
	{
		mTick();
		m_programCounter = address;
	}

	formatToOpcode("${:04X}", m_programCounter);
}

void Sm83::JP_n16()
{
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t address = lo | (hi << 8);

	mTick();
	m_programCounter = address;
	formatToOpcode("${:04X}", m_programCounter);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	formatToOpcode("${:02X}", operand);
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
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t callAddress = lo | (hi << 8);

	mTick();

	// save hi byte of pc to stack followed by the lo byte
	m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
	m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

	m_programCounter = callAddress;
	formatToOpcode("${:04X}", m_programCounter);
}

void Sm83::CALL_CC_n16(bool condition)
{
	uint8_t lo = cpuFetch();
	uint8_t hi = cpuFetch();
	uint16_t callAddress = lo | (hi << 8);

	if (condition)
	{
		mTick();

		// save hi byte of pc to stack followed by the lo byte
		m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
		m_bus->cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

		m_programCounter = callAddress;
	}

	formatToOpcode("${:04X}", m_programCounter);
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
