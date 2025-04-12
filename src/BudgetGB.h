#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "SDL3/SDL.h"
#include "bus.h"
#include "cartridge.h"
#include "disassembler.h"
#include "renderer.h"
#include "sm83.h"
#include "utils/vec.h"

class BudgetGB
{
  public:
	static constexpr uint32_t LCD_WIDTH            = 160;
	static constexpr uint32_t LCD_HEIGHT           = 144;
	static constexpr uint32_t INITIAL_WINDOW_SCALE = 4;

	BudgetGB(const std::string &romPath = "");
	~BudgetGB();

	void run();

  private:
	struct MenuOptions
	{
		bool openMenu    = false;
		bool toggleClamp = true;
	};

	bool m_isRunning = true;

	Cartridge    m_cartridge;
	Bus          m_bus;
	Disassembler m_disassembler;
	Sm83         m_cpu;
	MenuOptions  m_options;

	SDL_Window                      *m_window;
	RendererGB::RenderContext       *m_renderContext;
	std::vector<Utils::array_u8Vec3> m_lcdPixelBuffer;

	void gbProcessEvent();

	/**
	 * @brief Attempt to clamp viewport to perfectly fit the 10:9 apect ratio of the gameboy.
	 */
	void clampViewport();
};
