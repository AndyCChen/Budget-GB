#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "SDL3/SDL.h"
#include "bus.h"
#include "cartridge.h"
#include "disassembler.h"
#include "renderer.h"
#include "sm83.h"

namespace BudgetGBConstants {
inline constexpr std::size_t LCD_WIDTH = 160;
inline constexpr std::size_t LCD_HEIGHT = 144;
}

class BudgetGB
{
public:
	BudgetGB();
	BudgetGB(const std::string &romPath);

	~BudgetGB();

	void run();

private:
	bool m_isRunning = true;
	bool m_openMenu = false;
	Cartridge m_cartridge;
	Bus m_bus;
	Disassembler m_disassembler;
	Sm83 m_cpu;

	SDL_Window *m_window;
	RendererGB::RenderContext *m_renderContext;
	std::vector<uint8_t> m_lcdPixels;

	void gbProcessEvent();
};
