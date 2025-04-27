#include "opcodeLogger.h"

#include <iterator>
#include <optional>

#include "fmt/base.h"

OpcodeLogger::OpcodeLogger(std::size_t size)
{
	m_buffer.resize(size);
}

// start a new opcode entry in logger
void OpcodeLogger::begin(uint16_t address)
{
	m_bufferPosition = (m_bufferPosition + 1) % m_buffer.size();
	m_buffer[m_bufferPosition].clear();
	m_buffer[m_bufferPosition].m_opcodeAddress = address;
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
	m_buffer[m_bufferPosition].m_opcodeFormat = format;
}

void OpcodeLogger::setOpcodeFormat(const std::string &format, const uint16_t arg)
{
	m_buffer[m_bufferPosition].m_opcodeFormat = format;
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
	item.m_buffer.clear();

	fmt::format_to(std::back_inserter(item.m_buffer), "{:04X}   ", item.m_opcodeAddress);

	if (item.m_opcodeLength == 1)
		fmt::format_to(std::back_inserter(item.m_buffer), "{:02X}         ", item.m_opcodeBytes[0]);
	else if (item.m_opcodeLength == 2)
		fmt::format_to(std::back_inserter(item.m_buffer), "{:02X} {:02X}      ", item.m_opcodeBytes[0], item.m_opcodeBytes[1]);
	else if (item.m_opcodeLength == 3)
		fmt::format_to(std::back_inserter(item.m_buffer), "{:02X} {:02X} {:02X}   ", item.m_opcodeBytes[0], item.m_opcodeBytes[1], item.m_opcodeBytes[2]);

	if (!item.m_arg.has_value())
		fmt::format_to(std::back_inserter(item.m_buffer), item.m_opcodeFormat);
	else
		fmt::format_to(std::back_inserter(item.m_buffer), item.m_opcodeFormat, item.m_arg.value());

	return item.m_buffer.c_str();
}