#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "mappers/mapper.h"

class Cartridge
{
  public:
	Cartridge()
	{
		m_cartridgeLoaded = false;
	}

	bool loadCartridgeFromPath(const std::string &path, std::vector<std::string> &recentRoms);

	bool isLoaded() const
	{
		return m_cartridgeLoaded;
	}

	uint8_t cartridgeRead(uint16_t position)
	{
		return m_mapper->read(position);
	}

	void cartridgeWrite(uint16_t position, uint8_t data)
	{
		m_mapper->write(position, data);
	}

	// Retrieves cartridge info that is valid assuming the cartridge is loaded.
	const Mapper::CartInfo &getCartInfo() const
	{
		return m_mapper->m_cartInfo;
	}

	void resetMapper()
	{
		m_mapper->reset();
	}

	const char *getCartridgeErrorMsg()
	{
		return m_errorMsg.c_str();
	}

  private:
	std::unique_ptr<Mapper::IMapper> m_mapper;

	std::string m_errorMsg;
	bool        m_cartridgeLoaded;
};
