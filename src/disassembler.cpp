#include "disassembler.h"
#include <cstdint>

void Disassembler::instructionStep()
{
	m_bufferPosition = (m_bufferPosition + 1) % m_buffer.size();
	m_buffer[m_bufferPosition].clear();

	m_buffer[m_bufferPosition].m_opcodeAddress = fmt::format("{:04X}", m_programCounter);

	uint8_t opcode = fetch_n8();
	disassembleOpcode(opcode);
}

void Disassembler::disassembleOpcode(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x00:
		formatToOpcodeString("NOP");
		break;

	case 0x01:
		formatToOpcodeString("LD BC, {:04X}", fetch_n16());
		break;

	case 0x2:
		formatToOpcodeString("LD (BC), A");
		break;

	case 0x03:
		formatToOpcodeString("INC BC");
		break;

	case 0x04:
		formatToOpcodeString("INC B");
		break;

	case 0x05:
		formatToOpcodeString("DEC B");
		break;

	case 0x06:
		formatToOpcodeString("LD B, {:02X}", fetch_n8());
		break;

	case 0x07:
		formatToOpcodeString("RLCA");
		break;

	case 0x08:
		formatToOpcodeString("LD ({:04X}), SP", fetch_n16());
		break;

	case 0x09:
		formatToOpcodeString("ADD HL, BC");
		break;

	case 0x0A:
		formatToOpcodeString("LD A, (BC)");
		break;

	case 0x0B:
		formatToOpcodeString("DEC BC");
		break;

	case 0x0C:
		formatToOpcodeString("INC C");
		break;

	case 0x0D:
		formatToOpcodeString("DEC C");
		break;

	case 0x0E:
		formatToOpcodeString("LD C, {:02X}", fetch_n8());
		break;

	case 0x0F:
		formatToOpcodeString("RRCA");
		break;

	case 0x10:
		formatToOpcodeString("STOP");
		break;

	case 0x11:
		formatToOpcodeString("LD DE, {:04X}", fetch_n16());
		break;

	case 0x12:
		formatToOpcodeString("LD (DE), A");
		break;

	case 0x13:
		formatToOpcodeString("INC DE");
		break;

	case 0x14:
		formatToOpcodeString("INC D");
		break;

	case 0x15:
		formatToOpcodeString("DEC D");
		break;

	case 0x16:
		formatToOpcodeString("LD, D, {:02X}", fetch_n8());
		break;

	case 0x17:
		formatToOpcodeString("RLA");
		break;

	case 0x18:
		formatToOpcodeString("JR {:04X}", computeRelativeJump());
		break;

	case 0x19:
		formatToOpcodeString("ADD HL, DE");
		break;

	case 0x1A:
		formatToOpcodeString("LD A, (DE)");
		break;

	case 0x1B:
		formatToOpcodeString("DEC DE");
		break;

	case 0x1C:
		formatToOpcodeString("INC E");
		break;

	case 0x1D:
		formatToOpcodeString("DEC E");
		break;

	case 0x1E:
		formatToOpcodeString("LD E, {:02X}", fetch_n8());
		break;

	case 0x1F:
		formatToOpcodeString("RRA");
		break;

	case 0x20:
		formatToOpcodeString("JR NZ, {:04X}", computeRelativeJump());
		break;

	case 0x21:
		formatToOpcodeString("LD HL, {:04X}", fetch_n16());
		break;

	case 0x22:
		formatToOpcodeString("LD (HL+), A");
		break;

	case 0x23:
		formatToOpcodeString("INC HL");
		break;

	case 0x24:
		formatToOpcodeString("INC H");
		break;

	case 0x25:
		formatToOpcodeString("DEC H");
		break;

	case 0x26:
		formatToOpcodeString("LD H, {:02X}", fetch_n8());
		break;

	case 0x27:
		formatToOpcodeString("DAA");
		break;

	case 0x28:
		formatToOpcodeString("JR Z, {:04X}", computeRelativeJump());
		break;

	case 0x29:
		formatToOpcodeString("ADD HL, HL");
		break;

	case 0x2A:
		formatToOpcodeString("LD A, (HL+)");
		break;

	case 0x2B:
		formatToOpcodeString("DEC HL");
		break;

	case 0x2C:
		formatToOpcodeString("INC L");
		break;

	case 0x2D:
		formatToOpcodeString("DEC L");
		break;

	case 0x2E:
		formatToOpcodeString("LD L, {:02X}", fetch_n8());
		break;

	case 0x2F:
		formatToOpcodeString("CPL");
		break;

	case 0x30:
		formatToOpcodeString("JR NC, {:04X}", computeRelativeJump());
		break;

	case 0x31:
		formatToOpcodeString("LD SP, {:04X}", fetch_n16());
		break;

	case 0x32:
		formatToOpcodeString("LD (HL-), A");
		break;

	case 0x33:
		formatToOpcodeString("INC SP");
		break;

	case 0x34:
		formatToOpcodeString("INC (HL)");
		break;

	case 0x35:
		formatToOpcodeString("DEC (HL)");
		break;

	case 0x36:
		formatToOpcodeString("LD (HL), {:02X}", fetch_n8());
		break;

	case 0x37:
		formatToOpcodeString("SCF");
		break;

	case 0x38:
		formatToOpcodeString("JR C, {:04X}", computeRelativeJump());
		break;

	case 0x39:
		formatToOpcodeString("ADD HL, SP");
		break;

	case 0x3A:
		formatToOpcodeString("LD A, (HL-)");
		break;

	case 0x3B:
		formatToOpcodeString("DEC SP");
		break;

	case 0x3C:
		formatToOpcodeString("INC A");
		break;

	case 0x3D:
		formatToOpcodeString("DEC A");
		break;

	case 0x3E:
		formatToOpcodeString("LD A, {:02X}", fetch_n8());
		break;

	case 0x3F:
		formatToOpcodeString("CCF");
		break;

	case 0x40:
		formatToOpcodeString("LD B, B");
		break;

	case 0x41:
		formatToOpcodeString("LD B, C");
		break;

	case 0x42:
		formatToOpcodeString("LD B, D");
		break;

	case 0x43:
		formatToOpcodeString("LD B, E");
		break;

	case 0x44:
		formatToOpcodeString("LD B, H");
		break;

	case 0x45:
		formatToOpcodeString("LD B, L");
		break;

	case 0x46:
		formatToOpcodeString("LD B, (HL)");
		break;

	case 0x47:
		formatToOpcodeString("LD B, A");
		break;

	case 0x48:
		formatToOpcodeString("LD C, B");
		break;

	case 0x49:
		formatToOpcodeString("LD C, C");
		break;

	case 0x4A:
		formatToOpcodeString("LD C, D");
		break;

	case 0x4B:
		formatToOpcodeString("LD C, E");
		break;

	case 0x4C:
		formatToOpcodeString("LD C, H");
		break;

	case 0x4D:
		formatToOpcodeString("LD C, L");
		break;

	case 0x4E:
		formatToOpcodeString("LD C, (HL)");
		break;

	case 0x4F:
		formatToOpcodeString("LD C, A");
		break;

	case 0x50:
		formatToOpcodeString("LD D, B");
		break;

	case 0x51:
		formatToOpcodeString("LD D, C");
		break;

	case 0x52:
		formatToOpcodeString("LD D, D");
		break;

	case 0x53:
		formatToOpcodeString("LD D, E");
		break;

	case 0x54:
		formatToOpcodeString("LD D, H");
		break;

	case 0x55:
		formatToOpcodeString("LD D, L");
		break;

	case 0x56:
		formatToOpcodeString("LD D, (HL)");
		break;

	case 0x57:
		formatToOpcodeString("LD D, A");
		break;

	case 0x58:
		formatToOpcodeString("LD E, B");
		break;

	case 0x59:
		formatToOpcodeString("LD E, C");
		break;

	case 0x5A:
		formatToOpcodeString("LD E, D");
		break;

	case 0x5B:
		formatToOpcodeString("LD E, E");
		break;

	case 0x5C:
		formatToOpcodeString("LD E, H");
		break;

	case 0x5D:
		formatToOpcodeString("LD E, L");
		break;

	case 0x5E:
		formatToOpcodeString("LD E, (HL)");
		break;

	case 0x5F:
		formatToOpcodeString("LD E, A");
		break;

	case 0x60:
		formatToOpcodeString("LD H, B");
		break;

	case 0x61:
		formatToOpcodeString("LD H, C");
		break;

	case 0x62:
		formatToOpcodeString("LD H, D");
		break;

	case 0x63:
		formatToOpcodeString("LD H, E");
		break;

	case 0x64:
		formatToOpcodeString("LD H, H");
		break;

	case 0x65:
		formatToOpcodeString("LD H, L");
		break;

	case 0x66:
		formatToOpcodeString("LD H, (HL)");
		break;

	case 0x67:
		formatToOpcodeString("LD H, A");
		break;

	case 0x68:
		formatToOpcodeString("LD L, B");
		break;

	case 0x69:
		formatToOpcodeString("LD L, C");
		break;

	case 0x6A:
		formatToOpcodeString("LD L, D");
		break;

	case 0x6B:
		formatToOpcodeString("LD L, E");
		break;

	case 0x6C:
		formatToOpcodeString("LD L, H");
		break;

	case 0x6D:
		formatToOpcodeString("LD L, L");
		break;

	case 0x6E:
		formatToOpcodeString("LD L, (HL)");
		break;

	case 0x6F:
		formatToOpcodeString("LD L, A");
		break;

	case 0x70:
		formatToOpcodeString("LD (HL), B");
		break;

	case 0x71:
		formatToOpcodeString("LD (HL), C");
		break;

	case 0x72:
		formatToOpcodeString("LD (HL), D");
		break;

	case 0x73:
		formatToOpcodeString("LD (HL), E");
		break;

	case 0x74:
		formatToOpcodeString("LD (HL), H");
		break;

	case 0x75:
		formatToOpcodeString("LD (HL), L");
		break;

	case 0x76:
		formatToOpcodeString("HALT");
		break;

	case 0x77:
		formatToOpcodeString("LD (HL), A");
		break;

	case 0x78:
		formatToOpcodeString("LD A, B");
		break;

	case 0x79:
		formatToOpcodeString("LD A, C");
		break;

	case 0x7A:
		formatToOpcodeString("LD A, D");
		break;

	case 0x7B:
		formatToOpcodeString("LD A, E");
		break;

	case 0x7C:
		formatToOpcodeString("LD A, H");
		break;

	case 0x7D:
		formatToOpcodeString("LD A,L");
		break;

	case 0x7E:
		formatToOpcodeString("LD A, (HL)");
		break;

	case 0x7F:
		formatToOpcodeString("LD A, A");
		break;

	case 0x80:
		formatToOpcodeString("ADD A, B");
		break;

	case 0x81:
		formatToOpcodeString("ADD A, C");
		break;

	case 0x82:
		formatToOpcodeString("ADD A, D");
		break;

	case 0x83:
		formatToOpcodeString("ADD A, E");
		break;

	case 0x84:
		formatToOpcodeString("ADD A, H");
		break;

	case 0x85:
		formatToOpcodeString("ADD A, L");
		break;

	case 0x86:
		formatToOpcodeString("ADD A, (HL)");
		break;

	case 0x87:
		formatToOpcodeString("ADD A, A");
		break;

	case 0x88:
		formatToOpcodeString("ADC A, B");
		break;

	case 0x89:
		formatToOpcodeString("ADC A, C");
		break;
	case 0x8A:

		formatToOpcodeString("ADC A, D");
		break;

	case 0x8B:
		formatToOpcodeString("ADC A, E");
		break;

	case 0x8C:
		formatToOpcodeString("ADC A, H");
		break;

	case 0x8D:
		formatToOpcodeString("ADC A, L");
		break;

	case 0x8E:
		formatToOpcodeString("ADC A, (HL)");
		break;

	case 0x8F:
		formatToOpcodeString("ADC A, A");
		break;

	case 0x90:
		formatToOpcodeString("SUB A, B");
		break;

	case 0x91:
		formatToOpcodeString("SUB A, C");
		break;

	case 0x92:
		formatToOpcodeString("SUB A, D");
		break;

	case 0x93:
		formatToOpcodeString("SUB A, E");
		break;

	case 0x94:
		formatToOpcodeString("SUB A, H");
		break;

	case 0x95:
		formatToOpcodeString("SUB A, L");
		break;

	case 0x96:
		formatToOpcodeString("SUB A, (HL)");
		break;

	case 0x97:
		formatToOpcodeString("SUB A, A");
		break;

	case 0x98:
		formatToOpcodeString("SBC A, B");
		break;

	case 0x99:
		formatToOpcodeString("SBC A, C");
		break;

	case 0x9A:
		formatToOpcodeString("SBC A, D");
		break;

	case 0x9B:
		formatToOpcodeString("SBC A, E");
		break;

	case 0x9C:
		formatToOpcodeString("SBC A, H");
		break;

	case 0x9D:
		formatToOpcodeString("SBC A, L");
		break;

	case 0x9E:
		formatToOpcodeString("SBC A, (HL)");
		break;

	case 0x9F:
		formatToOpcodeString("SBC A, A");
		break;

	case 0xA0:
		formatToOpcodeString("AND A, B");
		break;

	case 0xA1:
		formatToOpcodeString("AND A, C");
		break;

	case 0xA2:
		formatToOpcodeString("AND A, D");
		break;

	case 0xA3:
		formatToOpcodeString("AND A, E");
		break;

	case 0xA4:
		formatToOpcodeString("AND A, H");
		break;

	case 0xA5:
		formatToOpcodeString("AND A, L");
		break;

	case 0xA6:
		formatToOpcodeString("AND A, (HL)");
		break;

	case 0xA7:
		formatToOpcodeString("AND A, A");
		break;

	case 0xA8:
		formatToOpcodeString("XOR A, B");
		break;

	case 0xA9:
		formatToOpcodeString("XOR A, C");
		break;

	case 0xAA:
		formatToOpcodeString("XOR A, D");
		break;

	case 0xAB:
		formatToOpcodeString("XOR A, E");
		break;

	case 0xAC:
		formatToOpcodeString("XOR A, H");
		break;

	case 0xAD:
		formatToOpcodeString("XOR A, L");
		break;

	case 0xAE:
		formatToOpcodeString("XOR A, (HL)");
		break;

	case 0xAF:
		formatToOpcodeString("XOR A, A");
		break;

	case 0xB0:
		formatToOpcodeString("OR A, B");
		break;

	case 0xB1:
		formatToOpcodeString("OR A, C");
		break;

	case 0xB2:
		formatToOpcodeString("OR A, D");
		break;

	case 0xB3:
		formatToOpcodeString("OR A, E");
		break;

	case 0xB4:
		formatToOpcodeString("OR A, H");
		break;

	case 0xB5:
		formatToOpcodeString("OR A, L");
		break;

	case 0xB6:
		formatToOpcodeString("OR A, (HL)");
		break;

	case 0xB7:
		formatToOpcodeString("OR A, A");
		break;

	case 0xB8:
		formatToOpcodeString("CP A, B");
		break;

	case 0xB9:
		formatToOpcodeString("CP A, C");
		break;

	case 0xBA:
		formatToOpcodeString("CP A, D");
		break;

	case 0xBB:
		formatToOpcodeString("CP A, E");
		break;

	case 0xBC:
		formatToOpcodeString("CP A, H");
		break;

	case 0xBD:
		formatToOpcodeString("CP A, L");
		break;

	case 0xBE:
		formatToOpcodeString("CP A, (HL)");
		break;

	case 0xBF:
		formatToOpcodeString("CP A, A");
		break;

	case 0xC0:
		formatToOpcodeString("RET NZ");
		break;

	case 0xC1:
		formatToOpcodeString("POP BC");
		break;

	case 0xC2:
		formatToOpcodeString("JP NZ, {:04X}", fetch_n16());
		break;

	case 0xC3:
		formatToOpcodeString("JP {:04X}", fetch_n16());
		break;

	case 0xC4:
		formatToOpcodeString("CALL NZ, {:04X}", fetch_n16());
		break;

	case 0xC5:
		formatToOpcodeString("PUSH BC");
		break;

	case 0xC6:
		formatToOpcodeString("ADD A, {:02X}", fetch_n8());
		break;

	case 0xC7:
		formatToOpcodeString("RST 00h");
		break;

	case 0xC8:
		formatToOpcodeString("RET Z");
		break;

	case 0xC9:
		formatToOpcodeString("RET");
		break;

	case 0xCA:
		formatToOpcodeString("JP Z, {:04X}", fetch_n16());
		break;

	// Prefix modeString, decode opcode with second opcode table
	case 0xCB:
		disassemblePrefixedOpcode(fetch_n8());
		break;

	case 0xCC:
		formatToOpcodeString("CALL Z, {:04X}", fetch_n16());
		break;

	case 0xCD:
		formatToOpcodeString("CALL {:04X}", fetch_n16());
		break;

	case 0xCE:
		formatToOpcodeString("ADC A, {:02X}", fetch_n8());
		break;

	case 0xCF:
		formatToOpcodeString("RST 08h");
		break;

	case 0xD0:
		formatToOpcodeString("RET NC");
		break;

	case 0xD1:
		formatToOpcodeString("POP DE");
		break;

	case 0xD2:
		formatToOpcodeString("JP NC, {:04X}", fetch_n16());
		break;

	case 0xD3:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xD4:
		formatToOpcodeString("CALL NC, {:04X}", fetch_n16());
		break;

	case 0xD5:
		formatToOpcodeString("PUSH DE");
		break;

	case 0xD6:
		formatToOpcodeString("SUB A, {:02X}", fetch_n8());
		break;

	case 0xD7:
		formatToOpcodeString("RST 10h");
		break;

	case 0xD8:
		formatToOpcodeString("RET C");
		break;

	case 0xD9:
		formatToOpcodeString("RETI");
		break;

	case 0xDA:
		formatToOpcodeString("JP C, {:04X}", fetch_n16());
		break;

	case 0xDB:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xDC:
		formatToOpcodeString("CALL C, {:04X}", fetch_n16());
		break;

	case 0xDD:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xDE:
		formatToOpcodeString("SBC A, {:02X}", fetch_n8());
		break;

	case 0xDF:
		formatToOpcodeString("RST 18h");
		break;

	case 0xE0:
		formatToOpcodeString("LD (FF00 + {:02X}), A", fetch_n8());
		break;

	case 0xE1:
		formatToOpcodeString("POP HL");
		break;

	case 0xE2:
		formatToOpcodeString("LD $(FF00 + C), A");
		break;

	case 0xE3:
	case 0xE4:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xE5:
		formatToOpcodeString("PUSH HL");
		break;

	case 0xE6:
		formatToOpcodeString("AND A, {:02X}", fetch_n8());
		break;

	case 0xE7:
		formatToOpcodeString("RST 20h");
		break;

	case 0xE8: {
		uint8_t value = fetch_n8();
		if (value & 0x80)
		{
			formatToOpcodeString("ADD SP, -{:02X}", static_cast<uint8_t>(~value + 1));
		}
		else
		{
			formatToOpcodeString("ADD SP,  {:02X}", value);
		}
		break;
	}

	case 0xE9:
		formatToOpcodeString("JP HL");
		break;

	case 0xEA:
		formatToOpcodeString("LD ({:04X}), A", fetch_n16());
		break;

	case 0xEB:
	case 0xEC:
	case 0xED:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xEE:
		formatToOpcodeString("XOR A, {:02X}", fetch_n8());
		break;

	case 0xEF:
		formatToOpcodeString("RST 28h");
		break;

	case 0xF0:
		formatToOpcodeString("LD A, (FF00 + {:02X})", fetch_n8());
		break;

	case 0xF1:
		formatToOpcodeString("POP AF");
		break;

	case 0xF2:
		formatToOpcodeString("LD A, (FF00 + C)");
		break;

	case 0xF3:
		formatToOpcodeString("DI");
		break;

	case 0xF4:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xF5:
		formatToOpcodeString("PUSH AF");
		break;

	case 0xF6:
		formatToOpcodeString("OR A, {:02X}", fetch_n8());
		break;

	case 0xF7:
		formatToOpcodeString("RST 30h");
		break;

	case 0xF8: {
		uint8_t value = fetch_n8();
		if (value & 0x80)
		{
			formatToOpcodeString("LD HL, SP - {:02X}", static_cast<uint8_t>(~value + 1));
		}
		else
		{
			formatToOpcodeString("LD HL, SP + {:02X}", value);
		}
		break;
	}

	case 0xF9:
		formatToOpcodeString("LD SP, HL");
		break;

	case 0xFA:
		formatToOpcodeString("LD A, {:04X}", fetch_n16());
		break;

	case 0xFB:
		formatToOpcodeString("EI");
		break;

	case 0xFC:
	case 0xFD:
		formatToOpcodeString("Illegal opcode!");
		break;

	case 0xFE:
		formatToOpcodeString("CP A, {:02X}", fetch_n8());
		break;

	case 0xFF:
		formatToOpcodeString("RST 38h");
		break;

	default:
		break;
	}
}

void Disassembler::disassemblePrefixedOpcode(uint8_t opcode)
{
	switch (opcode)
	{
	case 0x00:
		formatToOpcodeString("RLC B");
		break;

	case 0x01:
		formatToOpcodeString("RLC C");
		break;

	case 0x02:
		formatToOpcodeString("RLC D");
		break;

	case 0x03:
		formatToOpcodeString("RLC E");
		break;

	case 0x04:
		formatToOpcodeString("RLC H");
		break;
	case 0x05:

		formatToOpcodeString("RLC L");
		break;

	case 0x06:
		formatToOpcodeString("RLC (HL)");
		break;

	case 0x07:
		formatToOpcodeString("RLC A");
		break;

	case 0x08:
		formatToOpcodeString("RRC B");
		break;

	case 0x09:
		formatToOpcodeString("RRC C");
		break;

	case 0x0A:
		formatToOpcodeString("RRC D");
		break;

	case 0x0B:
		formatToOpcodeString("RRC E");
		break;

	case 0x0C:
		formatToOpcodeString("RRC H");
		break;

	case 0x0D:
		formatToOpcodeString("RRC L");
		break;

	case 0x0E:
		formatToOpcodeString("RRC (HL)");
		break;

	case 0x0F:
		formatToOpcodeString("RRC A");
		break;

	case 0x10:
		formatToOpcodeString("RL B");
		break;

	case 0x11:
		formatToOpcodeString("RL C");
		break;

	case 0x12:
		formatToOpcodeString("RL D");
		break;

	case 0x13:
		formatToOpcodeString("RL E");
		break;
	case 0x14:

		formatToOpcodeString("RL H");
		break;
	case 0x15:

		formatToOpcodeString("RL L");
		break;

	case 0x16:
		formatToOpcodeString("RL (HL)");
		break;

	case 0x17:
		formatToOpcodeString("RL A");
		break;

	case 0x18:
		formatToOpcodeString("RR B");
		break;

	case 0x19:
		formatToOpcodeString("RR C");
		break;

	case 0x1A:
		formatToOpcodeString("RR D");
		break;

	case 0x1B:
		formatToOpcodeString("RR E");
		break;

	case 0x1C:
		formatToOpcodeString("RR H");
		break;

	case 0x1D:
		formatToOpcodeString("RR L");
		break;

	case 0x1E:
		formatToOpcodeString("RR (HL)");
		break;

	case 0x1F:
		formatToOpcodeString("RR A");
		break;

	case 0x20:
		formatToOpcodeString("SLA B");
		break;

	case 0x21:
		formatToOpcodeString("SLA C");
		break;

	case 0x22:
		formatToOpcodeString("SLA D");
		break;

	case 0x23:
		formatToOpcodeString("SLA E");
		break;

	case 0x24:
		formatToOpcodeString("SLA H");
		break;

	case 0x25:
		formatToOpcodeString("SLA L");
		break;

	case 0x26:
		formatToOpcodeString("SLA (HL)");
		break;

	case 0x27:
		formatToOpcodeString("SLA A");
		break;

	case 0x28:
		formatToOpcodeString("SRA B");
		break;

	case 0x29:
		formatToOpcodeString("SRA C");
		break;

	case 0x2A:
		formatToOpcodeString("SRA D");
		break;

	case 0x2B:
		formatToOpcodeString("SRA E");
		break;

	case 0x2C:
		formatToOpcodeString("SRA H");
		break;

	case 0x2D:
		formatToOpcodeString("SRA L");
		break;

	case 0x2E:
		formatToOpcodeString("SRA (HL)");
		break;

	case 0x2F:
		formatToOpcodeString("SRA A");
		break;

	case 0x30:
		formatToOpcodeString("SWAP B");
		break;

	case 0x31:
		formatToOpcodeString("SWAP C");
		break;

	case 0x32:
		formatToOpcodeString("SWAP D");
		break;

	case 0x33:
		formatToOpcodeString("SWAP E");
		break;

	case 0x34:
		formatToOpcodeString("SWAP H");
		break;

	case 0x35:
		formatToOpcodeString("SWAP L");
		break;

	case 0x36:
		formatToOpcodeString("SWAP (HL)");
		break;

	case 0x37:
		formatToOpcodeString("SWAP A");
		break;

	case 0x38:
		formatToOpcodeString("SRL B");
		break;

	case 0x39:
		formatToOpcodeString("SRL C");
		break;

	case 0x3A:
		formatToOpcodeString("SRL D");
		break;

	case 0x3B:
		formatToOpcodeString("SRL E");
		break;

	case 0x3C:
		formatToOpcodeString("SRL H");
		break;

	case 0x3D:
		formatToOpcodeString("SRL L");
		break;

	case 0x3E:
		formatToOpcodeString("SRL (HL)");
		break;

	case 0x3F:
		formatToOpcodeString("SRL A");
		break;

	case 0x40:
		formatToOpcodeString("BIT 0, B");
		break;

	case 0x41:
		formatToOpcodeString("BIT 0, C");
		break;

	case 0x42:
		formatToOpcodeString("BIT 0, D");
		break;

	case 0x43:
		formatToOpcodeString("BIT 0, E");
		break;

	case 0x44:
		formatToOpcodeString("BIT 0, H");
		break;

	case 0x45:
		formatToOpcodeString("BIT 0, L");
		break;

	case 0x46:
		formatToOpcodeString("BIT 0, (HL)");
		break;

	case 0x47:
		formatToOpcodeString("BIT 0, A");
		break;

	case 0x48:
		formatToOpcodeString("BIT 1, B");
		break;

	case 0x49:
		formatToOpcodeString("BIT 1, C");
		break;

	case 0x4A:
		formatToOpcodeString("BIT 1, D");
		break;

	case 0x4B:
		formatToOpcodeString("BIT 1, E");
		break;

	case 0x4C:
		formatToOpcodeString("BIT 1, H");
		break;

	case 0x4D:
		formatToOpcodeString("BIT 1, L");
		break;

	case 0x4E:
		formatToOpcodeString("BIT 1, (HL)");
		break;

	case 0x4F:
		formatToOpcodeString("BIT 1, A");
		break;

	case 0x50:
		formatToOpcodeString("BIT 2, B");
		break;

	case 0x51:
		formatToOpcodeString("BIT 2, C");
		break;

	case 0x52:
		formatToOpcodeString("BIT 2, D");
		break;

	case 0x53:
		formatToOpcodeString("BIT 2, E");
		break;

	case 0x54:
		formatToOpcodeString("BIT 2, H");
		break;

	case 0x55:
		formatToOpcodeString("BIT 2, L");
		break;

	case 0x56:
		formatToOpcodeString("BIT 2, (HL)");
		break;

	case 0x57:
		formatToOpcodeString("BIT 2, A");
		break;

	case 0x58:
		formatToOpcodeString("BIT 3, B");
		break;

	case 0x59:
		formatToOpcodeString("BIT 3, C");
		break;

	case 0x5A:
		formatToOpcodeString("BIT 3, D");
		break;

	case 0x5B:
		formatToOpcodeString("BIT 3, E");
		break;

	case 0x5C:
		formatToOpcodeString("BIT 3, H");
		break;

	case 0x5D:
		formatToOpcodeString("BIT 3, L");
		break;

	case 0x5E:
		formatToOpcodeString("BIT 3, (HL)");
		break;

	case 0x5F:
		formatToOpcodeString("BIT 3, A");
		break;

	case 0x60:
		formatToOpcodeString("BIT 4, B");
		break;

	case 0x61:
		formatToOpcodeString("BIT 4, C");
		break;

	case 0x62:
		formatToOpcodeString("BIT 4, D");
		break;

	case 0x63:
		formatToOpcodeString("BIT 4, E");
		break;

	case 0x64:
		formatToOpcodeString("BIT 4, H");
		break;

	case 0x65:
		formatToOpcodeString("BIT 4, L");
		break;

	case 0x66:
		formatToOpcodeString("BIT 4, (HL)");
		break;

	case 0x67:
		formatToOpcodeString("BIT 4, A");
		break;

	case 0x68:
		formatToOpcodeString("BIT 5, B");
		break;

	case 0x69:
		formatToOpcodeString("BIT 5, C");
		break;

	case 0x6A:
		formatToOpcodeString("BIT 5, D");
		break;

	case 0x6B:
		formatToOpcodeString("BIT 5, E");
		break;

	case 0x6C:
		formatToOpcodeString("BIT 5, H");
		break;

	case 0x6D:
		formatToOpcodeString("BIT 5, L");
		break;

	case 0x6E:
		formatToOpcodeString("BIT 5, (HL)");
		break;

	case 0x6F:
		formatToOpcodeString("BIT 5, A");
		break;

	case 0x70:
		formatToOpcodeString("BIT 6, B");
		break;

	case 0x71:
		formatToOpcodeString("BIT 6, C");
		break;

	case 0x72:
		formatToOpcodeString("BIT 6, D");
		break;

	case 0x73:
		formatToOpcodeString("BIT 6, E");
		break;

	case 0x74:
		formatToOpcodeString("BIT 6, H");
		break;

	case 0x75:
		formatToOpcodeString("BIT 6, L");
		break;

	case 0x76:
		formatToOpcodeString("BIT 6, (HL)");
		break;

	case 0x77:
		formatToOpcodeString("BIT 6, A");
		break;

	case 0x78:
		formatToOpcodeString("BIT 7, B");
		break;

	case 0x79:
		formatToOpcodeString("BIT 7, C");
		break;

	case 0x7A:
		formatToOpcodeString("BIT 7, D");
		break;

	case 0x7B:
		formatToOpcodeString("BIT 7, E");
		break;

	case 0x7C:
		formatToOpcodeString("BIT 7, H");
		break;

	case 0x7D:
		formatToOpcodeString("BIT 7, L");
		break;

	case 0x7E:
		formatToOpcodeString("BIT 7, (HL)");
		break;

	case 0x7F:
		formatToOpcodeString("BIT 7, A");
		break;

	case 0x80:
		formatToOpcodeString("RES 0, B");
		break;

	case 0x81:
		formatToOpcodeString("RES 0, C");
		break;

	case 0x82:
		formatToOpcodeString("RES 0, D");
		break;

	case 0x83:
		formatToOpcodeString("RES 0, E");
		break;

	case 0x84:
		formatToOpcodeString("RES 0, H");
		break;

	case 0x85:
		formatToOpcodeString("RES 0, L");
		break;

	case 0x86:
		formatToOpcodeString("RES 0, (HL)");
		break;

	case 0x87:
		formatToOpcodeString("RES 0, A");
		break;

	case 0x88:
		formatToOpcodeString("RES 1, B");
		break;

	case 0x89:
		formatToOpcodeString("RES 1, C");
		break;

	case 0x8A:
		formatToOpcodeString("RES 1, D");
		break;

	case 0x8B:
		formatToOpcodeString("RES 1, E");
		break;

	case 0x8C:
		formatToOpcodeString("RES 1, H");
		break;

	case 0x8D:
		formatToOpcodeString("RES 1, L");
		break;

	case 0x8E:
		formatToOpcodeString("RES 1, (HL)");
		break;

	case 0x8F:
		formatToOpcodeString("RES 1, A");
		break;

	case 0x90:
		formatToOpcodeString("RES 2, B");
		break;

	case 0x91:
		formatToOpcodeString("RES 2, C");
		break;

	case 0x92:
		formatToOpcodeString("RES 2, D");
		break;

	case 0x93:
		formatToOpcodeString("RES 2, E");
		break;

	case 0x94:
		formatToOpcodeString("RES 2, H");
		break;

	case 0x95:
		formatToOpcodeString("RES 2, L");
		break;

	case 0x96:
		formatToOpcodeString("RES 2, (HL)");
		break;

	case 0x97:
		formatToOpcodeString("RES 2, A");
		break;

	case 0x98:
		formatToOpcodeString("RES 3, B");
		break;

	case 0x99:
		formatToOpcodeString("RES 3, C");
		break;

	case 0x9A:
		formatToOpcodeString("RES 3, D");
		break;

	case 0x9B:
		formatToOpcodeString("RES 3, E");
		break;

	case 0x9C:
		formatToOpcodeString("RES 3, H");
		break;

	case 0x9D:
		formatToOpcodeString("RES 3, L");
		break;

	case 0x9E:
		formatToOpcodeString("RES 3, (HL)");
		break;

	case 0x9F:
		formatToOpcodeString("RES 3, A");
		break;

	case 0xA0:
		formatToOpcodeString("RES 4, B");
		break;

	case 0xA1:
		formatToOpcodeString("RES 4, C");
		break;

	case 0xA2:
		formatToOpcodeString("RES 4, D");
		break;

	case 0xA3:
		formatToOpcodeString("RES 4, E");
		break;

	case 0xA4:
		formatToOpcodeString("RES 4, H");
		break;

	case 0xA5:
		formatToOpcodeString("RES 4, L");
		break;

	case 0xA6:
		formatToOpcodeString("RES 4, (HL)");
		break;

	case 0xA7:
		formatToOpcodeString("RES 4, A");
		break;

	case 0xA8:
		formatToOpcodeString("RES 5, B");
		break;

	case 0xA9:
		formatToOpcodeString("RES 5, C");
		break;

	case 0xAA:
		formatToOpcodeString("RES 5, D");
		break;

	case 0xAB:
		formatToOpcodeString("RES 5, E");
		break;

	case 0xAC:
		formatToOpcodeString("RES 5, H");
		break;

	case 0xAD:
		formatToOpcodeString("RES 5, L");
		break;

	case 0xAE:
		formatToOpcodeString("RES 5, (HL)");
		break;

	case 0xAF:
		formatToOpcodeString("RES 5, A");
		break;

	case 0xB0:
		formatToOpcodeString("RES 6, B");
		break;

	case 0xB1:
		formatToOpcodeString("RES 6, C");
		break;

	case 0xB2:
		formatToOpcodeString("RES 6, D");
		break;

	case 0xB3:
		formatToOpcodeString("RES 6, E");
		break;

	case 0xB4:
		formatToOpcodeString("RES 6, H");
		break;

	case 0xB5:
		formatToOpcodeString("RES 6, L");
		break;

	case 0xB6:
		formatToOpcodeString("RES 6, (HL)");
		break;

	case 0xB7:
		formatToOpcodeString("RES 6, A");
		break;

	case 0xB8:
		formatToOpcodeString("RES 7, B");
		break;

	case 0xB9:
		formatToOpcodeString("RES 7, C");
		break;

	case 0xBA:
		formatToOpcodeString("RES 7, D");
		break;

	case 0xBB:
		formatToOpcodeString("RES 7, E");
		break;

	case 0xBC:
		formatToOpcodeString("RES 7, H");
		break;

	case 0xBD:
		formatToOpcodeString("RES 7, L");
		break;

	case 0xBE:
		formatToOpcodeString("RES 7, (HL)");
		break;

	case 0xBF:
		formatToOpcodeString("RES 7, A");
		break;

	case 0xC0:
		formatToOpcodeString("SET 0, B");
		break;

	case 0xC1:
		formatToOpcodeString("SET 0, C");
		break;

	case 0xC2:
		formatToOpcodeString("SET 0, D");
		break;

	case 0xC3:
		formatToOpcodeString("SET 0, E");
		break;

	case 0xC4:
		formatToOpcodeString("SET 0, H");
		break;

	case 0xC5:
		formatToOpcodeString("SET 0, L");
		break;

	case 0xC6:
		formatToOpcodeString("SET 0, (HL)");
		break;

	case 0xC7:
		formatToOpcodeString("SET 0, A");
		break;

	case 0xC8:
		formatToOpcodeString("SET 1, B");
		break;

	case 0xC9:
		formatToOpcodeString("SET 1, C");
		break;

	case 0xCA:
		formatToOpcodeString("SET 1, D");
		break;

	case 0xCB:
		formatToOpcodeString("SET 1, E");
		break;

	case 0xCC:
		formatToOpcodeString("SET 1, H");
		break;

	case 0xCD:
		formatToOpcodeString("SET 1, L");
		break;

	case 0xCE:
		formatToOpcodeString("SET 1, (HL)");
		break;

	case 0xCF:
		formatToOpcodeString("SET 1, A");
		break;

	case 0xD0:
		formatToOpcodeString("SET 2, B");
		break;

	case 0xD1:
		formatToOpcodeString("SET 2, C");
		break;

	case 0xD2:
		formatToOpcodeString("SET 2, D");
		break;

	case 0xD3:
		formatToOpcodeString("SET 2, E");
		break;

	case 0xD4:
		formatToOpcodeString("SET 2, H");
		break;

	case 0xD5:
		formatToOpcodeString("SET 2, L");
		break;

	case 0xD6:
		formatToOpcodeString("SET 2, (HL)");
		break;

	case 0xD7:
		formatToOpcodeString("SET 2, A");
		break;

	case 0xD8:
		formatToOpcodeString("SET 3, B");
		break;

	case 0xD9:
		formatToOpcodeString("SET 3, C");
		break;

	case 0xDA:
		formatToOpcodeString("SET 3, D");
		break;

	case 0xDB:
		formatToOpcodeString("SET 3, E");
		break;

	case 0xDC:
		formatToOpcodeString("SET 3, H");
		break;

	case 0xDD:
		formatToOpcodeString("SET 3, L");
		break;

	case 0xDE:
		formatToOpcodeString("SET 3, (HL)");
		break;

	case 0xDF:
		formatToOpcodeString("SET 3, A");
		break;

	case 0xE0:
		formatToOpcodeString("SET 4, B");
		break;

	case 0xE1:
		formatToOpcodeString("SET 4, C");
		break;

	case 0xE2:
		formatToOpcodeString("SET 4, D");
		break;

	case 0xE3:
		formatToOpcodeString("SET 4, E");
		break;

	case 0xE4:
		formatToOpcodeString("SET 4, H");
		break;

	case 0xE5:
		formatToOpcodeString("SET 4, L");
		break;

	case 0xE6:
		formatToOpcodeString("SET 4, (HL)");
		break;

	case 0xE7:
		formatToOpcodeString("SET 4, A");
		break;

	case 0xE8:
		formatToOpcodeString("SET 5, B");
		break;

	case 0xE9:
		formatToOpcodeString("SET 5, C");
		break;

	case 0xEA:
		formatToOpcodeString("SET 5, D");
		break;

	case 0xEB:
		formatToOpcodeString("SET 5, E");
		break;

	case 0xEC:
		formatToOpcodeString("SET 5, H");
		break;

	case 0xED:
		formatToOpcodeString("SET 5, L");
		break;

	case 0xEE:
		formatToOpcodeString("SET 5, (HL)");
		break;

	case 0xEF:
		formatToOpcodeString("SET 5, A");
		break;

	case 0xF0:
		formatToOpcodeString("SET 6, B");
		break;

	case 0xF1:
		formatToOpcodeString("SET 6, C");
		break;

	case 0xF2:
		formatToOpcodeString("SET 6, D");
		break;

	case 0xF3:
		formatToOpcodeString("SET 6, E");
		break;

	case 0xF4:
		formatToOpcodeString("SET 6, H");
		break;

	case 0xF5:
		formatToOpcodeString("SET 6, L");
		break;

	case 0xF6:
		formatToOpcodeString("SET 6, (HL)");
		break;

	case 0xF7:
		formatToOpcodeString("SET 6, A");
		break;

	case 0xF8:
		formatToOpcodeString("SET 7, B");
		break;

	case 0xF9:
		formatToOpcodeString("SET 7, C");
		break;

	case 0xFA:
		formatToOpcodeString("SET 7, D");
		break;

	case 0xFB:
		formatToOpcodeString("SET 7, E");
		break;

	case 0xFC:
		formatToOpcodeString("SET 7, H");
		break;

	case 0xFD:
		formatToOpcodeString("SET 7, L");
		break;

	case 0xFE:
		formatToOpcodeString("SET 7, (HL)");
		break;

	case 0xFF:
		formatToOpcodeString("SET 7, A");
		break;
	}
}

void Disassembler::logToConsole()
{
	DisassembledInstruction &current = m_buffer[m_bufferPosition];
	fmt::println("{:s} {:20s}{:s}", current.m_opcodeAddress, current.m_opcodeBytes, current.m_opcodeString);
}
