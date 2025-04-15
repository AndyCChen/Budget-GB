#pragma once

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
#include "imgui.h"

#include <cstdint>
#include <random>

class BudgetGB
{
  public:
	static constexpr uint32_t LCD_WIDTH            = 160;
	static constexpr uint32_t LCD_HEIGHT           = 144;
	static constexpr uint32_t INITIAL_WINDOW_SCALE = 4; // value should be 1-6 only!

	BudgetGB(const std::string &romPath = "");
	~BudgetGB();

	// Called once per frame at refresh rate
	void onUpdate(float deltaTime);

	SDL_AppResult processEvent(SDL_Event *event);

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
		GuiContextFlags_SHOW_IMGUI_DEMO          = 1 << 0,
		GuiContextFlags_SHOW_MAIN_MENU           = 1 << 1,
		GuiContextFlags_PAUSE                    = 1 << 2,
		GuiContextFlags_FULLSCREEN               = 1 << 3,
		GuiContextFlags_REENABLE_MULTI_VIEWPORTS = 1 << 4,
	};

	struct GuiContext
	{
		GuiContext()
		{
			flags              = GuiContextFlags_SHOW_IMGUI_DEMO;
			windowSizeSelector = 1 << BudgetGB::INITIAL_WINDOW_SCALE;
		}

		uint32_t flags = 0;
		uint32_t windowSizeSelector;
	};

	Cartridge    m_cartridge;
	Bus          m_bus;
	Disassembler m_disassembler;
	Sm83         m_cpu;

	GuiContext m_guiContext;

	float m_accumulatedDeltaTime = 0.0f;

	SDL_Window                      *m_window;
	RendererGB::RenderContext       *m_renderContext;
	std::vector<Utils::array_u8Vec3> m_lcdPixelBuffer;

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
	void resizeViewportFixed(WindowScale scale);

	/**
	 * @brief Handles any trigger events that need to be processed in the after start frame but before end frame.
	 *  The processEvent() is called by the sdl APP_EVENT callback which has no guarrentee which thread it is being
	 * called from. This function handles events that must be handled mid-frame.
	 */
	void handleEvents();

	void drawGui();
};
