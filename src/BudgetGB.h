#pragma once

#include <string>

#include "bus.h"
#include "cartridge.h"
#include "sm83.h"

class BudgetGB
{
  public:
	BudgetGB();
	BudgetGB(const std::string &romPath);

  private:
	Bus m_bus;
	Sm83 m_cpu;
	Cartridge m_cartridge;
};
