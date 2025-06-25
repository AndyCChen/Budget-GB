#pragma once

#include <string>
#include <cstdint>
#include <array>

class DmgBootRom
{
  public:
	static constexpr uint16_t DMG_BOOTROM_SIZE = 256;

  private:
	std::array<uint8_t, DMG_BOOTROM_SIZE> m_bootrom{};
	std::string                           m_errorMsg = "";

	bool loaded = false;

  public:
	bool loadFromFile(const std::string &path);

	uint8_t read(uint16_t position)
	{
		return m_bootrom[position & 0xFF];
	}

	std::string getErrorMsg()
	{
		return m_errorMsg;
	}

	bool isLoaded() const
	{
		return loaded;
	}
};