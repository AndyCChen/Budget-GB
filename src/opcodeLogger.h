#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

class OpcodeLogger
{
  public:
	struct Sm83Instruction
	{
		uint8_t                 m_opcodeLength  = 0;         // opcodes are 1-3 bytes long
		uint16_t                m_opcodeAddress = 0;         // starting address of instruction
		std::array<uint8_t, 3>  m_opcodeBytes   = {0, 0, 0}; // store an array of bytes representing it's opcode and any operands if any
		std::string             m_opcodeFormat  = "";
		std::optional<uint16_t> m_arg           = std::nullopt; // stores either a 1-2 immediate bytes of a opcode or a computed jump address for JR instructions

		std::string m_buffer; // intermediate buffer to hold full string formated instruction log

		Sm83Instruction()
		{
			m_buffer.reserve(100);
		}

		void clear()
		{
			m_arg          = std::nullopt;
			m_opcodeLength = 0;
			m_opcodeFormat.clear();
		}
	};

	OpcodeLogger(std::size_t size);

	// start a new opcode entry in logger
	void begin(uint16_t address);

	void setOpcodeAddress(const uint16_t address);

	void appendOpcodeByte(const uint8_t opByte);

	void setOpcodeFormat(const std::string &format);

	void setOpcodeFormat(const std::string &format, const uint16_t arg);

	std::size_t bufferSize() const;

	std::size_t bufferPosition() const;

	const char *getLogAt(std::size_t index);

  private:
	std::size_t                  m_bufferPosition = 0;
	std::vector<Sm83Instruction> m_buffer;
};