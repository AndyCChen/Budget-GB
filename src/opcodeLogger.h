#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class OpcodeLogger
{
  public:
	struct LoggerOption
	{
		const char *const label;
		const uint32_t    lines;
	};

	uint8_t m_selectedOptionIdx = 0;

	static constexpr std::array<LoggerOption, 3> LOGGER_OPTIONS = {{{"100", 100}, {"1000", 1000}, {"10000", 10000}}};

	struct Sm83Instruction
	{
		uint8_t                 m_opcodeLength  = 0;            // opcodes are 1-3 bytes long
		std::optional<uint16_t> m_opcodeAddress = std::nullopt; // starting address of instruction
		std::array<uint8_t, 3>  m_opcodeBytes   = {0, 0, 0};    // store an array of bytes representing it's opcode and any operands if any
		std::string             m_opcodeFormat  = "";
		std::optional<uint16_t> m_arg           = std::nullopt; // stores either a 1-2 immediate bytes of a opcode or a computed jump address for JR instructions

		uint16_t m_stackPointer = 0;
		uint16_t m_registerAF   = 0;
		uint16_t m_registerBC   = 0;
		uint16_t m_registerDE   = 0;
		uint16_t m_registerHL   = 0;

		std::string m_buffer = ""; // intermediate buffer to hold full string formated instruction log

		void clear()
		{
			m_opcodeAddress = std::nullopt;
			m_arg           = std::nullopt;
			m_opcodeLength  = 0;
			m_opcodeFormat.clear();
		}
	};

	// start next opcode entry in logger
	void next(uint16_t address, uint16_t sp, uint16_t af, uint16_t bc, uint16_t de, uint16_t hl);

	void setOpcodeAddress(const uint16_t address);

	void appendOpcodeByte(const uint8_t opByte);

	void setOpcodeFormat(const std::string &format);

	void setOpcodeFormat(const std::string &format, const uint16_t arg);

	std::size_t bufferSize() const;

	std::size_t bufferPosition() const;

	const char *getLogAt(std::size_t index);

	void startLog();
	void stopLog();

  private:
	std::size_t                  m_bufferPosition = 0;
	std::vector<Sm83Instruction> m_buffer;
};