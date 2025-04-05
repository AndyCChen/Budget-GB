#pragma once

#include <string>

#include "SDL3/SDL.h"
#include "bus.h"
#include "cartridge.h"
#include "disassembler.h"
#include "sm83.h"

class BudgetGB
{
public:
	BudgetGB();
	BudgetGB(const std::string &romPath);

	~BudgetGB();

	void run();

private:
	bool m_isRunning = true;
	Cartridge m_cartridge;
	Bus m_bus;
	Disassembler m_disassembler;
	Sm83 m_cpu;

	SDL_Window *m_window;

	void gbProcessEvent();
};
