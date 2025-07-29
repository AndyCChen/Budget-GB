#pragma once

#include "mapper.h"

#include <array>
#include <cstdint>
#include <fstream>

namespace Mapper
{

// default 32kb cartridge with no memory bank switching
class NoMBC : public IMapper
{
  public:
	NoMBC(std::ifstream &romFile, const Mapper::CartInfo &cartInfo);
	~NoMBC() override = default;

	virtual uint8_t read(uint16_t position) override;
	virtual void    write(uint16_t position, uint8_t data) override;
	virtual void    reset() override
	{
	}

  private:
	std::array<uint8_t, 1024 * 32> m_rom{};
};
} // namespace Mapper
