#include "opcodeLogger.h"

#include <iterator>
#include <optional>

#include "fmt/base.h"

// start a new opcode entry in logger
void OpcodeLogger::next(uint16_t address, uint16_t sp, uint16_t af, uint16_t bc, uint16_t de, uint16_t hl)
{
	m_bufferPosition = (m_bufferPosition + 1) % m_buffer.size();

	Sm83Instruction &intr = m_buffer[m_bufferPosition];
	intr.clear();
	intr.m_opcodeAddress = address;
	intr.m_stackPointer  = sp;
	intr.m_registerAF    = af;
	intr.m_registerBC    = bc;
	intr.m_registerDE    = de;
	intr.m_registerHL    = hl;
}

void OpcodeLogger::setOpcodeAddress(const uint16_t address)
{
	m_buffer[m_bufferPosition].m_opcodeAddress = address;
}

void OpcodeLogger::appendOpcodeByte(const uint8_t opByte)
{
	auto &intr                                = m_buffer[m_bufferPosition];
	intr.m_opcodeBytes[intr.m_opcodeLength++] = opByte;
}

void OpcodeLogger::setOpcodeFormat(const std::string &format)
{
	format.copy(m_buffer[m_bufferPosition].m_opcodeFormat.data(), m_buffer[m_bufferPosition].m_opcodeFormat.size() - 1);
	//m_buffer[m_bufferPosition].m_opcodeFormat = format;
}

void OpcodeLogger::setOpcodeFormat(const std::string &format, const uint16_t arg)
{
	format.copy(m_buffer[m_bufferPosition].m_opcodeFormat.data(), m_buffer[m_bufferPosition].m_opcodeFormat.size() - 1);
	m_buffer[m_bufferPosition].m_arg          = arg;
}

std::size_t OpcodeLogger::bufferSize() const
{
	return m_buffer.size();
}

std::size_t OpcodeLogger::bufferPosition() const
{
	return m_bufferPosition;
}

const char *OpcodeLogger::getLogAt(std::size_t index)
{
	Sm83Instruction &item = m_buffer[index];
	if (item.m_opcodeAddress.has_value())
	{
		item.m_buffer.fill(0);

		char addressBuffer[5]{};
		char opcodeBytesBuffer[9]{};
		char opcodeBuffer[32]{};
		char registerBuffer[64]{};

		fmt::format_to_n(addressBuffer, sizeof(addressBuffer), "{:04X}", item.m_opcodeAddress.value());

		if (item.m_opcodeLength == 1)
			fmt::format_to_n(opcodeBytesBuffer, sizeof(opcodeBytesBuffer), "{:02X}", item.m_opcodeBytes[0]);
		else if (item.m_opcodeLength == 2)
			fmt::format_to_n(opcodeBytesBuffer, sizeof(opcodeBytesBuffer), "{:02X} {:02X}", item.m_opcodeBytes[0], item.m_opcodeBytes[1]);
		else if (item.m_opcodeLength == 3)
			fmt::format_to_n(opcodeBytesBuffer, sizeof(opcodeBytesBuffer), "{:02X} {:02X} {:02X}", item.m_opcodeBytes[0], item.m_opcodeBytes[1], item.m_opcodeBytes[2]);

		if (!item.m_arg.has_value())
			fmt::format_to_n(opcodeBuffer, sizeof(opcodeBuffer), item.m_opcodeFormat.data());
		else
			fmt::format_to_n(opcodeBuffer, sizeof(opcodeBuffer), item.m_opcodeFormat.data(), item.m_arg.value());

		fmt::format_to_n(registerBuffer, sizeof(registerBuffer), "SP:{:04X} AF:{:04X} BC:{:04X} DE:{:04X} HL:{:04X}", item.m_stackPointer, item.m_registerAF, item.m_registerBC, item.m_registerDE, item.m_registerHL);

		fmt::format_to_n(item.m_buffer.data(), sizeof(item.m_buffer), "{:s}   {:>8s}   {:20s} {:s}  {:s}{:s}{:s}{:s}", addressBuffer, opcodeBytesBuffer, opcodeBuffer, registerBuffer, (item.m_registerAF & 0x80) ? "Z" : "z", (item.m_registerAF & 0x40) ? "N" : "n", (item.m_registerAF & 0x20) ? "H" : "h", (item.m_registerAF & 0x10) ? "C" : "c");

		return item.m_buffer.data();
	}
	else
	{
		return "";
	}
}

void OpcodeLogger::startLog()
{
	m_buffer.resize(LOGGER_OPTIONS[m_selectedOptionIdx].lines);
	m_bufferPosition = 0;
}

void OpcodeLogger::stopLog()
{
	m_buffer.resize(0);
	m_bufferPosition = 0;
}
