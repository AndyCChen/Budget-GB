#include "sm83.h"
#include "fmt/format.h"
#include <cstdint>
#include <iterator>

Sm83::Sm83(Bus &bus)
	: m_bus(bus), m_opcodeLogger(255)
{
	initDMG();

	m_tCycleTicks = 0;
	m_ime         = false;
	m_logEnable   = true;
}

void Sm83::runInstruction()
{
	if (m_logEnable)
	{
		m_opcodeLogger.begin(m_programCounter);
	}

	uint8_t opcode = cpuFetch_u8();
	decodeExecute(opcode);
}

uint8_t Sm83::cpuFetch_u8()
{
	uint8_t value = m_bus.cpuRead(m_programCounter++);
	m_opcodeLogger.appendOpcodeByte(value);
	opcodeOperand = value;
	return value;
}

uint16_t Sm83::cpuFetch_u16()
{
	uint8_t lo = m_bus.cpuRead(m_programCounter++);
	uint8_t hi = m_bus.cpuRead(m_programCounter++);
	m_opcodeLogger.appendOpcodeByte(lo);
	m_opcodeLogger.appendOpcodeByte(hi);
	uint16_t value = static_cast<uint16_t>((hi << 8) | lo);
	opcodeOperand  = value;
	return value;
}

void Sm83::formatToOpcodeString(const std::string &format, uint16_t arg)
{
	if (!m_logEnable)
		return;

	m_opcodeLogger.setOpcodeFormat(format, arg);
	// fmt::format_to(std::back_inserter(m_instructionBuffer[m_instructionBufferPosition].m_opcodeFormat), format, std::forward<T>(args)...);
}

void Sm83::formatToOpcodeString(const std::string &format)
{
	if (!m_logEnable)
		return;

	m_opcodeLogger.setOpcodeFormat(format);
}

void Sm83::decodeExecute(uint8_t opcode)
{
	switch (opcode)
	{

	case 0x00:
		formatToOpcodeString("NOP");
		break;

	case 0x01:
		LD_r16_n16(m_registerBC);
		formatToOpcodeString("LD BC, {:04X}", opcodeOperand);
		break;

	case 0x02:
		LD_indirect_r16_r8(m_registerBC, m_registerAF.accumulator);
		formatToOpcodeString("LD (BC), A");
		break;

	case 0x03:
		INC_r16(m_registerBC);
		formatToOpcodeString("INC BC");
		break;

	case 0x04:
		INC_r8(m_registerBC.hi);
		formatToOpcodeString("INC B");
		break;

	case 0x05:
		DEC_r8(m_registerBC.hi);
		formatToOpcodeString("DEC B");
		break;

	case 0x06:
		LD_r8_n8(m_registerBC.hi);
		formatToOpcodeString("LD B, {:02X}", opcodeOperand);
		break;

	case 0x07:
		RLCA();
		formatToOpcodeString("RLCA");
		break;

	case 0x08:
		LD_indirect_n16_SP();
		formatToOpcodeString("LD ({:04X}), SP", opcodeOperand);
		break;

	case 0x09:
		ADD_HL_r16(m_registerBC);
		formatToOpcodeString("ADD HL, BC");
		break;

	case 0x0A:
		LD_A_indirect_r16(m_registerBC);
		formatToOpcodeString("LD A, (BC)");
		break;

	case 0x0B:
		DEC_r16(m_registerBC);
		formatToOpcodeString("DEC BC");
		break;

	case 0x0C:
		INC_r8(m_registerBC.lo);
		formatToOpcodeString("INC C");
		break;

	case 0x0D:
		DEC_r8(m_registerBC.lo);
		formatToOpcodeString("DEC C");
		break;

	case 0x0E:
		LD_r8_n8(m_registerBC.lo);
		formatToOpcodeString("LD C, {:02X}", opcodeOperand);
		break;

	case 0x0F:
		RRCA();
		formatToOpcodeString("RRCA");
		break;

	case 0x10:
		formatToOpcodeString("STOP");
		break;

	case 0x11:
		LD_r16_n16(m_registerDE);
		formatToOpcodeString("LD DE, {:04X}", opcodeOperand);
		break;

	case 0x12:
		LD_indirect_r16_r8(m_registerDE, m_registerAF.accumulator);
		formatToOpcodeString("LD (DE), A");
		break;

	case 0x13:
		INC_r16(m_registerDE);
		formatToOpcodeString("INC DE");
		break;

	case 0x14:
		INC_r8(m_registerDE.hi);
		formatToOpcodeString("INC D");
		break;

	case 0x15:
		DEC_r8(m_registerDE.hi);
		formatToOpcodeString("DEC D");
		break;

	case 0x16:
		LD_r8_n8(m_registerDE.hi);
		formatToOpcodeString("LD, D, {:02X}", opcodeOperand);
		break;

	case 0x17:
		RLA();
		formatToOpcodeString("RLA");
		break;

	case 0x18:
		JR_i8();
		formatToOpcodeString("JR {:04X}", computedJumpAddress);
		break;

	case 0x19:
		ADD_HL_r16(m_registerDE);
		formatToOpcodeString("ADD HL, DE");
		break;

	case 0x1A:
		LD_A_indirect_r16(m_registerDE);
		formatToOpcodeString("LD A, (DE)");
		break;

	case 0x1B:
		DEC_r16(m_registerDE);
		formatToOpcodeString("DEC DE");
		break;

	case 0x1C:
		INC_r8(m_registerDE.lo);
		formatToOpcodeString("INC E");
		break;

	case 0x1D:
		DEC_r8(m_registerDE.lo);
		formatToOpcodeString("DEC E");
		break;

	case 0x1E:
		LD_r8_n8(m_registerDE.lo);
		formatToOpcodeString("LD E, {:02X}", opcodeOperand);
		break;

	case 0x1F:
		RRA();
		formatToOpcodeString("RRA");
		break;

	case 0x20:
		JR_CC_i8(!m_registerAF.flags.Z);
		formatToOpcodeString("JR NZ, {:04X}", computedJumpAddress);
		break;

	case 0x21:
		LD_r16_n16(m_registerHL);
		formatToOpcodeString("LD HL, {:04X}", opcodeOperand);
		break;

	case 0x22:
		LD_indirect_HLI_A();
		formatToOpcodeString("LD (HL+), A");
		break;

	case 0x23:
		INC_r16(m_registerHL);
		formatToOpcodeString("INC HL");
		break;

	case 0x24:
		INC_r8(m_registerHL.hi);
		formatToOpcodeString("INC H");
		break;

	case 0x25:
		DEC_r8(m_registerHL.hi);
		formatToOpcodeString("DEC H");
		break;

	case 0x26:
		LD_r8_n8(m_registerHL.hi);
		formatToOpcodeString("LD H, {:02X}", opcodeOperand);
		break;

	case 0x27:
		DAA();
		formatToOpcodeString("DAA");
		break;

	case 0x28:
		JR_CC_i8(m_registerAF.flags.Z);
		formatToOpcodeString("JR Z, {:04X}", computedJumpAddress);
		break;

	case 0x29:
		ADD_HL_r16(m_registerHL);
		formatToOpcodeString("ADD HL, HL");
		break;

	case 0x2A:
		LD_A_indirect_HLI();
		formatToOpcodeString("LD A, (HL+)");
		break;

	case 0x2B:
		DEC_r16(m_registerHL);
		formatToOpcodeString("DEC HL");
		break;

	case 0x2C:
		INC_r8(m_registerHL.lo);
		formatToOpcodeString("INC L");
		break;

	case 0x2D:
		DEC_r8(m_registerHL.lo);
		formatToOpcodeString("DEC L");
		break;

	case 0x2E:
		LD_r8_n8(m_registerHL.lo);
		formatToOpcodeString("LD L, {:02X}", opcodeOperand);
		break;

	case 0x2F:
		CPL();
		formatToOpcodeString("CPL");
		break;

	case 0x30:
		JR_CC_i8(!m_registerAF.flags.C);
		formatToOpcodeString("JR NC, {:04X}", computedJumpAddress);
		break;

	case 0x31:
		LD_SP_n16();
		formatToOpcodeString("LD SP, {:04X}", opcodeOperand);
		break;

	case 0x32:
		LD_indirect_HLD_A();
		formatToOpcodeString("LD (HL-), A");
		break;

	case 0x33:
		INC_SP();
		formatToOpcodeString("INC SP");
		break;

	case 0x34:
		INC_indirect_HL();
		formatToOpcodeString("INC (HL)");
		break;

	case 0x35:
		DEC_indirect_HL();
		formatToOpcodeString("DEC (HL)");
		break;

	case 0x36:
		LD_indirect_HL_n8();
		formatToOpcodeString("LD (HL), {:02X}", opcodeOperand);
		break;

	case 0x37:
		SCF();
		formatToOpcodeString("SCF");
		break;

	case 0x38:
		JR_CC_i8(m_registerAF.flags.C);
		formatToOpcodeString("JR C, {:04X}", computedJumpAddress);
		break;

	case 0x39:
		ADD_HL_SP();
		formatToOpcodeString("ADD HL, SP");
		break;

	case 0x3A:
		LD_A_indirect_HLD();
		formatToOpcodeString("LD A, (HL-)");
		break;

	case 0x3B:
		DEC_SP();
		formatToOpcodeString("DEC SP");
		break;

	case 0x3C:
		INC_r8(m_registerAF.accumulator);
		formatToOpcodeString("INC A");
		break;

	case 0x3D:
		DEC_r8(m_registerAF.accumulator);
		formatToOpcodeString("DEC A");
		break;

	case 0x3E:
		LD_r8_n8(m_registerAF.accumulator);
		formatToOpcodeString("LD A, {:02X}", opcodeOperand);
		break;

	case 0x3F:
		CCF();
		formatToOpcodeString("CCF");
		break;

	case 0x40:
		LD_r8_r8(m_registerBC.hi, m_registerBC.hi);
		formatToOpcodeString("LD B, B");
		break;

	case 0x41:
		LD_r8_r8(m_registerBC.hi, m_registerBC.lo);
		formatToOpcodeString("LD B, C");
		break;

	case 0x42:
		LD_r8_r8(m_registerBC.hi, m_registerDE.hi);
		formatToOpcodeString("LD B, D");
		break;

	case 0x43:
		LD_r8_r8(m_registerBC.hi, m_registerDE.lo);
		formatToOpcodeString("LD B, E");
		break;

	case 0x44:
		LD_r8_r8(m_registerBC.hi, m_registerHL.hi);
		formatToOpcodeString("LD B, H");
		break;

	case 0x45:
		LD_r8_r8(m_registerBC.hi, m_registerHL.lo);
		formatToOpcodeString("LD B, L");
		break;

	case 0x46:
		LD_r8_indirect_HL(m_registerBC.hi);
		formatToOpcodeString("LD B, (HL)");
		break;

	case 0x47:
		LD_r8_r8(m_registerBC.hi, m_registerAF.accumulator);
		formatToOpcodeString("LD B, A");
		break;

	case 0x48:
		LD_r8_r8(m_registerBC.lo, m_registerBC.hi);
		formatToOpcodeString("LD C, B");
		break;

	case 0x49:
		LD_r8_r8(m_registerBC.lo, m_registerBC.lo);
		formatToOpcodeString("LD C, C");
		break;

	case 0x4A:
		LD_r8_r8(m_registerBC.lo, m_registerDE.hi);
		formatToOpcodeString("LD C, D");
		break;

	case 0x4B:
		LD_r8_r8(m_registerBC.lo, m_registerDE.lo);
		formatToOpcodeString("LD C, E");
		break;

	case 0x4C:
		LD_r8_r8(m_registerBC.lo, m_registerHL.hi);
		formatToOpcodeString("LD C, H");
		break;

	case 0x4D:
		LD_r8_r8(m_registerBC.lo, m_registerHL.lo);
		formatToOpcodeString("LD C, L");
		break;

	case 0x4E:
		LD_r8_indirect_HL(m_registerBC.lo);
		formatToOpcodeString("LD C, (HL)");
		break;

	case 0x4F:
		LD_r8_r8(m_registerBC.lo, m_registerAF.accumulator);
		formatToOpcodeString("LD C, A");
		break;

	case 0x50:
		LD_r8_r8(m_registerDE.hi, m_registerBC.hi);
		formatToOpcodeString("LD D, B");
		break;

	case 0x51:
		LD_r8_r8(m_registerDE.hi, m_registerBC.lo);
		formatToOpcodeString("LD D, C");
		break;

	case 0x52:
		LD_r8_r8(m_registerDE.hi, m_registerDE.hi);
		formatToOpcodeString("LD D, D");
		break;

	case 0x53:
		LD_r8_r8(m_registerDE.hi, m_registerDE.lo);
		formatToOpcodeString("LD D, E");
		break;

	case 0x54:
		LD_r8_r8(m_registerDE.hi, m_registerHL.hi);
		formatToOpcodeString("LD D, H");
		break;

	case 0x55:
		LD_r8_r8(m_registerDE.hi, m_registerHL.lo);
		formatToOpcodeString("LD D, L");
		break;

	case 0x56:
		LD_r8_indirect_HL(m_registerDE.hi);
		formatToOpcodeString("LD D, (HL)");
		break;

	case 0x57:
		LD_r8_r8(m_registerDE.hi, m_registerAF.accumulator);
		formatToOpcodeString("LD D, A");
		break;

	case 0x58:
		LD_r8_r8(m_registerDE.lo, m_registerBC.hi);
		formatToOpcodeString("LD E, B");
		break;

	case 0x59:
		LD_r8_r8(m_registerDE.lo, m_registerBC.lo);
		formatToOpcodeString("LD E, C");
		break;

	case 0x5A:
		LD_r8_r8(m_registerDE.lo, m_registerDE.hi);
		formatToOpcodeString("LD E, D");
		break;

	case 0x5B:
		LD_r8_r8(m_registerDE.lo, m_registerDE.lo);
		formatToOpcodeString("LD E, E");
		break;

	case 0x5C:
		LD_r8_r8(m_registerDE.lo, m_registerHL.hi);
		formatToOpcodeString("LD E, H");
		break;

	case 0x5D:
		LD_r8_r8(m_registerDE.lo, m_registerHL.lo);
		formatToOpcodeString("LD E, L");
		break;

	case 0x5E:
		LD_r8_indirect_HL(m_registerDE.lo);
		formatToOpcodeString("LD E, (HL)");
		break;

	case 0x5F:
		LD_r8_r8(m_registerDE.lo, m_registerAF.accumulator);
		formatToOpcodeString("LD E, A");
		break;

	case 0x60:
		LD_r8_r8(m_registerHL.hi, m_registerBC.hi);
		formatToOpcodeString("LD H, B");
		break;

	case 0x61:
		LD_r8_r8(m_registerHL.hi, m_registerBC.lo);
		formatToOpcodeString("LD H, C");
		break;

	case 0x62:
		LD_r8_r8(m_registerHL.hi, m_registerDE.hi);
		formatToOpcodeString("LD H, D");
		break;

	case 0x63:
		LD_r8_r8(m_registerHL.hi, m_registerDE.lo);
		formatToOpcodeString("LD H, E");
		break;

	case 0x64:
		LD_r8_r8(m_registerHL.hi, m_registerHL.hi);
		formatToOpcodeString("LD H, H");
		break;

	case 0x65:
		LD_r8_r8(m_registerHL.hi, m_registerHL.lo);
		formatToOpcodeString("LD H, L");
		break;

	case 0x66:
		LD_r8_indirect_HL(m_registerHL.hi);
		formatToOpcodeString("LD H, (HL)");
		break;

	case 0x67:
		LD_r8_r8(m_registerHL.hi, m_registerAF.accumulator);
		formatToOpcodeString("LD H, A");
		break;

	case 0x68:
		LD_r8_r8(m_registerHL.lo, m_registerBC.hi);
		formatToOpcodeString("LD L, B");
		break;

	case 0x69:
		LD_r8_r8(m_registerHL.lo, m_registerBC.lo);
		formatToOpcodeString("LD L, C");
		break;

	case 0x6A:
		LD_r8_r8(m_registerHL.lo, m_registerDE.hi);
		formatToOpcodeString("LD L, D");
		break;

	case 0x6B:
		LD_r8_r8(m_registerHL.lo, m_registerDE.lo);
		formatToOpcodeString("LD L, E");
		break;

	case 0x6C:
		LD_r8_r8(m_registerHL.lo, m_registerHL.hi);
		formatToOpcodeString("LD L, H");
		break;

	case 0x6D:
		LD_r8_r8(m_registerHL.lo, m_registerHL.lo);
		formatToOpcodeString("LD L, L");
		break;

	case 0x6E:
		LD_r8_indirect_HL(m_registerHL.lo);
		formatToOpcodeString("LD L, (HL)");
		break;

	case 0x6F:
		LD_r8_r8(m_registerHL.lo, m_registerAF.accumulator);
		formatToOpcodeString("LD L, A");
		break;

	case 0x70:
		LD_indirect_r16_r8(m_registerHL, m_registerBC.hi);
		formatToOpcodeString("LD (HL), B");
		break;

	case 0x71:
		LD_indirect_r16_r8(m_registerHL, m_registerBC.lo);
		formatToOpcodeString("LD (HL), C");
		break;

	case 0x72:
		LD_indirect_r16_r8(m_registerHL, m_registerDE.hi);
		formatToOpcodeString("LD (HL), D");
		break;

	case 0x73:
		LD_indirect_r16_r8(m_registerHL, m_registerDE.lo);
		formatToOpcodeString("LD (HL), E");
		break;

	case 0x74:
		LD_indirect_r16_r8(m_registerHL, m_registerHL.hi);
		formatToOpcodeString("LD (HL), H");
		break;

	case 0x75:
		LD_indirect_r16_r8(m_registerHL, m_registerHL.lo);
		formatToOpcodeString("LD (HL), L");
		break;

	case 0x76:
		formatToOpcodeString("HALT");
		break;

	case 0x77:
		LD_indirect_r16_r8(m_registerHL, m_registerAF.accumulator);
		formatToOpcodeString("LD (HL), A");
		break;

	case 0x78:
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.hi);
		formatToOpcodeString("LD A, B");
		break;

	case 0x79:
		LD_r8_r8(m_registerAF.accumulator, m_registerBC.lo);
		formatToOpcodeString("LD A, C");
		break;

	case 0x7A:
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.hi);
		formatToOpcodeString("LD A, D");
		break;

	case 0x7B:
		LD_r8_r8(m_registerAF.accumulator, m_registerDE.lo);
		formatToOpcodeString("LD A, E");
		break;

	case 0x7C:
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.hi);
		formatToOpcodeString("LD A, H");
		break;

	case 0x7D:
		LD_r8_r8(m_registerAF.accumulator, m_registerHL.lo);
		formatToOpcodeString("LD A,L");
		break;

	case 0x7E:
		LD_r8_indirect_HL(m_registerAF.accumulator);
		formatToOpcodeString("LD A, (HL)");
		break;

	case 0x7F:
		LD_r8_r8(m_registerAF.accumulator, m_registerAF.accumulator);
		formatToOpcodeString("LD A, A");
		break;

	case 0x80:
		ADD_A_r8(m_registerBC.hi);
		formatToOpcodeString("ADD A, B");
		break;

	case 0x81:
		ADD_A_r8(m_registerBC.lo);
		formatToOpcodeString("ADD A, C");
		break;

	case 0x82:
		ADD_A_r8(m_registerDE.hi);
		formatToOpcodeString("ADD A, D");
		break;

	case 0x83:
		ADD_A_r8(m_registerDE.lo);
		formatToOpcodeString("ADD A, E");
		break;

	case 0x84:
		ADD_A_r8(m_registerHL.hi);
		formatToOpcodeString("ADD A, H");
		break;

	case 0x85:
		ADD_A_r8(m_registerHL.lo);
		formatToOpcodeString("ADD A, L");
		break;

	case 0x86:
		ADD_A_indirect_HL();
		formatToOpcodeString("ADD A, (HL)");
		break;

	case 0x87:
		ADD_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("ADD A, A");
		break;

	case 0x88:
		ADC_A_r8(m_registerBC.hi);
		formatToOpcodeString("ADC A, B");
		break;

	case 0x89:
		ADC_A_r8(m_registerBC.lo);
		formatToOpcodeString("ADC A, C");
		break;

	case 0x8A:
		ADC_A_r8(m_registerDE.hi);
		formatToOpcodeString("ADC A, D");
		break;

	case 0x8B:
		ADC_A_r8(m_registerDE.lo);
		formatToOpcodeString("ADC A, E");
		break;

	case 0x8C:
		ADC_A_r8(m_registerHL.hi);
		formatToOpcodeString("ADC A, H");
		break;

	case 0x8D:
		ADC_A_r8(m_registerHL.lo);
		formatToOpcodeString("ADC A, L");
		break;

	case 0x8E:
		ADC_A_indirect_HL();
		formatToOpcodeString("ADC A, (HL)");
		break;

	case 0x8F:
		ADC_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("ADC A, A");
		break;

	case 0x90:
		SUB_A_r8(m_registerBC.hi);
		formatToOpcodeString("SUB A, B");
		break;

	case 0x91:
		SUB_A_r8(m_registerBC.lo);
		formatToOpcodeString("SUB A, C");
		break;

	case 0x92:
		SUB_A_r8(m_registerDE.hi);
		formatToOpcodeString("SUB A, D");
		break;

	case 0x93:
		SUB_A_r8(m_registerDE.lo);
		formatToOpcodeString("SUB A, E");
		break;

	case 0x94:
		SUB_A_r8(m_registerHL.hi);
		formatToOpcodeString("SUB A, H");
		break;

	case 0x95:
		SUB_A_r8(m_registerHL.lo);
		formatToOpcodeString("SUB A, L");
		break;

	case 0x96:
		SUB_A_indirect_HL();
		formatToOpcodeString("SUB A, (HL)");
		break;

	case 0x97:
		SUB_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("SUB A, A");
		break;

	case 0x98:
		SBC_A_r8(m_registerBC.hi);
		formatToOpcodeString("SBC A, B");
		break;

	case 0x99:
		SBC_A_r8(m_registerBC.lo);
		formatToOpcodeString("SBC A, C");
		break;

	case 0x9A:
		SBC_A_r8(m_registerDE.hi);
		formatToOpcodeString("SBC A, D");
		break;

	case 0x9B:
		SBC_A_r8(m_registerDE.lo);
		formatToOpcodeString("SBC A, E");
		break;

	case 0x9C:
		SBC_A_r8(m_registerHL.hi);
		formatToOpcodeString("SBC A, H");
		break;

	case 0x9D:
		SBC_A_r8(m_registerHL.lo);
		formatToOpcodeString("SBC A, L");
		break;

	case 0x9E:
		SBC_A_indirect_HL();
		formatToOpcodeString("SBC A, (HL)");
		break;

	case 0x9F:
		SBC_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("SBC A, A");
		break;

	case 0xA0:
		AND_A_r8(m_registerBC.hi);
		formatToOpcodeString("AND A, B");
		break;

	case 0xA1:
		AND_A_r8(m_registerBC.lo);
		formatToOpcodeString("AND A, C");
		break;

	case 0xA2:
		AND_A_r8(m_registerDE.hi);
		formatToOpcodeString("AND A, D");
		break;

	case 0xA3:
		AND_A_r8(m_registerDE.lo);
		formatToOpcodeString("AND A, E");
		break;

	case 0xA4:
		AND_A_r8(m_registerHL.hi);
		formatToOpcodeString("AND A, H");
		break;

	case 0xA5:
		AND_A_r8(m_registerHL.lo);
		formatToOpcodeString("AND A, L");
		break;

	case 0xA6:
		AND_A_indirect_HL();
		formatToOpcodeString("AND A, (HL)");
		break;

	case 0xA7:
		AND_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("AND A, A");
		break;

	case 0xA8:
		XOR_A_r8(m_registerBC.hi);
		formatToOpcodeString("XOR A, B");
		break;

	case 0xA9:
		XOR_A_r8(m_registerBC.lo);
		formatToOpcodeString("XOR A, C");
		break;

	case 0xAA:
		XOR_A_r8(m_registerDE.hi);
		formatToOpcodeString("XOR A, D");
		break;

	case 0xAB:
		XOR_A_r8(m_registerDE.lo);
		formatToOpcodeString("XOR A, E");
		break;

	case 0xAC:
		XOR_A_r8(m_registerHL.hi);
		formatToOpcodeString("XOR A, H");
		break;

	case 0xAD:
		XOR_A_r8(m_registerHL.lo);
		formatToOpcodeString("XOR A, L");
		break;

	case 0xAE:
		XOR_A_indirect_HL();
		formatToOpcodeString("XOR A, (HL)");
		break;

	case 0xAF:
		XOR_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("XOR A, A");
		break;

	case 0xB0:
		OR_A_r8(m_registerBC.hi);
		formatToOpcodeString("OR A, B");
		break;

	case 0xB1:
		OR_A_r8(m_registerBC.lo);
		formatToOpcodeString("OR A, C");
		break;

	case 0xB2:
		OR_A_r8(m_registerDE.hi);
		formatToOpcodeString("OR A, D");
		break;

	case 0xB3:
		OR_A_r8(m_registerDE.lo);
		formatToOpcodeString("OR A, E");
		break;

	case 0xB4:
		OR_A_r8(m_registerHL.hi);
		formatToOpcodeString("OR A, H");
		break;

	case 0xB5:
		OR_A_r8(m_registerHL.lo);
		formatToOpcodeString("OR A, L");
		break;

	case 0xB6:
		OR_A_indirect_HL();
		formatToOpcodeString("OR A, (HL)");
		break;

	case 0xB7:
		OR_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("OR A, A");
		break;

	case 0xB8:
		CP_A_r8(m_registerBC.hi);
		formatToOpcodeString("CP A, B");
		break;

	case 0xB9:
		CP_A_r8(m_registerBC.lo);
		formatToOpcodeString("CP A, C");
		break;

	case 0xBA:
		CP_A_r8(m_registerDE.hi);
		formatToOpcodeString("CP A, D");
		break;

	case 0xBB:
		CP_A_r8(m_registerDE.lo);
		formatToOpcodeString("CP A, E");
		break;

	case 0xBC:
		CP_A_r8(m_registerHL.hi);
		formatToOpcodeString("CP A, H");
		break;

	case 0xBD:
		CP_A_r8(m_registerHL.lo);
		formatToOpcodeString("CP A, L");
		break;

	case 0xBE:
		CP_A_indirect_HL();
		formatToOpcodeString("CP A, (HL)");
		break;

	case 0xBF:
		CP_A_r8(m_registerAF.accumulator);
		formatToOpcodeString("CP A, A");
		break;

	case 0xC0:
		RET_CC(!m_registerAF.flags.Z);
		formatToOpcodeString("RET NZ");
		break;

	case 0xC1:
		POP_r16(m_registerBC);
		formatToOpcodeString("POP BC");
		break;

	case 0xC2:
		JP_CC_n16(!m_registerAF.flags.Z);
		formatToOpcodeString("JP NZ, {:04X}", opcodeOperand);
		break;

	case 0xC3:
		JP_n16();
		formatToOpcodeString("JP {:04X}", opcodeOperand);
		break;

	case 0xC4:
		CALL_CC_n16(!m_registerAF.flags.Z);
		formatToOpcodeString("CALL NZ, {:04X}", opcodeOperand);
		break;

	case 0xC5:
		PUSH_r16(m_registerBC);
		formatToOpcodeString("PUSH BC");
		break;

	case 0xC6:
		ADD_A_n8();
		formatToOpcodeString("ADD A, {:02X}", opcodeOperand);
		break;

	case 0xC7:
		RST(RstVector::H00);
		formatToOpcodeString("RST 00h");
		break;

	case 0xC8:
		RET_CC(m_registerAF.flags.Z);
		formatToOpcodeString("RET Z");
		break;

	case 0xC9:
		RET();
		formatToOpcodeString("RET");
		break;

	case 0xCA:
		JP_CC_n16(m_registerAF.flags.Z);
		formatToOpcodeString("JP Z, {:04X}", opcodeOperand);
		break;

	// Prefix mode, decode opcode with second opcode table
	case 0xCB:
		decodeExecutePrefixedMode(cpuFetch_u8());
		break;

	case 0xCC:
		CALL_CC_n16(m_registerAF.flags.Z);
		formatToOpcodeString("CALL Z, {:04X}", opcodeOperand);
		break;

	case 0xCD:
		CALL_n16();
		formatToOpcodeString("CALL {:04X}", opcodeOperand);
		break;

	case 0xCE:
		ADC_A_n8();
		formatToOpcodeString("ADC A, {:02X}", opcodeOperand);
		break;

	case 0xCF:
		RST(RstVector::H08);
		formatToOpcodeString("RST 08h");
		break;

	case 0xD0:
		RET_CC(!m_registerAF.flags.C);
		formatToOpcodeString("RET NC");
		break;

	case 0xD1:
		POP_r16(m_registerDE);
		formatToOpcodeString("POP DE");
		break;

	case 0xD2:
		JP_CC_n16(!m_registerAF.flags.C);
		formatToOpcodeString("JP NC, {:04X}", opcodeOperand);
		break;

	case 0xD3:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xD4:
		CALL_CC_n16(!m_registerAF.flags.C);
		formatToOpcodeString("CALL NC, {:04X}", opcodeOperand);
		break;

	case 0xD5:
		PUSH_r16(m_registerDE);
		formatToOpcodeString("PUSH DE");
		break;

	case 0xD6:
		SUB_A_n8();
		formatToOpcodeString("SUB A, {:02X}", opcodeOperand);
		break;

	case 0xD7:
		RST(RstVector::H10);
		formatToOpcodeString("RST 10h");
		break;

	case 0xD8:
		RET_CC(m_registerAF.flags.C);
		formatToOpcodeString("RET C");
		break;

	case 0xD9:
		RETI();
		formatToOpcodeString("RETI");
		break;

	case 0xDA:
		JP_CC_n16(m_registerAF.flags.C);
		formatToOpcodeString("JP C, {:04X}", opcodeOperand);
		break;

	case 0xDB:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xDC:
		CALL_CC_n16(m_registerAF.flags.C);
		formatToOpcodeString("CALL C, {:04X}", opcodeOperand);
		break;

	case 0xDD:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xDE:
		SBC_A_n8();
		formatToOpcodeString("SBC A, {:02X}", opcodeOperand);
		break;

	case 0xDF:
		RST(RstVector::H18);
		formatToOpcodeString("RST 18h");
		break;

	case 0xE0:
		LDH_indirect_n8_A();
		formatToOpcodeString("LD (FF00 + {:02X}), A", opcodeOperand);
		break;

	case 0xE1:
		POP_r16(m_registerHL);
		formatToOpcodeString("POP HL");
		break;

	case 0xE2:
		LDH_indirect_C_A();
		formatToOpcodeString("LD $(FF00 + C), A");
		break;

	case 0xE3:
	case 0xE4:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xE5:
		PUSH_r16(m_registerHL);
		formatToOpcodeString("PUSH HL");
		break;

	case 0xE6:
		AND_A_n8();
		formatToOpcodeString("AND A, {:02X}", opcodeOperand);
		break;

	case 0xE7:
		RST(RstVector::H20);
		formatToOpcodeString("RST 20h");
		break;

	case 0xE8: {
		ADD_SP_i8();
		uint8_t value = static_cast<uint8_t>(opcodeOperand);
		if (value & 0x80)
			formatToOpcodeString("ADD SP, -{:02X}", static_cast<uint8_t>(~value + 1));
		else
			formatToOpcodeString("ADD SP,  {:02X}", value);

		break;
	}

	case 0xE9:
		JP_HL();
		formatToOpcodeString("JP HL");
		break;

	case 0xEA:
		LD_indirect_n16_A();
		formatToOpcodeString("LD ({:04X}), A", opcodeOperand);
		break;

	case 0xEB:
	case 0xEC:
	case 0xED:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xEE:
		XOR_A_n8();
		formatToOpcodeString("XOR A, {:02X}", opcodeOperand);
		break;

	case 0xEF:
		RST(RstVector::H28);
		formatToOpcodeString("RST 28h");
		break;

	case 0xF0:
		LDH_A_indirect_n8();
		formatToOpcodeString("LD A, (FF00 + {:02X})", opcodeOperand);
		break;

	case 0xF1:
		POP_AF();
		formatToOpcodeString("POP AF");
		break;

	case 0xF2:
		LDH_A_indirect_C();
		formatToOpcodeString("LD A, (FF00 + C)");
		break;

	case 0xF3:
		DI();
		formatToOpcodeString("DI");
		break;

	case 0xF4:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xF5:
		PUSH_AF();
		formatToOpcodeString("PUSH AF");
		break;

	case 0xF6:
		OR_A_n8();
		formatToOpcodeString("OR A, {:02X}", opcodeOperand);
		break;

	case 0xF7:
		RST(RstVector::H30);
		formatToOpcodeString("RST 30h");
		break;

	case 0xF8: {
		LD_HL_SP_i8();
		uint8_t value = static_cast<uint8_t>(opcodeOperand);
		if (value & 0x80)
			formatToOpcodeString("LD HL, SP - {:02X}", static_cast<uint8_t>(~value + 1));
		else
			formatToOpcodeString("LD HL, SP + {:02X}", value);

		break;
	}

	case 0xF9:
		LD_SP_HL();
		formatToOpcodeString("LD SP, HL");
		break;

	case 0xFA:
		LD_A_indirect_n16();
		formatToOpcodeString("LD A, {:04X}", opcodeOperand);
		break;

	case 0xFB:
		EI();
		formatToOpcodeString("EI");
		break;

	case 0xFC:
	case 0xFD:
		m_programCounter -= 1;
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xFE:
		CP_A_n8();
		formatToOpcodeString("CP A, {:02X}", opcodeOperand);
		break;

	case 0xFF:
		RST(RstVector::H38);
		formatToOpcodeString("RST 38h");
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
		formatToOpcodeString("RLC B");
		break;

	case 0x01:
		RLC_r8(m_registerBC.lo);
		formatToOpcodeString("RLC C");
		break;

	case 0x02:
		RLC_r8(m_registerDE.hi);
		formatToOpcodeString("RLC D");
		break;

	case 0x03:
		RLC_r8(m_registerDE.lo);
		formatToOpcodeString("RLC E");
		break;

	case 0x04:
		RLC_r8(m_registerHL.hi);
		formatToOpcodeString("RLC H");
		break;

	case 0x05:
		RLC_r8(m_registerHL.lo);
		formatToOpcodeString("RLC L");
		break;

	case 0x06:
		RLC_indirect_HL();
		formatToOpcodeString("RLC (HL)");
		break;

	case 0x07:
		RLC_r8(m_registerAF.accumulator);
		formatToOpcodeString("RLC A");
		break;

	case 0x08:
		RRC_r8(m_registerBC.hi);
		formatToOpcodeString("RRC B");
		break;

	case 0x09:
		RRC_r8(m_registerBC.lo);
		formatToOpcodeString("RRC C");
		break;

	case 0x0A:
		RRC_r8(m_registerDE.hi);
		formatToOpcodeString("RRC D");
		break;

	case 0x0B:
		RRC_r8(m_registerDE.lo);
		formatToOpcodeString("RRC E");
		break;

	case 0x0C:
		RRC_r8(m_registerHL.hi);
		formatToOpcodeString("RRC H");
		break;

	case 0x0D:
		RRC_r8(m_registerHL.lo);
		formatToOpcodeString("RRC L");
		break;

	case 0x0E:
		RRC_indirect_HL();
		formatToOpcodeString("RRC (HL)");
		break;

	case 0x0F:
		RRC_r8(m_registerAF.accumulator);
		formatToOpcodeString("RRC A");
		break;

	case 0x10:
		RL_r8(m_registerBC.hi);
		formatToOpcodeString("RL B");
		break;

	case 0x11:
		RL_r8(m_registerBC.lo);
		formatToOpcodeString("RL C");
		break;

	case 0x12:
		RL_r8(m_registerDE.hi);
		formatToOpcodeString("RL D");
		break;

	case 0x13:
		RL_r8(m_registerDE.lo);
		formatToOpcodeString("RL E");
		break;

	case 0x14:
		RL_r8(m_registerHL.hi);
		formatToOpcodeString("RL H");
		break;

	case 0x15:
		RL_r8(m_registerHL.lo);
		formatToOpcodeString("RL L");
		break;

	case 0x16:
		RL_indirect_HL();
		formatToOpcodeString("RL (HL)");
		break;

	case 0x17:
		RL_r8(m_registerAF.accumulator);
		formatToOpcodeString("RL A");
		break;

	case 0x18:
		RR_r8(m_registerBC.hi);
		formatToOpcodeString("RR B");
		break;

	case 0x19:
		RR_r8(m_registerBC.lo);
		formatToOpcodeString("RR C");
		break;

	case 0x1A:
		RR_r8(m_registerDE.hi);
		formatToOpcodeString("RR D");
		break;

	case 0x1B:
		RR_r8(m_registerDE.lo);
		formatToOpcodeString("RR E");
		break;

	case 0x1C:
		RR_r8(m_registerHL.hi);
		formatToOpcodeString("RR H");
		break;

	case 0x1D:
		RR_r8(m_registerHL.lo);
		formatToOpcodeString("RR L");
		break;

	case 0x1E:
		RR_indirect_HL();
		formatToOpcodeString("RR (HL)");
		break;

	case 0x1F:
		RR_r8(m_registerAF.accumulator);
		formatToOpcodeString("RR A");
		break;

	case 0x20:
		SLA_r8(m_registerBC.hi);
		formatToOpcodeString("SLA B");
		break;

	case 0x21:
		SLA_r8(m_registerBC.lo);
		formatToOpcodeString("SLA C");
		break;

	case 0x22:
		SLA_r8(m_registerDE.hi);
		formatToOpcodeString("SLA D");
		break;

	case 0x23:
		SLA_r8(m_registerDE.lo);
		formatToOpcodeString("SLA E");
		break;

	case 0x24:
		SLA_r8(m_registerHL.hi);
		formatToOpcodeString("SLA H");
		break;

	case 0x25:
		SLA_r8(m_registerHL.lo);
		formatToOpcodeString("SLA L");
		break;

	case 0x26:
		SLA_indirect_HL();
		formatToOpcodeString("SLA (HL)");
		break;

	case 0x27:
		SLA_r8(m_registerAF.accumulator);
		formatToOpcodeString("SLA A");
		break;

	case 0x28:
		SRA_r8(m_registerBC.hi);
		formatToOpcodeString("SRA B");
		break;

	case 0x29:
		SRA_r8(m_registerBC.lo);
		formatToOpcodeString("SRA C");
		break;

	case 0x2A:
		SRA_r8(m_registerDE.hi);
		formatToOpcodeString("SRA D");
		break;

	case 0x2B:
		SRA_r8(m_registerDE.lo);
		formatToOpcodeString("SRA E");
		break;

	case 0x2C:
		SRA_r8(m_registerHL.hi);
		formatToOpcodeString("SRA H");
		break;

	case 0x2D:
		SRA_r8(m_registerHL.lo);
		formatToOpcodeString("SRA L");
		break;

	case 0x2E:
		SRA_indirect_HL();
		formatToOpcodeString("SRA (HL)");
		break;

	case 0x2F:
		SRA_r8(m_registerAF.accumulator);
		formatToOpcodeString("SRA A");
		break;

	case 0x30:
		SWAP_r8(m_registerBC.hi);
		formatToOpcodeString("SWAP B");
		break;

	case 0x31:
		SWAP_r8(m_registerBC.lo);
		formatToOpcodeString("SWAP C");
		break;

	case 0x32:
		SWAP_r8(m_registerDE.hi);
		formatToOpcodeString("SWAP D");
		break;

	case 0x33:
		SWAP_r8(m_registerDE.lo);
		formatToOpcodeString("SWAP E");
		break;

	case 0x34:
		SWAP_r8(m_registerHL.hi);
		formatToOpcodeString("SWAP H");
		break;

	case 0x35:
		SWAP_r8(m_registerHL.lo);
		formatToOpcodeString("SWAP L");
		break;

	case 0x36:
		SWAP_indirect_HL();
		formatToOpcodeString("SWAP (HL)");
		break;

	case 0x37:
		SWAP_r8(m_registerAF.accumulator);
		formatToOpcodeString("SWAP A");
		break;

	case 0x38:
		SRL_r8(m_registerBC.hi);
		formatToOpcodeString("SRL B");
		break;

	case 0x39:
		SRL_r8(m_registerBC.lo);
		formatToOpcodeString("SRL C");
		break;

	case 0x3A:
		SRL_r8(m_registerDE.hi);
		formatToOpcodeString("SRL D");
		break;

	case 0x3B:
		SRL_r8(m_registerDE.lo);
		formatToOpcodeString("SRL E");
		break;

	case 0x3C:
		SRL_r8(m_registerHL.hi);
		formatToOpcodeString("SRL H");
		break;

	case 0x3D:
		SRL_r8(m_registerHL.lo);
		formatToOpcodeString("SRL L");
		break;

	case 0x3E:
		SRL_indirect_HL();
		formatToOpcodeString("SRL (HL)");
		break;

	case 0x3F:
		SRL_r8(m_registerAF.accumulator);
		formatToOpcodeString("SRL A");
		break;

	case 0x40:
		BIT_r8(BitSelect::B0, m_registerBC.hi);
		formatToOpcodeString("BIT 0, B");
		break;

	case 0x41:
		BIT_r8(BitSelect::B0, m_registerBC.lo);
		formatToOpcodeString("BIT 0, C");
		break;

	case 0x42:
		BIT_r8(BitSelect::B0, m_registerDE.hi);
		formatToOpcodeString("BIT 0, D");
		break;

	case 0x43:
		BIT_r8(BitSelect::B0, m_registerDE.lo);
		formatToOpcodeString("BIT 0, E");
		break;

	case 0x44:
		BIT_r8(BitSelect::B0, m_registerHL.hi);
		formatToOpcodeString("BIT 0, H");
		break;

	case 0x45:
		BIT_r8(BitSelect::B0, m_registerHL.lo);
		formatToOpcodeString("BIT 0, L");
		break;

	case 0x46:
		BIT_indirect_HL(BitSelect::B0);
		formatToOpcodeString("BIT 0, (HL)");
		break;

	case 0x47:
		BIT_r8(BitSelect::B0, m_registerAF.accumulator);
		formatToOpcodeString("BIT 0, A");
		break;

	case 0x48:
		BIT_r8(BitSelect::B1, m_registerBC.hi);
		formatToOpcodeString("BIT 1, B");
		break;

	case 0x49:
		BIT_r8(BitSelect::B1, m_registerBC.lo);
		formatToOpcodeString("BIT 1, C");
		break;

	case 0x4A:
		BIT_r8(BitSelect::B1, m_registerDE.hi);
		formatToOpcodeString("BIT 1, D");
		break;

	case 0x4B:
		BIT_r8(BitSelect::B1, m_registerDE.lo);
		formatToOpcodeString("BIT 1, E");
		break;

	case 0x4C:
		BIT_r8(BitSelect::B1, m_registerHL.hi);
		formatToOpcodeString("BIT 1, H");
		break;

	case 0x4D:
		BIT_r8(BitSelect::B1, m_registerHL.lo);
		formatToOpcodeString("BIT 1, L");
		break;

	case 0x4E:
		BIT_indirect_HL(BitSelect::B1);
		formatToOpcodeString("BIT 1, (HL)");
		break;

	case 0x4F:
		BIT_r8(BitSelect::B1, m_registerAF.accumulator);
		formatToOpcodeString("BIT 1, A");
		break;

	case 0x50:
		BIT_r8(BitSelect::B2, m_registerBC.hi);
		formatToOpcodeString("BIT 2, B");
		break;

	case 0x51:
		BIT_r8(BitSelect::B2, m_registerBC.lo);
		formatToOpcodeString("BIT 2, C");
		break;

	case 0x52:
		BIT_r8(BitSelect::B2, m_registerDE.hi);
		formatToOpcodeString("BIT 2, D");
		break;

	case 0x53:
		BIT_r8(BitSelect::B2, m_registerDE.lo);
		formatToOpcodeString("BIT 2, E");
		break;

	case 0x54:
		BIT_r8(BitSelect::B2, m_registerHL.hi);
		formatToOpcodeString("BIT 2, H");
		break;

	case 0x55:
		BIT_r8(BitSelect::B2, m_registerHL.lo);
		formatToOpcodeString("BIT 2, L");
		break;

	case 0x56:
		BIT_indirect_HL(BitSelect::B2);
		formatToOpcodeString("BIT 2, (HL)");
		break;

	case 0x57:
		BIT_r8(BitSelect::B2, m_registerAF.accumulator);
		formatToOpcodeString("BIT 2, A");
		break;

	case 0x58:
		BIT_r8(BitSelect::B3, m_registerBC.hi);
		formatToOpcodeString("BIT 3, B");
		break;

	case 0x59:
		BIT_r8(BitSelect::B3, m_registerBC.lo);
		formatToOpcodeString("BIT 3, C");
		break;

	case 0x5A:
		BIT_r8(BitSelect::B3, m_registerDE.hi);
		formatToOpcodeString("BIT 3, D");
		break;

	case 0x5B:
		BIT_r8(BitSelect::B3, m_registerDE.lo);
		formatToOpcodeString("BIT 3, E");
		break;

	case 0x5C:
		BIT_r8(BitSelect::B3, m_registerHL.hi);
		formatToOpcodeString("BIT 3, H");
		break;

	case 0x5D:
		BIT_r8(BitSelect::B3, m_registerHL.lo);
		formatToOpcodeString("BIT 3, L");
		break;

	case 0x5E:
		BIT_indirect_HL(BitSelect::B3);
		formatToOpcodeString("BIT 3, (HL)");
		break;

	case 0x5F:
		BIT_r8(BitSelect::B3, m_registerAF.accumulator);
		formatToOpcodeString("BIT 3, A");
		break;

	case 0x60:
		BIT_r8(BitSelect::B4, m_registerBC.hi);
		formatToOpcodeString("BIT 4, B");
		break;

	case 0x61:
		BIT_r8(BitSelect::B4, m_registerBC.lo);
		formatToOpcodeString("BIT 4, C");
		break;

	case 0x62:
		BIT_r8(BitSelect::B4, m_registerDE.hi);
		formatToOpcodeString("BIT 4, D");
		break;

	case 0x63:
		BIT_r8(BitSelect::B4, m_registerDE.lo);
		formatToOpcodeString("BIT 4, E");
		break;

	case 0x64:
		BIT_r8(BitSelect::B4, m_registerHL.hi);
		formatToOpcodeString("BIT 4, H");
		break;

	case 0x65:
		BIT_r8(BitSelect::B4, m_registerHL.lo);
		formatToOpcodeString("BIT 4, L");
		break;

	case 0x66:
		BIT_indirect_HL(BitSelect::B4);
		formatToOpcodeString("BIT 4, (HL)");
		break;

	case 0x67:
		BIT_r8(BitSelect::B4, m_registerAF.accumulator);
		formatToOpcodeString("BIT 4, A");
		break;

	case 0x68:
		BIT_r8(BitSelect::B5, m_registerBC.hi);
		formatToOpcodeString("BIT 5, B");
		break;

	case 0x69:
		BIT_r8(BitSelect::B5, m_registerBC.lo);
		formatToOpcodeString("BIT 5, C");
		break;

	case 0x6A:
		BIT_r8(BitSelect::B5, m_registerDE.hi);
		formatToOpcodeString("BIT 5, D");
		break;

	case 0x6B:
		BIT_r8(BitSelect::B5, m_registerDE.lo);
		formatToOpcodeString("BIT 5, E");
		break;

	case 0x6C:
		BIT_r8(BitSelect::B5, m_registerHL.hi);
		formatToOpcodeString("BIT 5, H");
		break;

	case 0x6D:
		BIT_r8(BitSelect::B5, m_registerHL.lo);
		formatToOpcodeString("BIT 5, L");
		break;

	case 0x6E:
		BIT_indirect_HL(BitSelect::B5);
		formatToOpcodeString("BIT 5, (HL)");
		break;

	case 0x6F:
		BIT_r8(BitSelect::B5, m_registerAF.accumulator);
		formatToOpcodeString("BIT 5, A");
		break;

	case 0x70:
		BIT_r8(BitSelect::B6, m_registerBC.hi);
		formatToOpcodeString("BIT 6, B");
		break;

	case 0x71:
		BIT_r8(BitSelect::B6, m_registerBC.lo);
		formatToOpcodeString("BIT 6, C");
		break;

	case 0x72:
		BIT_r8(BitSelect::B6, m_registerDE.hi);
		formatToOpcodeString("BIT 6, D");
		break;

	case 0x73:
		BIT_r8(BitSelect::B6, m_registerDE.lo);
		formatToOpcodeString("BIT 6, E");
		break;

	case 0x74:
		BIT_r8(BitSelect::B6, m_registerHL.hi);
		formatToOpcodeString("BIT 6, H");
		break;

	case 0x75:
		BIT_r8(BitSelect::B6, m_registerHL.lo);
		formatToOpcodeString("BIT 6, L");
		break;

	case 0x76:
		BIT_indirect_HL(BitSelect::B6);
		formatToOpcodeString("BIT 6, (HL)");
		break;

	case 0x77:
		BIT_r8(BitSelect::B6, m_registerAF.accumulator);
		formatToOpcodeString("BIT 6, A");
		break;

	case 0x78:
		BIT_r8(BitSelect::B7, m_registerBC.hi);
		formatToOpcodeString("BIT 7, B");
		break;

	case 0x79:
		BIT_r8(BitSelect::B7, m_registerBC.lo);
		formatToOpcodeString("BIT 7, C");
		break;

	case 0x7A:
		BIT_r8(BitSelect::B7, m_registerDE.hi);
		formatToOpcodeString("BIT 7, D");
		break;

	case 0x7B:
		BIT_r8(BitSelect::B7, m_registerDE.lo);
		formatToOpcodeString("BIT 7, E");
		break;

	case 0x7C:
		BIT_r8(BitSelect::B7, m_registerHL.hi);
		formatToOpcodeString("BIT 7, H");
		break;

	case 0x7D:
		BIT_r8(BitSelect::B7, m_registerHL.lo);
		formatToOpcodeString("BIT 7, L");
		break;

	case 0x7E:
		BIT_indirect_HL(BitSelect::B7);
		formatToOpcodeString("BIT 7, (HL)");
		break;

	case 0x7F:
		BIT_r8(BitSelect::B7, m_registerAF.accumulator);
		formatToOpcodeString("BIT 7, A");
		break;

	case 0x80:
		RES_r8(BitSelect::B0, m_registerBC.hi);
		formatToOpcodeString("RES 0, B");
		break;

	case 0x81:
		RES_r8(BitSelect::B0, m_registerBC.lo);
		formatToOpcodeString("RES 0, C");
		break;

	case 0x82:
		RES_r8(BitSelect::B0, m_registerDE.hi);
		formatToOpcodeString("RES 0, D");
		break;

	case 0x83:
		RES_r8(BitSelect::B0, m_registerDE.lo);
		formatToOpcodeString("RES 0, E");
		break;

	case 0x84:
		RES_r8(BitSelect::B0, m_registerHL.hi);
		formatToOpcodeString("RES 0, H");
		break;

	case 0x85:
		RES_r8(BitSelect::B0, m_registerHL.lo);
		formatToOpcodeString("RES 0, L");
		break;

	case 0x86:
		RES_indirect_HL(BitSelect::B0);
		formatToOpcodeString("RES 0, (HL)");
		break;

	case 0x87:
		RES_r8(BitSelect::B0, m_registerAF.accumulator);
		formatToOpcodeString("RES 0, A");
		break;

	case 0x88:
		RES_r8(BitSelect::B1, m_registerBC.hi);
		formatToOpcodeString("RES 1, B");
		break;

	case 0x89:
		RES_r8(BitSelect::B1, m_registerBC.lo);
		formatToOpcodeString("RES 1, C");
		break;

	case 0x8A:
		RES_r8(BitSelect::B1, m_registerDE.hi);
		formatToOpcodeString("RES 1, D");
		break;

	case 0x8B:
		RES_r8(BitSelect::B1, m_registerDE.lo);
		formatToOpcodeString("RES 1, E");
		break;

	case 0x8C:
		RES_r8(BitSelect::B1, m_registerHL.hi);
		formatToOpcodeString("RES 1, H");
		break;

	case 0x8D:
		RES_r8(BitSelect::B1, m_registerHL.lo);
		formatToOpcodeString("RES 1, L");
		break;

	case 0x8E:
		RES_indirect_HL(BitSelect::B1);
		formatToOpcodeString("RES 1, (HL)");
		break;

	case 0x8F:
		RES_r8(BitSelect::B1, m_registerAF.accumulator);
		formatToOpcodeString("RES 1, A");
		break;

	case 0x90:
		RES_r8(BitSelect::B2, m_registerBC.hi);
		formatToOpcodeString("RES 2, B");
		break;

	case 0x91:
		RES_r8(BitSelect::B2, m_registerBC.lo);
		formatToOpcodeString("RES 2, C");
		break;

	case 0x92:
		RES_r8(BitSelect::B2, m_registerDE.hi);
		formatToOpcodeString("RES 2, D");
		break;

	case 0x93:
		RES_r8(BitSelect::B2, m_registerDE.lo);
		formatToOpcodeString("RES 2, E");
		break;

	case 0x94:
		RES_r8(BitSelect::B2, m_registerHL.hi);
		formatToOpcodeString("RES 2, H");
		break;

	case 0x95:
		RES_r8(BitSelect::B2, m_registerHL.lo);
		formatToOpcodeString("RES 2, L");
		break;

	case 0x96:
		RES_indirect_HL(BitSelect::B2);
		formatToOpcodeString("RES 2, (HL)");
		break;

	case 0x97:
		RES_r8(BitSelect::B2, m_registerAF.accumulator);
		formatToOpcodeString("RES 2, A");
		break;

	case 0x98:
		RES_r8(BitSelect::B3, m_registerBC.hi);
		formatToOpcodeString("RES 3, B");
		break;

	case 0x99:
		RES_r8(BitSelect::B3, m_registerBC.lo);
		formatToOpcodeString("RES 3, C");
		break;

	case 0x9A:
		RES_r8(BitSelect::B3, m_registerDE.hi);
		formatToOpcodeString("RES 3, D");
		break;

	case 0x9B:
		RES_r8(BitSelect::B3, m_registerDE.lo);
		formatToOpcodeString("RES 3, E");
		break;

	case 0x9C:
		RES_r8(BitSelect::B3, m_registerHL.hi);
		formatToOpcodeString("RES 3, H");
		break;

	case 0x9D:
		RES_r8(BitSelect::B3, m_registerHL.lo);
		formatToOpcodeString("RES 3, L");
		break;

	case 0x9E:
		RES_indirect_HL(BitSelect::B3);
		formatToOpcodeString("RES 3, (HL)");
		break;

	case 0x9F:
		RES_r8(BitSelect::B3, m_registerAF.accumulator);
		formatToOpcodeString("RES 3, A");
		break;

	case 0xA0:
		RES_r8(BitSelect::B4, m_registerBC.hi);
		formatToOpcodeString("RES 4, B");
		break;

	case 0xA1:
		RES_r8(BitSelect::B4, m_registerBC.lo);
		formatToOpcodeString("RES 4, C");
		break;

	case 0xA2:
		RES_r8(BitSelect::B4, m_registerDE.hi);
		formatToOpcodeString("RES 4, D");
		break;

	case 0xA3:
		RES_r8(BitSelect::B4, m_registerDE.lo);
		formatToOpcodeString("RES 4, E");
		break;

	case 0xA4:
		RES_r8(BitSelect::B4, m_registerHL.hi);
		formatToOpcodeString("RES 4, H");
		break;

	case 0xA5:
		RES_r8(BitSelect::B4, m_registerHL.lo);
		formatToOpcodeString("RES 4, L");
		break;

	case 0xA6:
		RES_indirect_HL(BitSelect::B4);
		formatToOpcodeString("RES 4, (HL)");
		break;

	case 0xA7:
		RES_r8(BitSelect::B4, m_registerAF.accumulator);
		formatToOpcodeString("RES 4, A");
		break;

	case 0xA8:
		RES_r8(BitSelect::B5, m_registerBC.hi);
		formatToOpcodeString("RES 5, B");
		break;

	case 0xA9:
		RES_r8(BitSelect::B5, m_registerBC.lo);
		formatToOpcodeString("RES 5, C");
		break;

	case 0xAA:
		RES_r8(BitSelect::B5, m_registerDE.hi);
		formatToOpcodeString("RES 5, D");
		break;

	case 0xAB:
		RES_r8(BitSelect::B5, m_registerDE.lo);
		formatToOpcodeString("RES 5, E");
		break;

	case 0xAC:
		RES_r8(BitSelect::B5, m_registerHL.hi);
		formatToOpcodeString("RES 5, H");
		break;

	case 0xAD:
		RES_r8(BitSelect::B5, m_registerHL.lo);
		formatToOpcodeString("RES 5, L");
		break;

	case 0xAE:
		RES_indirect_HL(BitSelect::B5);
		formatToOpcodeString("RES 5, (HL)");
		break;

	case 0xAF:
		RES_r8(BitSelect::B5, m_registerAF.accumulator);
		formatToOpcodeString("RES 5, A");
		break;

	case 0xB0:
		RES_r8(BitSelect::B6, m_registerBC.hi);
		formatToOpcodeString("RES 6, B");
		break;

	case 0xB1:
		RES_r8(BitSelect::B6, m_registerBC.lo);
		formatToOpcodeString("RES 6, C");
		break;

	case 0xB2:
		RES_r8(BitSelect::B6, m_registerDE.hi);
		formatToOpcodeString("RES 6, D");
		break;

	case 0xB3:
		RES_r8(BitSelect::B6, m_registerDE.lo);
		formatToOpcodeString("RES 6, E");
		break;

	case 0xB4:
		RES_r8(BitSelect::B6, m_registerHL.hi);
		formatToOpcodeString("RES 6, H");
		break;

	case 0xB5:
		RES_r8(BitSelect::B6, m_registerHL.lo);
		formatToOpcodeString("RES 6, L");
		break;

	case 0xB6:
		RES_indirect_HL(BitSelect::B6);
		formatToOpcodeString("RES 6, (HL)");
		break;

	case 0xB7:
		RES_r8(BitSelect::B6, m_registerAF.accumulator);
		formatToOpcodeString("RES 6, A");
		break;

	case 0xB8:
		RES_r8(BitSelect::B7, m_registerBC.hi);
		formatToOpcodeString("RES 7, B");
		break;

	case 0xB9:
		RES_r8(BitSelect::B7, m_registerBC.lo);
		formatToOpcodeString("RES 7, C");
		break;

	case 0xBA:
		RES_r8(BitSelect::B7, m_registerDE.hi);
		formatToOpcodeString("RES 7, D");
		break;

	case 0xBB:
		RES_r8(BitSelect::B7, m_registerDE.lo);
		formatToOpcodeString("RES 7, E");
		break;

	case 0xBC:
		RES_r8(BitSelect::B7, m_registerHL.hi);
		formatToOpcodeString("RES 7, H");
		break;

	case 0xBD:
		RES_r8(BitSelect::B7, m_registerHL.lo);
		formatToOpcodeString("RES 7, L");
		break;

	case 0xBE:
		RES_indirect_HL(BitSelect::B7);
		formatToOpcodeString("RES 7, (HL)");
		break;

	case 0xBF:
		RES_r8(BitSelect::B7, m_registerAF.accumulator);
		formatToOpcodeString("RES 7, A");
		break;

	case 0xC0:
		SET_r8(BitSelect::B0, m_registerBC.hi);
		formatToOpcodeString("SET 0, B");
		break;

	case 0xC1:
		SET_r8(BitSelect::B0, m_registerBC.lo);
		formatToOpcodeString("SET 0, C");
		break;

	case 0xC2:
		SET_r8(BitSelect::B0, m_registerDE.hi);
		formatToOpcodeString("SET 0, D");
		break;

	case 0xC3:
		SET_r8(BitSelect::B0, m_registerDE.lo);
		formatToOpcodeString("SET 0, E");
		break;

	case 0xC4:
		SET_r8(BitSelect::B0, m_registerHL.hi);
		formatToOpcodeString("SET 0, H");
		break;

	case 0xC5:
		SET_r8(BitSelect::B0, m_registerHL.lo);
		formatToOpcodeString("SET 0, L");
		break;

	case 0xC6:
		SET_indirect_HL(BitSelect::B0);
		formatToOpcodeString("SET 0, (HL)");
		break;

	case 0xC7:
		SET_r8(BitSelect::B0, m_registerAF.accumulator);
		formatToOpcodeString("SET 0, A");
		break;

	case 0xC8:
		SET_r8(BitSelect::B1, m_registerBC.hi);
		formatToOpcodeString("SET 1, B");
		break;

	case 0xC9:
		SET_r8(BitSelect::B1, m_registerBC.lo);
		formatToOpcodeString("SET 1, C");
		break;

	case 0xCA:
		SET_r8(BitSelect::B1, m_registerDE.hi);
		formatToOpcodeString("SET 1, D");
		break;

	case 0xCB:
		SET_r8(BitSelect::B1, m_registerDE.lo);
		formatToOpcodeString("SET 1, E");
		break;

	case 0xCC:
		SET_r8(BitSelect::B1, m_registerHL.hi);
		formatToOpcodeString("SET 1, H");
		break;

	case 0xCD:
		SET_r8(BitSelect::B1, m_registerHL.lo);
		formatToOpcodeString("SET 1, L");
		break;

	case 0xCE:
		SET_indirect_HL(BitSelect::B1);
		formatToOpcodeString("SET 1, (HL)");
		break;

	case 0xCF:
		SET_r8(BitSelect::B1, m_registerAF.accumulator);
		formatToOpcodeString("SET 1, A");
		break;

	case 0xD0:
		SET_r8(BitSelect::B2, m_registerBC.hi);
		formatToOpcodeString("SET 2, B");
		break;

	case 0xD1:
		SET_r8(BitSelect::B2, m_registerBC.lo);
		formatToOpcodeString("SET 2, C");
		break;

	case 0xD2:
		SET_r8(BitSelect::B2, m_registerDE.hi);
		formatToOpcodeString("SET 2, D");
		break;

	case 0xD3:
		SET_r8(BitSelect::B2, m_registerDE.lo);
		formatToOpcodeString("SET 2, E");
		break;

	case 0xD4:
		SET_r8(BitSelect::B2, m_registerHL.hi);
		formatToOpcodeString("SET 2, H");
		break;

	case 0xD5:
		SET_r8(BitSelect::B2, m_registerHL.lo);
		formatToOpcodeString("SET 2, L");
		break;

	case 0xD6:
		SET_indirect_HL(BitSelect::B2);
		formatToOpcodeString("SET 2, (HL)");
		break;

	case 0xD7:
		SET_r8(BitSelect::B2, m_registerAF.accumulator);
		formatToOpcodeString("SET 2, A");
		break;

	case 0xD8:
		SET_r8(BitSelect::B3, m_registerBC.hi);
		formatToOpcodeString("SET 3, B");
		break;

	case 0xD9:
		SET_r8(BitSelect::B3, m_registerBC.lo);
		formatToOpcodeString("SET 3, C");
		break;

	case 0xDA:
		SET_r8(BitSelect::B3, m_registerDE.hi);
		formatToOpcodeString("SET 3, D");
		break;

	case 0xDB:
		SET_r8(BitSelect::B3, m_registerDE.lo);
		formatToOpcodeString("SET 3, E");
		break;

	case 0xDC:
		SET_r8(BitSelect::B3, m_registerHL.hi);
		formatToOpcodeString("SET 3, H");
		break;

	case 0xDD:
		SET_r8(BitSelect::B3, m_registerHL.lo);
		formatToOpcodeString("SET 3, L");
		break;

	case 0xDE:
		SET_indirect_HL(BitSelect::B3);
		formatToOpcodeString("SET 3, (HL)");
		break;

	case 0xDF:
		SET_r8(BitSelect::B3, m_registerAF.accumulator);
		formatToOpcodeString("SET 3, A");
		break;

	case 0xE0:
		SET_r8(BitSelect::B4, m_registerBC.hi);
		formatToOpcodeString("SET 4, B");
		break;

	case 0xE1:
		SET_r8(BitSelect::B4, m_registerBC.lo);
		formatToOpcodeString("SET 4, C");
		break;

	case 0xE2:
		SET_r8(BitSelect::B4, m_registerDE.hi);
		formatToOpcodeString("SET 4, D");
		break;

	case 0xE3:
		SET_r8(BitSelect::B4, m_registerDE.lo);
		formatToOpcodeString("SET 4, E");
		break;

	case 0xE4:
		SET_r8(BitSelect::B4, m_registerHL.hi);
		formatToOpcodeString("SET 4, H");
		break;

	case 0xE5:
		SET_r8(BitSelect::B4, m_registerHL.lo);
		formatToOpcodeString("SET 4, L");
		break;

	case 0xE6:
		SET_indirect_HL(BitSelect::B4);
		formatToOpcodeString("SET 4, (HL)");
		break;

	case 0xE7:
		SET_r8(BitSelect::B4, m_registerAF.accumulator);
		formatToOpcodeString("SET 4, A");
		break;

	case 0xE8:
		SET_r8(BitSelect::B5, m_registerBC.hi);
		formatToOpcodeString("SET 5, B");
		break;

	case 0xE9:
		SET_r8(BitSelect::B5, m_registerBC.lo);
		formatToOpcodeString("SET 5, C");
		break;

	case 0xEA:
		SET_r8(BitSelect::B5, m_registerDE.hi);
		formatToOpcodeString("SET 5, D");
		break;

	case 0xEB:
		SET_r8(BitSelect::B5, m_registerDE.lo);
		formatToOpcodeString("SET 5, E");
		break;

	case 0xEC:
		SET_r8(BitSelect::B5, m_registerHL.hi);
		formatToOpcodeString("SET 5, H");
		break;

	case 0xED:
		SET_r8(BitSelect::B5, m_registerHL.lo);
		formatToOpcodeString("SET 5, L");
		break;

	case 0xEE:
		SET_indirect_HL(BitSelect::B5);
		formatToOpcodeString("SET 5, (HL)");
		break;

	case 0xEF:
		SET_r8(BitSelect::B5, m_registerAF.accumulator);
		formatToOpcodeString("SET 5, A");
		break;

	case 0xF0:
		SET_r8(BitSelect::B6, m_registerBC.hi);
		formatToOpcodeString("SET 6, B");
		break;

	case 0xF1:
		SET_r8(BitSelect::B6, m_registerBC.lo);
		formatToOpcodeString("SET 6, C");
		break;

	case 0xF2:
		SET_r8(BitSelect::B6, m_registerDE.hi);
		formatToOpcodeString("SET 6, D");
		break;

	case 0xF3:
		SET_r8(BitSelect::B6, m_registerDE.lo);
		formatToOpcodeString("SET 6, E");
		break;

	case 0xF4:
		SET_r8(BitSelect::B6, m_registerHL.hi);
		formatToOpcodeString("SET 6, H");
		break;

	case 0xF5:
		SET_r8(BitSelect::B6, m_registerHL.lo);
		formatToOpcodeString("SET 6, L");
		break;

	case 0xF6:
		SET_indirect_HL(BitSelect::B6);
		formatToOpcodeString("SET 6, (HL)");
		break;

	case 0xF7:
		SET_r8(BitSelect::B6, m_registerAF.accumulator);
		formatToOpcodeString("SET 6, A");
		break;

	case 0xF8:
		SET_r8(BitSelect::B7, m_registerBC.hi);
		formatToOpcodeString("SET 7, B");
		break;

	case 0xF9:
		SET_r8(BitSelect::B7, m_registerBC.lo);
		formatToOpcodeString("SET 7, C");
		break;

	case 0xFA:
		SET_r8(BitSelect::B7, m_registerDE.hi);
		formatToOpcodeString("SET 7, D");
		break;

	case 0xFB:
		SET_r8(BitSelect::B7, m_registerDE.lo);
		formatToOpcodeString("SET 7, E");
		break;

	case 0xFC:
		SET_r8(BitSelect::B7, m_registerHL.hi);
		formatToOpcodeString("SET 7, H");
		break;

	case 0xFD:
		SET_r8(BitSelect::B7, m_registerHL.lo);
		formatToOpcodeString("SET 7, L");
		break;

	case 0xFE:
		SET_indirect_HL(BitSelect::B7);
		formatToOpcodeString("SET 7, (HL)");
		break;

	case 0xFF:
		SET_r8(BitSelect::B7, m_registerAF.accumulator);
		formatToOpcodeString("SET 7, A");
		break;

	default:
		break;
	}
}

void Sm83::LDH_indirect_n8_A()
{
	uint8_t  offset  = cpuFetch_u8();
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
	uint8_t  offset          = cpuFetch_u8();
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
	uint8_t  offset = cpuFetch_u8();
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
	dest = cpuFetch_u8();
}

void Sm83::LD_r8_r8(uint8_t &dest, uint8_t &src)
{
	dest = src;
}

void Sm83::LD_r16_n16(Sm83Register &dest)
{
	dest.set_u16(cpuFetch_u16());
}

void Sm83::LD_SP_n16()
{
	m_stackPointer = cpuFetch_u16();
}

void Sm83::LD_indirect_r16_r8(Sm83Register &dest, uint8_t &src)
{
	uint16_t address = (dest.hi << 8) | dest.lo;
	m_bus.cpuWrite(address, src);
}

void Sm83::LD_indirect_n16_A()
{
	uint16_t address = cpuFetch_u16();
	m_bus.cpuWrite(address, m_registerAF.accumulator);
}

void Sm83::LD_A_indirect_n16()
{
	uint16_t address         = cpuFetch_u16();
	m_registerAF.accumulator = m_bus.cpuRead(address);
}

void Sm83::LD_A_indirect_r16(Sm83Register &src)
{
	uint16_t address         = (src.hi << 8) | src.lo;
	m_registerAF.accumulator = m_bus.cpuRead(address);
}

void Sm83::LD_indirect_HL_n8()
{
	uint8_t  value   = cpuFetch_u8();
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
	uint16_t address = cpuFetch_u16();

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
	uint8_t operand = cpuFetch_u8();

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
	uint8_t operand = cpuFetch_u8();
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

	m_registerAF.flags.H     = (((m_registerAF.accumulator & 0xF) + (operand & 0xF) + m_registerAF.flags.C) & 0x10) >> 4;
	m_registerAF.flags.C     = (sum & 0x100) >> 8;
	m_registerAF.flags.N     = 0;
	m_registerAF.flags.Z     = static_cast<uint8_t>(sum) == 0;
	m_registerAF.accumulator = static_cast<uint8_t>(sum);
}

void Sm83::ADC_A_indirect_HL()
{
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	ADC_A_r8(operand);
}

void Sm83::ADC_A_n8()
{
	uint8_t operand = cpuFetch_u8();
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
	uint8_t operand = cpuFetch_u8();
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
	uint8_t operand = cpuFetch_u8();
	SBC_A_r8(operand);
}

void Sm83::SBC_A_indirect_HL()
{
	uint8_t operand = m_bus.cpuRead((m_registerHL.hi << 8) | m_registerHL.lo);
	SBC_A_r8(operand);
}

void Sm83::JR_i8()
{
	int8_t offset       = static_cast<int8_t>(cpuFetch_u8());
	computedJumpAddress = m_programCounter + offset;
	cpuTickM();
	m_programCounter = computedJumpAddress;
}

void Sm83::JR_CC_i8(bool condition)
{
	int8_t offset       = static_cast<int8_t>(cpuFetch_u8());
	computedJumpAddress = m_programCounter + offset;

	// 1 extra m-cycle on jump taken
	// 1 m-cycle == 4 t-cycles
	if (condition)
	{
		cpuTickM();
		m_programCounter = computedJumpAddress;
	}
}

void Sm83::JP_CC_n16(bool condition)
{
	uint16_t address = cpuFetch_u16();

	if (condition)
	{
		cpuTickM();
		m_programCounter = address;
	}
}

void Sm83::JP_n16()
{
	uint16_t address = cpuFetch_u16();

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
	uint8_t operand = cpuFetch_u8();
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
	uint8_t operand = cpuFetch_u8();
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
	uint8_t operand = cpuFetch_u8();
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
	uint8_t operand = cpuFetch_u8();
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
	uint16_t callAddress = cpuFetch_u16();

	cpuTickM();

	// save hi byte of pc to stack followed by the lo byte
	m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter >> 8));
	m_bus.cpuWrite(--m_stackPointer, static_cast<uint8_t>(m_programCounter));

	m_programCounter = callAddress;
}

void Sm83::CALL_CC_n16(bool condition)
{
	uint16_t callAddress = cpuFetch_u16();

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
