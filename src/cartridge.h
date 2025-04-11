#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Cartridge
{
  public:
	Cartridge()
	{
		// minimum 32kb of cartridge
		m_cartridgeRom.resize(1024 * 32);
		m_externalRam.resize(0);
	}

	bool    loadRomFromPath(const std::string &path);
	uint8_t cartridgeRead(uint16_t position);

  private:
	std::vector<uint8_t> m_cartridgeRom;
	std::vector<uint8_t> m_externalRam;
};
