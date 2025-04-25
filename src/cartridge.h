#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Cartridge
{
  public:
	Cartridge()
	{
		m_cartridgeLoaded = false;
	}

	bool    loadCartridgeFromPath(const std::string &path);
	uint8_t cartridgeRead(uint16_t position);
	bool    isLoaded() const
	{
		return m_cartridgeLoaded;
	}

  private:
	std::vector<uint8_t> m_cartridgeRom;
	std::vector<uint8_t> m_externalRam;
	bool                 m_cartridgeLoaded;
};
