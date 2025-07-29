#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "SDL3/SDL.h"
#include "bus.h"
#include "cartridge.h"
#include "config.h"
#include "disassembler.h"
#include "emulatorConstants.h"
#include "imgui.h"
#include "patternTileView.h"
#include "renderer.h"
#include "sm83.h"
#include "utils/vec.h"

class BudgetGB
{
  public:
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
	enum GuiContextFlags
	{
		GuiContextFlags_SHOW_IMGUI_DEMO = 1 << 0,
		GuiContextFlags_SHOW_MAIN_MENU  = 1 << 1,
		GuiContextFlags_PAUSE           = 1 << 2,
		GuiContextFlags_FULLSCREEN      = 1 << 3,
		GuiContextFlags_SHOW_TILES      = 1 << 5,
		GuiContextFlags_SHOW_CPU_VIEWER = 1 << 6,
		GuiContextFlags_SHOW_PALETTES   = 1 << 7,

		GuiContextFlags_SHOW_BOOTROM_ERROR = 1 << 8,

		GuiContextFlags_TOGGLE_INSTRUCTION_LOG = 1 << 9,
	};

	struct GuiContext
	{
		uint32_t flags             = 0;
		bool     blockJoypadInputs = false;

		int         guiPalettes_selectedPalette = 0;
		int         guiPalettes_activePalette   = 0;
		std::string guiPalettes_renameBuffer;

		bool guiCpuViewer_snapInstructionScrollY = false;
	};

	Cartridge    m_cartridge;
	Bus          m_bus;
	Sm83         m_cpu;
	PPU          m_ppu;
	Disassembler m_disassembler;

	GuiContext             m_guiContext;
	BudgetGbConfig::Config m_config;

	float m_accumulatedDeltaTime = 0.0f;

	SDL_Window                       *m_window;
	RendererGB::RenderContext        *m_renderContext;
	BudgetGbConstants::LcdColorBuffer m_lcdColorBuffer{};

	RendererGB::TexturedQuadUniquePtr m_lcdDisplayQuad;

	std::unique_ptr<PatternTileView> m_patternTileViewport;

	/*std::random_device              m_rd;
	std::mt19937                    m_gen;
	std::uniform_int_distribution<> m_palleteRange;*/

	void resetBudgetGB()
	{
		if (m_config.useBootrom)
		{
			if (!m_cpu.m_bootrom.loadFromFile(m_config.bootromPath))
				m_guiContext.flags |= GuiContextFlags_SHOW_BOOTROM_ERROR;
		}

		m_cpu.init(m_config.useBootrom && m_cpu.m_bootrom.isLoaded());
		m_bus.init(m_config.useBootrom && m_cpu.m_bootrom.isLoaded());
		m_disassembler.setProgramCounter(m_cpu.m_programCounter);
		m_disassembler.step();
		m_cartridge.resetMapper();
	}

	/**
	 * @brief Resize viewport to fit any arbitary window size while still respecting the 10:9 aspect
	 * ratio of gameboy display. Will stretch the visible viewport to the window dimensions to the max.
	 */
	void resizeViewportStretched();

	/**
	 * @brief Resize the viewport to dimensions that are multiples of the lcd display dimensions (144 by 160 pixels)
	 */
	void resizeViewportFit();

	/**
	 * @brief Resize window with prefined fixed scales.
	 */
	void resizeWindowFixed(BudgetGbConfig::WindowScale scale);

	// gui draw functions

	/**
	 * @brief Top level entry point for gui.
	 */
	void guiMain();
	void guiCpuViewer();
	void guiPalettes();
};
