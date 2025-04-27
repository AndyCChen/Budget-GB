#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "SDL3/SDL.h"
#include "bus.h"
#include "cartridge.h"
#include "imgui.h"
#include "renderer.h"
#include "sm83.h"
#include "utils/vec.h"

#include <cstdint>
#include <random>

class BudgetGB
{
  public:
	static constexpr uint32_t LCD_WIDTH            = 160;
	static constexpr uint32_t LCD_HEIGHT           = 144;
	static constexpr uint32_t INITIAL_WINDOW_SCALE = 4;       // value should be 1-6 only!
	static constexpr uint32_t CLOCK_RATE_T         = 4194304; // sm83 clock rate for the gameboy in T-cycles

	/**
	 * @brief Initialize gameboy instance with a optional path to a cartridge.
	 * @param cartridgePath
	 */
	BudgetGB(const std::string &cartridgePath = "");
	~BudgetGB();

	// Called once per frame at refresh rate
	void onUpdate(float deltaTime);

	SDL_AppResult processEvent(SDL_Event *event);

	/**
	 * @brief Resets bus and cpu before loading a cartridge from the provided path. Used for loading new cartridges after the gameboy instance
	 * has been created.
	 * @param cartridgePath
	 * @return True on success, false otherwise.
	 */
	bool loadCartridge(const std::string &cartridgePath);

  private:
	enum class WindowScale
	{
		WindowScale_1x1 = 1,
		WindowScale_1x2,
		WindowScale_1x3,
		WindowScale_1x4,
		WindowScale_1x5,
		WindowScale_1x6,
	};

	enum GuiContextFlags
	{
		GuiContextFlags_SHOW_IMGUI_DEMO        = 1 << 0,
		GuiContextFlags_SHOW_MAIN_MENU         = 1 << 1,
		GuiContextFlags_PAUSE                  = 1 << 2,
		GuiContextFlags_FULLSCREEN             = 1 << 3,
		GuiContextFlags_SHOW_CPU_VIEWER        = 1 << 4,
		GuiContextFlags_INSTRUCTION_STEP       = 1 << 5,
		GuiContextFlags_TOGGLE_INSTRUCTION_LOG = 1 << 6,
	};

	struct GuiContext
	{
		GuiContext()
		{
			flags              = GuiContextFlags_SHOW_IMGUI_DEMO | GuiContextFlags_PAUSE;
			windowSizeSelector = 1 << BudgetGB::INITIAL_WINDOW_SCALE;
		}

		uint32_t flags = 0;
		uint32_t windowSizeSelector;
	};

	Cartridge m_cartridge;
	Bus       m_bus;
	Sm83      m_cpu;

	GuiContext m_guiContext;

	float m_accumulatedDeltaTime = 0.0f;

	SDL_Window                      *m_window;
	RendererGB::RenderContext       *m_renderContext;
	std::vector<Utils::array_u8Vec4> m_lcdPixelBuffer;

	std::random_device              m_rd;
	std::mt19937                    m_gen;
	std::uniform_int_distribution<> m_palleteRange;

	/**
	 * @brief Resize viewport to fit any arbitary window size while still respecting the 10:9 aspect
	 * ratio of gameboy display.
	 */
	void resizeViewport();

	/**
	 * @brief Resize window with prefined fixed scales.
	 */
	void resizeWindowFixed(WindowScale scale);

	// gui draw functions

	/**
	 * @brief Top level entry point for gui.
	 */
	void guiMain();
	void guiCpuViewer(bool *toggle);
};
