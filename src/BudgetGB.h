#pragma once

#include <string>

#include "bus.h"
#include "cartridge.h"
#include "sm83.h"
#include "disassembler.h"

class BudgetGB
{
  public:
	BudgetGB();
	BudgetGB(const std::string &romPath);

	void run();

  private:
	Cartridge m_cartridge;
	Bus m_bus;
	Disassembler m_disassembler;
	Sm83 m_cpu;
};
