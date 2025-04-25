#include "sm83.h"
#include <cstdint>

Sm83::Sm83(Bus &bus, Disassembler &disassembler) : m_bus(bus), m_disassembler(disassembler)
{
	m_tCycleTicks = 0;

	initDMG();

	m_ime = false;
	
	m_logEnable = false;
}

void Sm83::runInstruction()
{
	if (m_logEnable)
	{
		m_disassembler.setProgramCounter(m_programCounter);
		m_disassembler.instructionStep();
		m_disassembler.logToConsole();
	}

	uint8_t opcode = cpuFetch();
	decodeExecute(opcode);

	/*if (m_logEnable)*/
	/*	fmt::print("{}\n", m_instructionString);*/
}

void Sm83::decodeExecute(uint8_t opcode)
{
	switch (opcode)
	{

	case 0x00:
		break;

	case 0x01:
		LD_r16_n16(m_registerBC);
		break;

	case 0x02:
		LD_indirect_r16_r8(m_registerBC, m_registerAF.accumulator);
		break;

	case 0x03:
		INC_r16(m_registerBC);
		break;

	case 0x04:
		INC_r8(m_registerBC.hi);
		break;

	case 0x05:
		DEC_r8(m_registerBC.hi);
		break;

	case 0x06:
		LD_r8_n8(m_registerBC.hi);
		break;

	case 0x07:
		RLCA();
		break;

	case 0x08:
		LD_indirect_n16_SP();
		break;

	case 0x09:
		ADD_HL_r16(m_registerBC);
		break;

	case 0x0A:
		LD_A_indirect_r16(m_registerBC);
		break;

	case 0x0B:
		DEC_r16(m_registerBC);
		break;

	case 0x0C:
		INC_r8(m_registerBC.lo);
		break;

	case 0x0D:
		DEC_r8(m_registerBC.lo);
		break;

	case 0x0E:
		LD_r8_n8(m_registerBC.lo);
		break;

	case 0x0F:
		RRCA();
		break;

	case 0x10:
		break;

	case 0x11:
		LD_r16_n16(m_registerDE);
		break;

	case 0x12:
		LD_indirect_r16_r8(m_registerDE, m_registerAF.accumulator);
		break;

	case 0x13:
		INC_r16(m_registerDE);
		break;

	case 0x14:
		INC_r8(m_registerDE.hi);
		break;

	case 0x15:
		DEC_r8(m_registerDE.hi);
		break;

	case 0x16:
		LD_r8_n8(m_registerDE.hi);
		break;

	case 0x17:
		RLA();
		break;

	case 0x18:
		JR_i8();
		break;

	case 0x19:
		ADD_HL_r16(m_registerDE);
		break;

	case 0x1A:
		LD_A_indirect_r16(m_registerDE);
		break;

	case 0x1B:
		DEC_r16(m_registerDE);
		break;

	case 0x1C:
		INC_r8(m_registerDE.lo);
		break;

	case 0x1D:
		DEC_r8(m_registerDE.lo);
		break;

	case 0x1E:
		LD_r8_n8(m_registerDE.lo);
		break;

	case 0x1F:
		RRA();
		break;

	case 0x20:
		JR_CC_i8(!m_registerAF.flags.Z);
		break;

	case 0x21:
		LD_r16_n16(m_registerHL);
		break;

	case 0x22:
		LD_indirect_HLI_A();
		break;

	case 0x23:
		INC_r16(m_registerHL);
		break;

	case 0x24:
		INC_r8(m_registerHL.hi);
		break;

	case 0x25:
		DEC_r8(m_registerHL.hi);
		break;

	case 0x26:
		LD_r8_n8(m_registerHL.hi);
		break;

	case 0x27:
		DAA();
		break;

	case 0x28:
		JR_CC_i8(m_registerAF.flags.Z);
		break;

	case 0x29:
		ADD_HL_r16(m_registerHL);
		break;

	case 0x2A:
		LD_A_indirect_HLI();
		break;

	case 0x2B:
		DEC_r16(m_registerHL);
		break;

	case 0x2C:
		INC_r8(m_registerHL.lo);
		break;

	case 0x2D:
		DEC_r8(m_registerHL.lo);
		break;

	case 0x2E:
		LD_r8_n8(m_registerHL.lo);
		break;

	case 0x2F:
		CPL();
		break;

	case 0x30:
		JR_CC_i8(!m_registerAF.flags.C);
		break;

	case 0x31:
		LD_SP_n16();
		break;

	case 0x32:
		LD_indirect_HLD_A();
		break;

	case 0x33:
		INC_SP();
		break;

	case 0x34:
		INC_indirect_HL();
		break;

	case 0x35:
		DEC_indirect_HL();
		break;

	case 0x36:
		LD_indirect_HL_n8();
		break;

	case 0x37:
		SCF();
		break;

	case 0x38:
		JR_CC_i8(m_registerAF.flags.C);
		break;

	case 0x39:
		ADD_HL_SP();
		break;

	case 0x3A:
		LD_A_indirect_HLD();
		break;

	case 0x3B:
		DEC_SP();
		break;

	case 0x3C:
		INC_r8(m_registerAF.accumulator);
		break;

	case 0x3D:
		DEC_r8(m_registerAF.accumulator);
		break;

	case 0x3E:
		LD_r8_n8(m_registerAF.accumulator);
		break;

	case 0x3F:
		CCF();
		break;

	case 0x40:
		LD_r8_r8(m_registerBC.hi, m_registerBC.hi);
		break;

	case 0x41:
		LD_r8_r8(m_registerBC.hi, m_registerBC.lo);
		break;

	case 0x42:
		LD_r8_r8(m_registerBC.hi, m_registerDE.hi);
		break;

	case 0x43:
		LD_r8_r8(m_registerBC.hi, m_registerDE.lo);
		break;

	case 0x44:
		LD_r8_r8(m_registerBC.hi, m_registerHL.hi);
		break;

	case 0x45:
		LD_r8_r8(m_registerBC.hi, m_registerHL.lo);
		break;

	case 0x46:
		LD_r8_indirect_HL(m_registerBC.hi);
		break;

	case 0x47:
		LD_r8_r8(m_registerBC.hi, m_registerAF.accumulator);
		break;

	case 0x48:
		LD_r8_r8(m_registerBC.lo, m_registerBC.hi);
		break;

	case 0x49:
		LD_r8_r8(m_registerBC.lo, m_registerBC.lo);
		break;

	case 0x4A:
		LD_r8_r8(m_registerBC.lo, m_registerDE.hi);
		break;

	case 0x4B:
		LD_r8_r8(m_registerBC.lo, m_registerDE.lo);
		break;

	case 0x4C:
		LD_r8_r8(m_registerBC.lo, m_registerHL.hi);
		break;

	case 0x4D:
		LD_r8_r8(m_registerBC.lo, m_registerHL.lo);
		break;

	case 0x4E:
		LD_r8_indirect_HL(m_registerBC.lo);
		break;

	case 0x4F:
		LD_r8_r8(m_registerBC.lo, m_registerAF.accumulator);
		break;

	case 0x50:
		LD_r8_r8(m_registerDE.hi, m_registerBC.hi);
		break;

	case 0x51:
		LD_r8_r8(m_registerDE.hi, m_registerBC.lo);
		break;
	case 0x52:
		LD_r8_r8(m_registerDE.hi, m_registerDE.hi);
		break;

	case 0x53:
		LD_r8_r8(m_registerDE.hi, m_registerDE.lo);
		break;

	case 0x54:
		LD_r8_r8(m_registerDE.hi, m_registerHL.hi);
		break;

	case 0x55:
		LD_r8_r8(m_registerDE.hi, m_registerHL.lo);
		break;
	case 0x56:
		LD_r8_indirect_HL(m_registerDE.hi);
		break;

	case 0x57:
		LD_r8_r8(m_registerDE.hi, m_registerAF.accumulator);
		break;

	case 0x58:
		LD_r8_r8(m_registerDE.lo, m_registerBC.hi);
		break;
	case 0x59:
		LD_r8_r8(m_registerDE.lo, m_registerBC.lo);
		break;

	case 0x5A:
		LD_r8_r8(m_registerDE.lo, m_registerDE.hi);
		break;
	case 0x5B:
		LD_r8_r8(m_registerDE.lo, m_registerDE.lo);
		break;

	case 0x5C:
		LD_r8_r8(m_registerDE.lo, m_registerHL.hi);
		break;

	case 0x5D:
		LD_r8_r8(m_registerDE.lo, m_registerHL.lo);
		break;

	case 0x5E:
		LD_r8_indirect_HL(m_registerDE.lo);
		break;

	case 0x5F:
		LD_r8_r8(m_registerDE.lo, m_registerAF.accumulator);
		break;

	case 0x60:
		LD_r8_r8(m_registerHL.hi, m_registerBC.hi);
		break;

	case 0x61:
		LD_r8_r8(m_registerHL.hi, m_registerBC.lo);
		break;

	case 0x62:
		LD_r8_r8(m_registerHL.hi, m_registerDE.hi);
		break;

	case 0x63:
		LD_r8_r8(m_registerHL.hi, m_registerDE.lo);
		break;
	case 0x64:

		LD_r8_r8(m_registerHL.hi, m_registerHL.hi);
		break;

	case 0x65:
		LD_r8_r8(m_registerHL.hi, m_registerHL.lo);
		break;

	case 0x66:
		LD_r8_indirect_HL(m_registerHL.hi);
		break;

	case 0x67:
		LD_r8_r8(m_registerHL.hi, m_registerAF.accumulator);
		break;

	case 0x68:
		LD_r8_r8(m_registerHL.lo, m_registerBC.hi);
		break;

	case 0x69:
		LD_r8_r8(m_registerHL.lo, m_registerBC.lo);
		break;

	case 0x6A:
		LD_r8_r8(m_registerHL.lo, m_registerDE.hi);
		break;

	case 0x6B:
		LD_r8_r8(m_registerHL.lo, m_registerDE.lo);
		break;

	case 0x6C:
		LD_r8_r8(m_registerHL.lo, m_registerHL.hi);
		break;

	case 0x6D:
		LD_r8_r8(m_registerHL.lo, m_registerHL.lo);
		break;

	case 0x6E:
		LD_r8_indirect_HL(m_registerHL.lo);
		break;

	case 0x6F:
		LD_r8_r8(m_registerHL.lo, m_registerAF.accumulator);
		break;

	case 0x70:
		LD_indirect_r16_r8(m_registerHL, m_registerBC.hi);
		break;

	case 0x71:
		LD_indirect_r16_r8(m_registerHL, m_registerBC.lo);
		break;

	case 0x72:
		LD_indirect_r16_r8(m_registerHL, m_registerDE.hi);
		break;

	case 0x73:
		LD_indirect_r16_r8(m_registerHL, m_registerDE.lo);
		break;

	case 0x74:
		LD_indirect_r16_r8(m_registerHL, m_registerHL.hi);
		break;

	case 0x75:
		LD_indirect_r16_r8(m_registerHL, m_registerHL.lo);
		break;

	case 0x76:
		break;

	case 0x77:
		LD_indirect_r16_r8(m_registerHL, m_registerAF.accumulator);
		break;

	case 0x78:
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.hi);
		break;

	case 0x79:
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.lo);
		break;

	case 0x7A:
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.hi);
		break;

	case 0x7B:
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.lo);
		break;

	case 0x7C:
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.hi);
		break;

	case 0x7D:
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.lo);
		break;

	case 0x7E:
		LD_r8_indirect_HL(m_registerAF.accumulator);
		break;

	case 0x7F:
		LD_r8_r8(m_registerAF.accumulator, m_registerAF.accumulator);
		break;

	case 0x80:
		ADD_A_r8(m_registerBC.hi);
		break;

	case 0x81:
		ADD_A_r8(m_registerBC.lo);
		break;

	case 0x82:
		ADD_A_r8(m_registerDE.hi);
		break;

	case 0x83:
		ADD_A_r8(m_registerDE.lo);
		break;

	case 0x84:
		ADD_A_r8(m_registerHL.hi);
		break;

	case 0x85:
		ADD_A_r8(m_registerHL.lo);
		break;

	case 0x86:
		ADD_A_indirect_HL();
		break;

	case 0x87:
		ADD_A_r8(m_registerAF.accumulator);
		break;

	case 0x88:
		ADC_A_r8(m_registerBC.hi);
		break;

	case 0x89:
		ADC_A_r8(m_registerBC.lo);
		break;
	case 0x8A:

		ADC_A_r8(m_registerDE.hi);
		break;

	case 0x8B:
		ADC_A_r8(m_registerDE.lo);
		break;

	case 0x8C:
		ADC_A_r8(m_registerHL.hi);
		break;

	case 0x8D:
		ADC_A_r8(m_registerHL.lo);
		break;

	case 0x8E:
		ADC_A_indirect_HL();
		break;

	case 0x8F:
		ADC_A_r8(m_registerAF.accumulator);
		break;

	case 0x90:
		SUB_A_r8(m_registerBC.hi);
		break;

	case 0x91:
		SUB_A_r8(m_registerBC.lo);
		break;

	case 0x92:
		SUB_A_r8(m_registerDE.hi);
		break;
	case 0x93:

		SUB_A_r8(m_registerDE.lo);
		break;

	case 0x94:
		SUB_A_r8(m_registerHL.hi);
		break;

	case 0x95:
		SUB_A_r8(m_registerHL.lo);
		break;

	case 0x96:
		SUB_A_indirect_HL();
		break;

	case 0x97:
		SUB_A_r8(m_registerAF.accumulator);
		break;

	case 0x98:
		SBC_A_r8(m_registerBC.hi);
		break;

	case 0x99:
		SBC_A_r8(m_registerBC.lo);
		break;

	case 0x9A:
		SBC_A_r8(m_registerDE.hi);
		break;

	case 0x9B:
		SBC_A_r8(m_registerDE.lo);
		break;

	case 0x9C:
		SBC_A_r8(m_registerHL.hi);
		break;

	case 0x9D:
		SBC_A_r8(m_registerHL.lo);
		break;

	case 0x9E:
		SBC_A_indirect_HL();
		break;

	case 0x9F:
		SBC_A_r8(m_registerAF.accumulator);
		break;

	case 0xA0:
		AND_A_r8(m_registerBC.hi);
		break;

	case 0xA1:
		AND_A_r8(m_registerBC.lo);
		break;

	case 0xA2:
		AND_A_r8(m_registerDE.hi);
		break;

	case 0xA3:
		AND_A_r8(m_registerDE.lo);
		break;

	case 0xA4:
		AND_A_r8(m_registerHL.hi);
		break;

	case 0xA5:
		AND_A_r8(m_registerHL.lo);
		break;

	case 0xA6:
		AND_A_indirect_HL();
		break;

	case 0xA7:
		AND_A_r8(m_registerAF.accumulator);
		break;

	case 0xA8:
		XOR_A_r8(m_registerBC.hi);
		break;

	case 0xA9:
		XOR_A_r8(m_registerBC.lo);
		break;

	case 0xAA:
		XOR_A_r8(m_registerDE.hi);
		break;

	case 0xAB:
		XOR_A_r8(m_registerDE.lo);
		break;

	case 0xAC:
		XOR_A_r8(m_registerHL.hi);
		break;

	case 0xAD:
		XOR_A_r8(m_registerHL.lo);
		break;

	case 0xAE:
		XOR_A_indirect_HL();
		break;

	case 0xAF:
		XOR_A_r8(m_registerAF.accumulator);
		break;

	case 0xB0:
		OR_A_r8(m_registerBC.hi);
		break;

	case 0xB1:
		OR_A_r8(m_registerBC.lo);
		break;

	case 0xB2:
		OR_A_r8(m_registerDE.hi);
		break;

	case 0xB3:
		OR_A_r8(m_registerDE.lo);
		break;

	case 0xB4:
		OR_A_r8(m_registerHL.hi);
		break;

	case 0xB5:
		OR_A_r8(m_registerHL.lo);
		break;

	case 0xB6:
		OR_A_indirect_HL();
		break;

	case 0xB7:
		OR_A_r8(m_registerAF.accumulator);
		break;

	case 0xB8:
		CP_A_r8(m_registerBC.hi);
		break;

	case 0xB9:
		CP_A_r8(m_registerBC.lo);
		break;

	case 0xBA:
		CP_A_r8(m_registerDE.hi);
		break;

	case 0xBB:
		CP_A_r8(m_registerDE.lo);
		break;

	case 0xBC:
		CP_A_r8(m_registerHL.hi);
		break;

	case 0xBD:
		CP_A_r8(m_registerHL.lo);
		break;

	case 0xBE:
		CP_A_indirect_HL();
		break;

	case 0xBF:
		CP_A_r8(m_registerAF.accumulator);
		break;

	case 0xC0:
		RET_CC(!m_registerAF.flags.Z);
		break;

	case 0xC1:
		POP_r16(m_registerBC);
		break;

	case 0xC2:
		JP_CC_n16(!m_registerAF.flags.Z);
		break;

	case 0xC3:
		JP_n16();
		break;

	case 0xC4:
		CALL_CC_n16(!m_registerAF.flags.Z);
		break;

	case 0xC5:
		PUSH_r16(m_registerBC);
		break;

	case 0xC6:
		ADD_A_n8();
		break;

	case 0xC7:
		RST(RstVector::H00);
		break;

	case 0xC8:
		RET_CC(m_registerAF.flags.Z);
		break;

	case 0xC9:
		RET();
		break;

	case 0xCA:
		JP_CC_n16(m_registerAF.flags.Z);
		break;

	// Prefix mode, decode opcode with second opcode table
	case 0xCB:
		decodeExecutePrefixedMode(cpuFetch());
		break;

	case 0xCC:
		CALL_CC_n16(m_registerAF.flags.Z);
		break;

	case 0xCD:
		CALL_n16();
		break;

	case 0xCE:
		ADC_A_n8();
		break;

	case 0xCF:
		RST(RstVector::H08);
		break;

	case 0xD0:
		RET_CC(!m_registerAF.flags.C);
		break;

	case 0xD1:
		POP_r16(m_registerDE);
		break;

	case 0xD2:
		JP_CC_n16(!m_registerAF.flags.C);
		break;

	case 0xD3:
		m_programCounter -= 1;
		break;

	case 0xD4:
		CALL_CC_n16(!m_registerAF.flags.C);
		break;

	case 0xD5:
		PUSH_r16(m_registerDE);
		break;

	case 0xD6:
		SUB_A_n8();
		break;

	case 0xD7:
		RST(RstVector::H10);
		break;

	case 0xD8:
		RET_CC(m_registerAF.flags.C);
		break;

	case 0xD9:
		RETI();
		break;

	case 0xDA:
		JP_CC_n16(m_registerAF.flags.C);
		break;

	case 0xDB:
		m_programCounter -= 1;
		break;

	case 0xDC:
		CALL_CC_n16(m_registerAF.flags.C);
		break;

	case 0xDD:
		m_programCounter -= 1;
		break;

	case 0xDE:
		SBC_A_n8();
		break;

	case 0xDF:
		RST(RstVector::H18);
		break;

	case 0xE0:
		LDH_indirect_n8_A();
		break;

	case 0xE1:
		POP_r16(m_registerHL);
		break;

	case 0xE2:
		LDH_indirect_C_A();
		break;

	case 0xE3:
	case 0xE4:
		m_programCounter -= 1;
		break;

	case 0xE5:
		PUSH_r16(m_registerHL);
		break;

	case 0xE6:
		AND_A_n8();
		break;

	case 0xE7:
		RST(RstVector::H20);
		break;

	case 0xE8:
		ADD_SP_i8();
		break;

	case 0xE9:
		JP_HL();
		break;

	case 0xEA:
		LD_indirect_n16_A();
		break;

	case 0xEB:
	case 0xEC:
	case 0xED:
		m_programCounter -= 1;
		break;

	case 0xEE:
		XOR_A_n8();
		break;

	case 0xEF:
		RST(RstVector::H28);
		break;

	case 0xF0:
		LDH_A_indirect_n8();
		break;

	case 0xF1:
		POP_AF();
		break;

	case 0xF2:
		LDH_A_indirect_C();
		break;

	case 0xF3:
		DI();
		break;

	case 0xF4:
		m_programCounter -= 1;
		break;

	case 0xF5:
		PUSH_AF();
		break;

	case 0xF6:
		OR_A_n8();
		break;

	case 0xF7:
		RST(RstVector::H30);
		break;

	case 0xF8:
		LD_HL_SP_i8();
		break;

	case 0xF9:
		LD_SP_HL();
		break;

	case 0xFA:
		LD_A_indirect_n16();
		break;

	case 0xFB:
		EI();
		break;

	case 0xFC:
	case 0xFD:
		m_programCounter -= 1;
		break;

	case 0xFE:
		CP_A_n8();
		break;

	case 0xFF:
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
		RLC_r8(m_registerBC.hi);
		break;

	case 0x01:
		RLC_r8(m_registerBC.lo);
		break;

	case 0x02:
		RLC_r8(m_registerDE.hi);
		break;

	case 0x03:
		RLC_r8(m_registerDE.lo);
		break;

	case 0x04:
		RLC_r8(m_registerHL.hi);
		break;
	case 0x05:

		RLC_r8(m_registerHL.lo);
		break;

	case 0x06:
		RLC_indirect_HL();
		break;

	case 0x07:
		RLC_r8(m_registerAF.accumulator);
		break;

	case 0x08:
		RRC_r8(m_registerBC.hi);
		break;

	case 0x09:
		RRC_r8(m_registerBC.lo);
		break;

	case 0x0A:
		RRC_r8(m_registerDE.hi);
		break;

	case 0x0B:
		RRC_r8(m_registerDE.lo);
		break;

	case 0x0C:
		RRC_r8(m_registerHL.hi);
		break;

	case 0x0D:
		RRC_r8(m_registerHL.lo);
		break;

	case 0x0E:
		RRC_indirect_HL();
		break;

	case 0x0F:
		RRC_r8(m_registerAF.accumulator);
		break;

	case 0x10:
		RL_r8(m_registerBC.hi);
		break;

	case 0x11:
		RL_r8(m_registerBC.lo);
		break;

	case 0x12:
		RL_r8(m_registerDE.hi);
		break;

	case 0x13:
		RL_r8(m_registerDE.lo);
		break;
	case 0x14:

		RL_r8(m_registerHL.hi);
		break;
	case 0x15:

		RL_r8(m_registerHL.lo);
		break;

	case 0x16:
		RL_indirect_HL();
		break;

	case 0x17:
		RL_r8(m_registerAF.accumulator);
		break;

	case 0x18:
		RR_r8(m_registerBC.hi);
		break;

	case 0x19:
		RR_r8(m_registerBC.lo);
		break;

	case 0x1A:
		RR_r8(m_registerDE.hi);
		break;

	case 0x1B:
		RR_r8(m_registerDE.lo);
		break;

	case 0x1C:
		RR_r8(m_registerHL.hi);
		break;

	case 0x1D:
		RR_r8(m_registerHL.lo);
		break;

	case 0x1E:
		RR_indirect_HL();
		break;

	case 0x1F:
		RR_r8(m_registerAF.accumulator);
		break;

	case 0x20:
		SLA_r8(m_registerBC.hi);
		break;

	case 0x21:
		SLA_r8(m_registerBC.lo);
		break;

	case 0x22:
		SLA_r8(m_registerDE.hi);
		break;

	case 0x23:
		SLA_r8(m_registerDE.lo);
		break;

	case 0x24:
		SLA_r8(m_registerHL.hi);
		break;

	case 0x25:
		SLA_r8(m_registerHL.lo);
		break;

	case 0x26:
		SLA_indirect_HL();
		break;

	case 0x27:
		SLA_r8(m_registerAF.accumulator);
		break;

	case 0x28:
		SRA_r8(m_registerBC.hi);
		break;

	case 0x29:
		SRA_r8(m_registerBC.lo);
		break;

	case 0x2A:
		SRA_r8(m_registerDE.hi);
		break;

	case 0x2B:
		SRA_r8(m_registerDE.lo);
		break;

	case 0x2C:
		SRA_r8(m_registerHL.hi);
		break;

	case 0x2D:
		SRA_r8(m_registerHL.lo);
		break;

	case 0x2E:
		SRA_indirect_HL();
		break;

	case 0x2F:
		SRA_r8(m_registerAF.accumulator);
		break;

	case 0x30:
		SWAP_r8(m_registerBC.hi);
		break;

	case 0x31:
		SWAP_r8(m_registerBC.lo);
		break;

	case 0x32:
		SWAP_r8(m_registerDE.hi);
		break;

	case 0x33:
		SWAP_r8(m_registerDE.lo);
		break;

	case 0x34:
		SWAP_r8(m_registerHL.hi);
		break;

	case 0x35:
		SWAP_r8(m_registerHL.lo);
		break;

	case 0x36:
		SWAP_indirect_HL();
		break;

	case 0x37:
		SWAP_r8(m_registerAF.accumulator);
		break;

	case 0x38:
		SRL_r8(m_registerBC.hi);
		break;

	case 0x39:
		SRL_r8(m_registerBC.lo);
		break;

	case 0x3A:
		SRL_r8(m_registerDE.hi);
		break;

	case 0x3B:
		SRL_r8(m_registerDE.lo);
		break;

	case 0x3C:
		SRL_r8(m_registerHL.hi);
		break;

	case 0x3D:
		SRL_r8(m_registerHL.lo);
		break;

	case 0x3E:
		SRL_indirect_HL();
		break;

	case 0x3F:
		SRL_r8(m_registerAF.accumulator);
		break;

	case 0x40:
		BIT_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0x41:
		BIT_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0x42:
		BIT_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0x43:
		BIT_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0x44:
		BIT_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0x45:
		BIT_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0x46:
		BIT_indirect_HL(BitSelect::B0);
		break;

	case 0x47:
		BIT_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0x48:
		BIT_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0x49:
		BIT_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0x4A:
		BIT_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0x4B:
		BIT_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0x4C:
		BIT_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0x4D:
		BIT_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0x4E:
		BIT_indirect_HL(BitSelect::B1);
		break;

	case 0x4F:
		BIT_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0x50:
		BIT_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0x51:
		BIT_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0x52:
		BIT_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0x53:
		BIT_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0x54:
		BIT_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0x55:
		BIT_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0x56:
		BIT_indirect_HL(BitSelect::B2);
		break;

	case 0x57:
		BIT_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0x58:
		BIT_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0x59:
		BIT_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0x5A:
		BIT_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0x5B:
		BIT_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0x5C:
		BIT_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0x5D:
		BIT_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0x5E:
		BIT_indirect_HL(BitSelect::B3);
		break;

	case 0x5F:
		BIT_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0x60:
		BIT_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0x61:
		BIT_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0x62:
		BIT_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0x63:
		BIT_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0x64:
		BIT_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0x65:
		BIT_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0x66:
		BIT_indirect_HL(BitSelect::B4);
		break;

	case 0x67:
		BIT_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0x68:
		BIT_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0x69:
		BIT_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0x6A:
		BIT_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0x6B:
		BIT_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0x6C:
		BIT_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0x6D:
		BIT_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0x6E:
		BIT_indirect_HL(BitSelect::B5);
		break;

	case 0x6F:
		BIT_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0x70:
		BIT_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0x71:
		BIT_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0x72:
		BIT_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0x73:
		BIT_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0x74:
		BIT_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0x75:
		BIT_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0x76:
		BIT_indirect_HL(BitSelect::B6);
		break;

	case 0x77:
		BIT_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0x78:
		BIT_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0x79:
		BIT_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0x7A:
		BIT_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0x7B:
		BIT_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0x7C:
		BIT_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0x7D:
		BIT_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0x7E:
		BIT_indirect_HL(BitSelect::B7);
		break;

	case 0x7F:
		BIT_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	case 0x80:
		RES_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0x81:
		RES_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0x82:
		RES_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0x83:
		RES_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0x84:
		RES_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0x85:
		RES_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0x86:
		RES_indirect_HL(BitSelect::B0);
		break;

	case 0x87:
		RES_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0x88:
		RES_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0x89:
		RES_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0x8A:
		RES_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0x8B:
		RES_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0x8C:
		RES_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0x8D:
		RES_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0x8E:
		RES_indirect_HL(BitSelect::B1);
		break;

	case 0x8F:
		RES_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0x90:
		RES_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0x91:
		RES_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0x92:
		RES_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0x93:
		RES_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0x94:
		RES_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0x95:
		RES_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0x96:
		RES_indirect_HL(BitSelect::B2);
		break;

	case 0x97:
		RES_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0x98:
		RES_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0x99:
		RES_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0x9A:
		RES_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0x9B:
		RES_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0x9C:
		RES_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0x9D:
		RES_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0x9E:
		RES_indirect_HL(BitSelect::B3);
		break;

	case 0x9F:
		RES_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0xA0:
		RES_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0xA1:
		RES_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0xA2:
		RES_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0xA3:
		RES_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0xA4:
		RES_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0xA5:
		RES_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0xA6:
		RES_indirect_HL(BitSelect::B4);
		break;

	case 0xA7:
		RES_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0xA8:
		RES_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0xA9:
		RES_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0xAA:
		RES_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0xAB:
		RES_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0xAC:
		RES_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0xAD:
		RES_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0xAE:
		RES_indirect_HL(BitSelect::B5);
		break;

	case 0xAF:
		RES_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0xB0:
		RES_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0xB1:
		RES_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0xB2:
		RES_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0xB3:
		RES_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0xB4:
		RES_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0xB5:
		RES_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0xB6:
		RES_indirect_HL(BitSelect::B6);
		break;

	case 0xB7:
		RES_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0xB8:
		RES_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0xB9:
		RES_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0xBA:
		RES_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0xBB:
		RES_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0xBC:
		RES_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0xBD:
		RES_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0xBE:
		RES_indirect_HL(BitSelect::B7);
		break;

	case 0xBF:
		RES_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	case 0xC0:
		SET_r8(BitSelect::B0, m_registerBC.hi);
		break;

	case 0xC1:
		SET_r8(BitSelect::B0, m_registerBC.lo);
		break;

	case 0xC2:
		SET_r8(BitSelect::B0, m_registerDE.hi);
		break;

	case 0xC3:
		SET_r8(BitSelect::B0, m_registerDE.lo);
		break;

	case 0xC4:
		SET_r8(BitSelect::B0, m_registerHL.hi);
		break;

	case 0xC5:
		SET_r8(BitSelect::B0, m_registerHL.lo);
		break;

	case 0xC6:
		SET_indirect_HL(BitSelect::B0);
		break;

	case 0xC7:
		SET_r8(BitSelect::B0, m_registerAF.accumulator);
		break;

	case 0xC8:
		SET_r8(BitSelect::B1, m_registerBC.hi);
		break;

	case 0xC9:
		SET_r8(BitSelect::B1, m_registerBC.lo);
		break;

	case 0xCA:
		SET_r8(BitSelect::B1, m_registerDE.hi);
		break;

	case 0xCB:
		SET_r8(BitSelect::B1, m_registerDE.lo);
		break;

	case 0xCC:
		SET_r8(BitSelect::B1, m_registerHL.hi);
		break;

	case 0xCD:
		SET_r8(BitSelect::B1, m_registerHL.lo);
		break;

	case 0xCE:
		SET_indirect_HL(BitSelect::B1);
		break;

	case 0xCF:
		SET_r8(BitSelect::B1, m_registerAF.accumulator);
		break;

	case 0xD0:
		SET_r8(BitSelect::B2, m_registerBC.hi);
		break;

	case 0xD1:
		SET_r8(BitSelect::B2, m_registerBC.lo);
		break;

	case 0xD2:
		SET_r8(BitSelect::B2, m_registerDE.hi);
		break;

	case 0xD3:
		SET_r8(BitSelect::B2, m_registerDE.lo);
		break;

	case 0xD4:
		SET_r8(BitSelect::B2, m_registerHL.hi);
		break;

	case 0xD5:
		SET_r8(BitSelect::B2, m_registerHL.lo);
		break;

	case 0xD6:
		SET_indirect_HL(BitSelect::B2);
		break;

	case 0xD7:
		SET_r8(BitSelect::B2, m_registerAF.accumulator);
		break;

	case 0xD8:
		SET_r8(BitSelect::B3, m_registerBC.hi);
		break;

	case 0xD9:
		SET_r8(BitSelect::B3, m_registerBC.lo);
		break;

	case 0xDA:
		SET_r8(BitSelect::B3, m_registerDE.hi);
		break;

	case 0xDB:
		SET_r8(BitSelect::B3, m_registerDE.lo);
		break;

	case 0xDC:
		SET_r8(BitSelect::B3, m_registerHL.hi);
		break;

	case 0xDD:
		SET_r8(BitSelect::B3, m_registerHL.lo);
		break;

	case 0xDE:
		SET_indirect_HL(BitSelect::B3);
		break;

	case 0xDF:
		SET_r8(BitSelect::B3, m_registerAF.accumulator);
		break;

	case 0xE0:
		SET_r8(BitSelect::B4, m_registerBC.hi);
		break;

	case 0xE1:
		SET_r8(BitSelect::B4, m_registerBC.lo);
		break;

	case 0xE2:
		SET_r8(BitSelect::B4, m_registerDE.hi);
		break;

	case 0xE3:
		SET_r8(BitSelect::B4, m_registerDE.lo);
		break;

	case 0xE4:
		SET_r8(BitSelect::B4, m_registerHL.hi);
		break;

	case 0xE5:
		SET_r8(BitSelect::B4, m_registerHL.lo);
		break;

	case 0xE6:
		SET_indirect_HL(BitSelect::B4);
		break;

	case 0xE7:
		SET_r8(BitSelect::B4, m_registerAF.accumulator);
		break;

	case 0xE8:
		SET_r8(BitSelect::B5, m_registerBC.hi);
		break;

	case 0xE9:
		SET_r8(BitSelect::B5, m_registerBC.lo);
		break;

	case 0xEA:
		SET_r8(BitSelect::B5, m_registerDE.hi);
		break;

	case 0xEB:
		SET_r8(BitSelect::B5, m_registerDE.lo);
		break;

	case 0xEC:
		SET_r8(BitSelect::B5, m_registerHL.hi);
		break;

	case 0xED:
		SET_r8(BitSelect::B5, m_registerHL.lo);
		break;

	case 0xEE:
		SET_indirect_HL(BitSelect::B5);
		break;

	case 0xEF:
		SET_r8(BitSelect::B5, m_registerAF.accumulator);
		break;

	case 0xF0:
		SET_r8(BitSelect::B6, m_registerBC.hi);
		break;

	case 0xF1:
		SET_r8(BitSelect::B6, m_registerBC.lo);
		break;

	case 0xF2:
		SET_r8(BitSelect::B6, m_registerDE.hi);
		break;

	case 0xF3:
		SET_r8(BitSelect::B6, m_registerDE.lo);
		break;

	case 0xF4:
		SET_r8(BitSelect::B6, m_registerHL.hi);
		break;

	case 0xF5:
		SET_r8(BitSelect::B6, m_registerHL.lo);
		break;

	case 0xF6:
		SET_indirect_HL(BitSelect::B6);
		break;

	case 0xF7:
		SET_r8(BitSelect::B6, m_registerAF.accumulator);
		break;

	case 0xF8:
		SET_r8(BitSelect::B7, m_registerBC.hi);
		break;

	case 0xF9:
		SET_r8(BitSelect::B7, m_registerBC.lo);
		break;

	case 0xFA:
		SET_r8(BitSelect::B7, m_registerDE.hi);
		break;

	case 0xFB:
		SET_r8(BitSelect::B7, m_registerDE.lo);
		break;

	case 0xFC:
		SET_r8(BitSelect::B7, m_registerHL.hi);
		break;

	case 0xFD:
		SET_r8(BitSelect::B7, m_registerHL.lo);
		break;

	case 0xFE:
		SET_indirect_HL(BitSelect::B7);
		break;

	case 0xFF:
		SET_r8(BitSelect::B7, m_registerAF.accumulator);
		break;

	default:
		break;
	}
}

void Sm83::LDH_indirect_n8_A()
{
	uint8_t  offset  = cpuFetch();
	uint16_t address = 0xFF00 + offset;
	m_bus.cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LDH_indirect_C_A()
{
	uint16_t address = 0xFF00 + m_registerBC.lo;
	m_bus.cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LDH_A_indirect_n8()
{
	uint8_t  offset          = cpuFetch();
	uint16_t address         = 0xFF00 + offset;
	m_registerAF.accumulator = m_bus.cpuRead(address);
}

void Sm83::LDH_A_indirect_C()
{
	uint16_t address         = 0xFF00 + m_registerBC.lo;
	m_registerAF.accumulator = m_bus.cpuRead(address);
}

void Sm83::LD_SP_HL()
{
	cpuTickM();
	m_stackPointer = (m_registerHL.hi << 8) | m_registerHL.lo;
}

void Sm83::LD_HL_SP_i8()
{
	uint8_t  offset = cpuFetch();
	uint16_t sum    = static_cast<uint16_t>(m_stackPointer + static_cast<int8_t>(offset));

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = ((m_stackPointer & 0xF) + (offset & 0xF)) >> 4;
	m_registerAF.flags.C = ((m_stackPointer & 0xFF) + offset) >> 8;

	cpuTickM();
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
	uint8_t lo     = cpuFetch();
	uint8_t hi     = cpuFetch();
	m_stackPointer = lo | (hi << 8);
}
void Sm83::LD_indirect_r16_r8(Sm83Register &dest, uint8_t &src)
{
	uint16_t address = (dest.hi << 8) | dest.lo;
	m_bus.cpuWrite(address, src);
}

void Sm83::LD_indirect_n16_A()
{
	uint8_t  lo      = cpuFetch();
	uint8_t  hi      = cpuFetch();
	uint16_t address = lo | (hi << 8);
	m_bus.cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LD_A_indirect_n16()
{
	uint8_t  lo              = cpuFetch();
	uint8_t  hi              = cpuFetch();
	uint16_t address         = lo | (hi << 8);
	m_registerAF.accumulator = m_bus.cpuRead(address);
}

void Sm83::LD_A_indirect_r16(Sm83Register &src)
{
	uint16_t address         = (src.hi << 8) | src.lo;
	m_registerAF.accumulator = m_bus.cpuRead(address);
}

void Sm83::LD_indirect_HL_n8()
{
	uint8_t  value   = cpuFetch();
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus.cpuWrite(address, value);
}

void Sm83::LD_r8_indirect_HL(uint8_t &dest)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	dest             = m_bus.cpuRead(address);
}

void Sm83::LD_indirect_HLI_A()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus.cpuWrite(address, m_registerAF.accumulator);
	address += 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_indirect_HLD_A()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_bus.cpuWrite(address, m_registerAF.accumulator);
	address -= 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_A_indirect_HLI()
{
	uint16_t address         = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_registerAF.accumulator = m_bus.cpuRead(address);
	address += 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_A_indirect_HLD()
{
	uint16_t address         = (m_registerHL.hi << 8) | m_registerHL.lo;
	m_registerAF.accumulator = m_bus.cpuRead(address);
	address -= 1;
	m_registerHL.hi = (address >> 8) & 0xFF;
	m_registerHL.lo = address & 0xFF;
}

void Sm83::LD_indirect_n16_SP()
{
	// fetch lo byte then hi byte
	uint8_t  lo      = cpuFetch();
	uint8_t  hi      = cpuFetch();
	uint16_t address = lo | (hi << 8);

	// store lo byte the hi byte at next address
	m_bus.cpuWrite(address, m_stackPointer & 0xFF);
	m_bus.cpuWrite(address + 1, (m_stackPointer & 0xFF00) >> 8);
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
	dest.lo        = value & 0xFF;
	dest.hi        = (value >> 8) & 0xFF;
}

void Sm83::INC_SP()
{
	m_stackPointer += 1;
}

void Sm83::INC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  value   = m_bus.cpuRead(address);
	INC_r8(value);
	m_bus.cpuWrite(address, value);
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
	uint8_t  value   = m_bus.cpuRead(address);
	DEC_r8(value);
	m_bus.cpuWrite(address, value);
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
	m_registerAF.flags.C     = (0x100 & shiftedVal) >> 8;
	m_registerAF.accumulator = static_cast<uint8_t>(shiftedVal);

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RRA()
{
	uint16_t shiftedVal = m_registerAF.accumulator >> 1;
	shiftedVal |= m_registerAF.flags.C << 7;
	m_registerAF.flags.C     = m_registerAF.accumulator & 0x1;
	m_registerAF.accumulator = static_cast<uint8_t>(shiftedVal);

	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RLC_r8(uint8_t &dest)
{
	m_registerAF.flags.C = (dest & 0x80) >> 7;
	dest                 = dest << 1;
	dest |= m_registerAF.flags.C;

	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RLC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	RLC_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::RRC_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest & 0x1;
	dest                 = dest >> 1;
	dest |= m_registerAF.flags.C << 7;

	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
}

void Sm83::RRC_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	RRC_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::RL_r8(uint8_t &dest)
{
	uint8_t shiftedValue = static_cast<uint8_t>((dest << 1) | m_registerAF.flags.C);

	m_registerAF.flags.C = (dest & 0x80) >> 7;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.Z = shiftedValue == 0;
	dest                 = shiftedValue;
}

void Sm83::RL_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	RL_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::RR_r8(uint8_t &dest)
{
	uint8_t shiftedValue = static_cast<uint8_t>((m_registerAF.flags.C << 7) | (dest >> 1));

	m_registerAF.flags.C = dest & 0x1;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.Z = shiftedValue == 0;
	dest                 = shiftedValue;
}

void Sm83::RR_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	RR_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::SLA_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest >> 7;
	dest                 = dest << 1;
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::SLA_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	SLA_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::SRA_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest & 0x1;
	dest                 = (dest & 0x80) | (dest >> 1);
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::SRA_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	SRA_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::SRL_r8(uint8_t &dest)
{
	m_registerAF.flags.C = dest & 0x1;
	dest                 = dest >> 1;
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.N = 0;
}

void Sm83::SRL_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	SRL_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::SWAP_r8(uint8_t &dest)
{
	dest                 = (dest >> 4) | (dest << 4);
	m_registerAF.flags.Z = dest == 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = 0;
	m_registerAF.flags.C = 0;
}

void Sm83::SWAP_indirect_HL()
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	SWAP_r8(dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::ADD_SP_i8()
{
	uint8_t operand = cpuFetch();

	cpuTickM();
	m_registerAF.flags.Z = 0;
	m_registerAF.flags.N = 0;
	m_registerAF.flags.H = ((m_stackPointer & 0xF) + (operand & 0xF)) >> 4;
	m_registerAF.flags.C = ((m_stackPointer & 0xFF) + (operand)) >> 8;

	cpuTickM();
	m_stackPointer = static_cast<uint16_t>(m_stackPointer + static_cast<int8_t>(operand));
}

void Sm83::ADD_HL_r16(Sm83Register &operand)
{
	uint16_t hl  = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint16_t op  = (operand.hi << 8) | operand.lo;
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
	uint16_t hl  = (m_registerHL.hi << 8) | m_registerHL.lo;
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	SBC_A_r8(operand);
}

void Sm83::JR_i8()
{
	int8_t offset = static_cast<int8_t>(cpuFetch());
	cpuTickM();
	m_programCounter += offset;
}

void Sm83::JR_CC_i8(bool condition)
{
	int8_t offset = static_cast<int8_t>(cpuFetch());

	// 1 extra m-cycle on jump taken
	// 1 m-cycle == 4 t-cycles
	if (condition)
	{
		cpuTickM();
		m_programCounter += offset;
	}
}

void Sm83::JP_CC_n16(bool condition)
{
	uint8_t  lo      = cpuFetch();
	uint8_t  hi      = cpuFetch();
	uint16_t address = lo | (hi << 8);

	if (condition)
	{
		cpuTickM();
		m_programCounter = address;
	}
}

void Sm83::JP_n16()
{
	uint8_t  lo      = cpuFetch();
	uint8_t  hi      = cpuFetch();
	uint16_t address = lo | (hi << 8);

	cpuTickM();
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
	m_registerAF.flags.N     = 1;
	m_registerAF.flags.H     = 1;
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
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
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	CP_A_r8(operand);
}

void Sm83::RET()
{
	uint8_t lo = m_bus.cpuRead(m_stackPointer++);
	uint8_t hi = m_bus.cpuRead(m_stackPointer++);

	cpuTickM();
	m_programCounter = (hi << 8) | lo;
}

void Sm83::RET_CC(bool condition)
{
	cpuTickM();
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
	uint8_t lo = m_bus.cpuRead(m_stackPointer++);
	uint8_t hi = m_bus.cpuRead(m_stackPointer++);
	dest.lo    = lo;
	dest.hi    = hi;
}

void Sm83::POP_AF()
{
	uint8_t lo = m_bus.cpuRead(m_stackPointer++);
	uint8_t hi = m_bus.cpuRead(m_stackPointer++);

	m_registerAF.accumulator = hi;
	m_registerAF.flags.setFlagsU8(lo);
}

void Sm83::PUSH_r16(Sm83Register &dest)
{
	cpuTickM();
	m_bus.cpuWrite(--m_stackPointer, dest.hi);
	m_bus.cpuWrite(--m_stackPointer, dest.lo);
}

void Sm83::PUSH_AF()
{
	cpuTickM();
	m_bus.cpuWrite(--m_stackPointer, m_registerAF.accumulator);
	m_bus.cpuWrite(--m_stackPointer, m_registerAF.flags.getFlagsU8());
}

void Sm83::CALL_n16()
{
	uint8_t  lo          = cpuFetch();
	uint8_t  hi          = cpuFetch();
	uint16_t callAddress = lo | (hi << 8);

	cpuTickM();

	// save hi byte of pc to stack followed by the lo byte
	m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
	m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

	m_programCounter = callAddress;
}

void Sm83::CALL_CC_n16(bool condition)
{
	uint8_t  lo          = cpuFetch();
	uint8_t  hi          = cpuFetch();
	uint16_t callAddress = lo | (hi << 8);

	if (condition)
	{
		cpuTickM();

		// save hi byte of pc to stack followed by the lo byte
		m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
		m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

		m_programCounter = callAddress;
	}
}

void Sm83::RST(RstVector vec)
{
	cpuTickM();
	m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
	m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

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
	uint8_t  dest    = m_bus.cpuRead(address);
	BIT_r8(b, dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::RES_r8(BitSelect b, uint8_t &dest)
{
	dest = dest & ~static_cast<uint8_t>(b);
}

void Sm83::RES_indirect_HL(BitSelect b)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	RES_r8(b, dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::SET_r8(BitSelect b, uint8_t &dest)
{
	dest = dest | static_cast<uint8_t>(b);
}

void Sm83::SET_indirect_HL(BitSelect b)
{
	uint16_t address = (m_registerHL.hi << 8) | m_registerHL.lo;
	uint8_t  dest    = m_bus.cpuRead(address);
	SET_r8(b, dest);
	m_bus.cpuWrite(address, dest);
}

void Sm83::DI()
{
	m_ime = false;
}

void Sm83::EI()
{
}
