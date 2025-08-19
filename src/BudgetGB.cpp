#include <stdexcept>
#include <string>

#include "fmt/base.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "implot.h"
#include "mappers/mapper.h"
#include "misc/cpp/imgui_stdlib.h"

#include "BudgetGB.h"

constexpr static SDL_DialogFileFilter romFileFilter[] = {
	{"*.gb", "gb;gb"},
};

constexpr static SDL_DialogFileFilter bootromFileFilter[] = {
	{"*.bin", "bin;bin"},
};

static void SDLCALL loadRomDialogCallback(void *userdata, const char *const *filelist, int filter);
static void SDLCALL loadBootromDialogCallback(void *userdata, const char *const *filelist, int filter);

BudgetGB::BudgetGB(const std::string &cartridgePath)
	: m_renderContext(RendererGB::initWindowWithRenderer(m_window, static_cast<uint32_t>(m_config.windowScale))),
	  m_cartridge(),
	  m_bus(m_cartridge, m_cpu, m_ppu, m_apu),
	  m_cpu(m_bus),
	  m_ppu(m_cpu.m_interrupts.m_interruptFlags),
	  m_apu(BudgetGbConstants::AUDIO_SAMPLE_RATE),
	  m_disassembler(m_bus)
{
	if (!m_renderContext)
	{
		throw std::runtime_error("Failed to initialize window with renderer!");
	}

	m_guiContext.guiPalettes_activePalette = m_guiContext.guiPalettes_selectedPalette = m_config.activePalette;

	if (cartridgePath != "")
	{
		if (m_cartridge.loadCartridgeFromPath(cartridgePath, m_config.recentRoms))
		{
			m_apu.resumeAudio();
			m_disassembler.setProgramCounter(m_cpu.m_programCounter);
			m_disassembler.step();
		}
	}

	if (m_config.useBootrom)
	{
		if (!m_cpu.m_bootrom.loadFromFile(m_config.bootromPath))
			m_guiContext.flags |= GuiContextFlags_SHOW_BOOTROM_ERROR;
	}

	m_cpu.init(m_config.useBootrom && m_cpu.m_bootrom.isLoaded());
	m_bus.init(m_config.useBootrom && m_cpu.m_bootrom.isLoaded());
	m_apu.init(m_config.useBootrom && m_cpu.m_bootrom.isLoaded());

	m_lcdDisplayQuad = RendererGB::texturedQuadCreate(m_renderContext, Utils::Vec2<float>{BudgetGbConstants::LCD_WIDTH, BudgetGbConstants::LCD_HEIGHT});

	int width, height;
	SDL_GetWindowSize(m_window, &width, &height);
	m_mainViewportSize = {(uint32_t)width, (uint32_t)height};

	m_screenRenderTarget = RendererGB::textureRenderTargetCreate(m_renderContext, Utils::Vec2<float>{(float)width, (float)height});
	m_screenQuad         = RendererGB::screenQuadCreate(m_renderContext);
}

BudgetGB::~BudgetGB()
{
	m_config.activePalette = m_guiContext.guiPalettes_activePalette;
	RendererGB::freeWindowWithRenderer(m_window, m_renderContext);
}

void BudgetGB::onUpdate()
{
	if (!(m_guiContext.flags & GuiContextFlags_PAUSE) && m_cartridge.isLoaded())
	{
		m_accumulatedDeltaTime += ImGui::GetIO().DeltaTime;
		constexpr float TIME_STEP = 1.0f / 60.0f;

		while (m_accumulatedDeltaTime > TIME_STEP)
		{
			m_bus.onUpdate();
			m_accumulatedDeltaTime -= TIME_STEP;
		}
	}

	guiMain();

	RendererGB::setGlobalPalette(m_renderContext, m_guiContext.guiPalettes_activePalette < 0 ? m_config.defaultPalette : m_config.palettes[m_guiContext.guiPalettes_activePalette]);

	RendererGB::textureRenderTargetSet(m_renderContext, m_screenRenderTarget.get(), Utils::Vec2<float>{(float)m_mainViewportSize.x, (float)m_mainViewportSize.y});
	RendererGB::texturedQuadUpdateTexture(m_renderContext, m_lcdDisplayQuad.get(), m_ppu.getColorBuffer().data(), m_ppu.getColorBuffer().size());
	RendererGB::texturedQuadDraw(m_renderContext, m_lcdDisplayQuad.get());

	ImTextureID textureID = RendererGB::textureRenderTargetGetTextureID(m_screenRenderTarget.get());

	RendererGB::mainViewportSetRenderTarget(m_renderContext);
	RendererGB::screenQuadDraw(m_renderContext, m_screenQuad.get(), textureID);

	RendererGB::endFrame(m_window, m_renderContext);
	RendererGB::newFrame(); // begin new frame at end of this game loop
}

SDL_AppResult BudgetGB::processEvent(SDL_Event *event)
{
	ImGui_ImplSDL3_ProcessEvent(event);

	if (!m_guiContext.blockJoypadInputs)
		m_cpu.m_joypad.processEvent(event);
	else
		m_cpu.m_joypad.clear();

	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;

	if (event->window.windowID == SDL_GetWindowID(m_window))
	{
		if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			return SDL_APP_SUCCESS;

		else if (event->type == SDL_EVENT_WINDOW_RESIZED)
		{
			if (m_config.fullscreenMode == BudgetGbConfig::FullscreenMode::FIT)
				resizeViewportFit();
			else
				resizeViewportStretched();
		}
	}

	if (!ImGui::GetIO().WantCaptureMouse && event->type == SDL_EVENT_MOUSE_BUTTON_UP)
		if (event->button.button == 3) // right mouse button brings up main menu
			m_guiContext.flags |= GuiContextFlags_SHOW_MAIN_MENU;

	if (event->type == SDL_EVENT_KEY_UP)
	{
		switch (event->key.scancode)
		{

		case SDL_SCANCODE_F5:
			m_audioChannelToggle.Pulse1 ^= 1;
			m_apu.setAudioChannelToggle(m_audioChannelToggle);
			break;

		case SDL_SCANCODE_F6:
			m_audioChannelToggle.Pulse2 ^= 1;
			m_apu.setAudioChannelToggle(m_audioChannelToggle);
			break;

		case SDL_SCANCODE_F7:
			m_audioChannelToggle.Wave ^= 1;
			m_apu.setAudioChannelToggle(m_audioChannelToggle);
			break;

		case SDL_SCANCODE_F8:
			m_audioChannelToggle.Noise ^= 1;
			m_apu.setAudioChannelToggle(m_audioChannelToggle);
			break;

		case SDL_SCANCODE_F11:
			m_guiContext.flags ^= GuiContextFlags_FULLSCREEN;
			SDL_SetWindowFullscreen(m_window, m_guiContext.flags & GuiContextFlags_FULLSCREEN);
			SDL_SyncWindow(m_window);

			if (m_guiContext.flags & GuiContextFlags_FULLSCREEN)
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
			else
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

			break;

		case SDL_SCANCODE_P:
			if (!ImGui::GetIO().WantTextInput)
			{
				m_guiContext.flags ^= GuiContextFlags_PAUSE;
				if (m_guiContext.flags & GuiContextFlags_PAUSE && m_cartridge.isLoaded())
				{
					m_disassembler.setProgramCounter(m_cpu.m_programCounter);
					m_disassembler.step();
				}
			}

			break;

		case SDL_SCANCODE_KP_MULTIPLY:
			if (m_cartridge.isLoaded())
				resetBudgetGB();
			break;

		default:
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

bool BudgetGB::loadCartridge(const std::string &cartridgePath)
{
	if (m_cartridge.loadCartridgeFromPath(cartridgePath, m_config.recentRoms))
	{
		resetBudgetGB();
		return true;
	}
	else
	{
		m_apu.pauseAudio();
		return false;
	}
}

void BudgetGB::resizeViewportStretched()
{
	int resizedWidth, resizedHeight;
	SDL_GetWindowSize(m_window, &resizedWidth, &resizedHeight);

	// aspect ratio of gameboy display is 10:9

	float widthRatio  = (float)resizedWidth / 10.0f;
	float heightRatio = (float)resizedHeight / 9.0f;

	Utils::Vec2<uint32_t> viewportSize;

	if (widthRatio < heightRatio)
	{
		viewportSize.x = resizedWidth;
		viewportSize.y = static_cast<uint32_t>(widthRatio * 9);
	}
	else
	{
		viewportSize.y = resizedHeight;
		viewportSize.x = static_cast<uint32_t>(heightRatio * 10);
	}

	int viewportX = (resizedWidth - viewportSize.x) / 2;
	int viewportY = (resizedHeight - viewportSize.y) / 2;

	RendererGB::mainViewportResize(m_renderContext, viewportX, viewportY, viewportSize.x, viewportSize.y);
	RendererGB::textureRenderTargetResize(m_renderContext, m_screenRenderTarget.get(), Utils::Vec2<float>{(float)viewportSize.x, (float)viewportSize.y});
	m_mainViewportSize = viewportSize;
}

void BudgetGB::resizeViewportFit()
{
	int resizedWidth, resizedHeight;
	SDL_GetWindowSize(m_window, &resizedWidth, &resizedHeight);

	Utils::Vec2<uint32_t> viewportSize;

	if (resizedHeight < resizedWidth)
	{
		viewportSize.y = BudgetGbConstants::LCD_HEIGHT * (resizedHeight / BudgetGbConstants::LCD_HEIGHT);
		viewportSize.x = static_cast<uint32_t>(10 * (viewportSize.y / 9.0f));
	}
	else
	{
		viewportSize.x = BudgetGbConstants::LCD_WIDTH * (resizedWidth / BudgetGbConstants::LCD_WIDTH);
		viewportSize.y = static_cast<uint32_t>(9 * (viewportSize.x / 10.0f));
	}

	int viewportX = (resizedWidth - viewportSize.x) / 2;
	int viewportY = (resizedHeight - viewportSize.y) / 2;

	RendererGB::mainViewportResize(m_renderContext, viewportX, viewportY, viewportSize.x, viewportSize.y);
	RendererGB::textureRenderTargetResize(m_renderContext, m_screenRenderTarget.get(), Utils::Vec2<float>{(float)viewportSize.x, (float)viewportSize.y});
	m_mainViewportSize = viewportSize;
}

void BudgetGB::resizeWindowFixed(BudgetGbConfig::WindowScale scale)
{
	if (m_guiContext.flags & GuiContextFlags_FULLSCREEN)
	{
		m_guiContext.flags &= ~GuiContextFlags_FULLSCREEN;
		SDL_SetWindowFullscreen(m_window, false);
		SDL_SyncWindow(m_window);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // re-enable viewports when exiting fullscreen
	}

	m_config.windowScale = scale;
	int scaleFactor      = static_cast<uint32_t>(m_config.windowScale);

	SDL_SetWindowSize(m_window, scaleFactor * BudgetGbConstants::LCD_WIDTH, scaleFactor * BudgetGbConstants::LCD_HEIGHT);

	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void BudgetGB::guiMain()
{
	m_guiContext.blockJoypadInputs = false;

	if (m_guiContext.flags & GuiContextFlags_SHOW_MAIN_MENU)
	{
		ImGui::OpenPopup("Main Menu?");
		m_guiContext.flags ^= GuiContextFlags_SHOW_MAIN_MENU;
	}

	bool showBootromModal = false;
	bool showCartInfo     = false;

	if (ImGui::BeginPopup("Main Menu?", ImGuiWindowFlags_NoMove))
	{
		if (ImGui::MenuItem("Pause", "P", m_guiContext.flags & GuiContextFlags_PAUSE))
		{
			m_guiContext.flags ^= GuiContextFlags_PAUSE;
			if (m_guiContext.flags & GuiContextFlags_PAUSE && m_cartridge.isLoaded())
			{
				m_disassembler.setProgramCounter(m_cpu.m_programCounter);
				m_disassembler.step();
			}
		}

		if (ImGui::MenuItem("Load ROM..."))
			SDL_ShowOpenFileDialog(loadRomDialogCallback, this, m_window, romFileFilter, 1, nullptr, false);

		if (ImGui::BeginMenu("Window Sizes"))
		{
			using namespace BudgetGbConfig;

			if (ImGui::MenuItem("1x1", "", m_config.windowScale == WindowScale::WindowScale_1x1))
				resizeWindowFixed(WindowScale::WindowScale_1x1);
			if (ImGui::MenuItem("1x2", "", m_config.windowScale == WindowScale::WindowScale_1x2))
				resizeWindowFixed(WindowScale::WindowScale_1x2);
			if (ImGui::MenuItem("1x3", "", m_config.windowScale == WindowScale::WindowScale_1x3))
				resizeWindowFixed(WindowScale::WindowScale_1x3);
			if (ImGui::MenuItem("1x4", "", m_config.windowScale == WindowScale::WindowScale_1x4))
				resizeWindowFixed(WindowScale::WindowScale_1x4);
			if (ImGui::MenuItem("1x5", "", m_config.windowScale == WindowScale::WindowScale_1x5))
				resizeWindowFixed(WindowScale::WindowScale_1x5);
			if (ImGui::MenuItem("1x6", "", m_config.windowScale == WindowScale::WindowScale_1x6))
				resizeWindowFixed(WindowScale::WindowScale_1x6);

			if (ImGui::MenuItem("Fullscreen", "F11", m_guiContext.flags & GuiContextFlags_FULLSCREEN))
			{
				m_guiContext.flags ^= GuiContextFlags_FULLSCREEN;
				SDL_SetWindowFullscreen(m_window, m_guiContext.flags & GuiContextFlags_FULLSCREEN);
				SDL_SyncWindow(m_window);

				if (m_guiContext.flags & GuiContextFlags_FULLSCREEN)
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
				else
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			}

			ImGui::BeginDisabled(!(m_guiContext.flags & GuiContextFlags_FULLSCREEN));

			if (ImGui::MenuItem("Fullscreen Fit", "", m_config.fullscreenMode == BudgetGbConfig::FullscreenMode::FIT))
			{
				m_config.fullscreenMode = BudgetGbConfig::FullscreenMode::FIT;
				resizeViewportFit();
			}

			if (ImGui::MenuItem("Fullscreen Stretched", "", m_config.fullscreenMode == BudgetGbConfig::FullscreenMode::STRETCHED))
			{
				m_config.fullscreenMode = BudgetGbConfig::FullscreenMode::STRETCHED;
				resizeViewportStretched();
			}

			ImGui::EndDisabled();

			ImGui::EndMenu();
		}

#ifdef SHOW_IMGUI_DEMOS

		if (ImGui::MenuItem("Toggle Imgui Demo", "", m_guiContext.flags & GuiContextFlags_SHOW_IMGUI_DEMO))
			m_guiContext.flags ^= GuiContextFlags_SHOW_IMGUI_DEMO;

		if (ImGui::MenuItem("Toggle Implot Demo", "", m_guiContext.flags & GuiContextFlags_SHOW_IMPLOT_DEMO))
			m_guiContext.flags ^= GuiContextFlags_SHOW_IMPLOT_DEMO;

#endif

		if (ImGui::MenuItem("Palettes", "", m_guiContext.flags & GuiContextFlags_SHOW_PALETTES))
			m_guiContext.flags ^= GuiContextFlags_SHOW_PALETTES;

		if (ImGui::MenuItem("Tile Viewer", "", m_guiContext.flags & GuiContextFlags_SHOW_TILES))
		{
			m_guiContext.flags ^= GuiContextFlags_SHOW_TILES;

			if (m_guiContext.flags & GuiContextFlags_SHOW_TILES)
				m_patternTileViewport = std::make_unique<PatternTileView>(m_ppu, m_renderContext);
			else
				m_patternTileViewport.reset();
		}

		if (ImGui::MenuItem("Audio", "", m_guiContext.flags & GuiContextFlags_SHOW_AUDIO))
		{
			m_guiContext.flags ^= GuiContextFlags_SHOW_AUDIO;
		}

		if (ImGui::MenuItem("CPU Viewer", "", m_guiContext.flags & GuiContextFlags_SHOW_CPU_VIEWER))
			m_guiContext.flags ^= GuiContextFlags_SHOW_CPU_VIEWER;

		ImGui::BeginDisabled(!m_cartridge.isLoaded());
		if (ImGui::MenuItem("Reset", "*"))
			resetBudgetGB();
		ImGui::EndDisabled();

		if (ImGui::Selectable("Bootrom"))
			showBootromModal = true;

		ImGui::BeginDisabled(!m_cartridge.isLoaded());
		if (ImGui::Selectable("CartInfo"))
			showCartInfo = true;
		ImGui::EndDisabled();

		if (ImGui::BeginMenu("Shaders"))
		{
			if (ImGui::MenuItem("None", "", m_guiContext.shaderSelect == RendererGB::ShaderSelect::None))
			{
				m_guiContext.shaderSelect = RendererGB::ShaderSelect::None;
				RendererGB::screenQuadSwapShader(m_renderContext, m_screenQuad.get(), m_guiContext.shaderSelect);
			}

			if (ImGui::MenuItem("zfast-lcd", "", m_guiContext.shaderSelect == RendererGB::ShaderSelect::Zfast))
			{
				m_guiContext.shaderSelect = RendererGB::ShaderSelect::Zfast;
				RendererGB::screenQuadSwapShader(m_renderContext, m_screenQuad.get(), m_guiContext.shaderSelect);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Recent roms", !m_config.recentRoms.empty()))
		{
			std::array<std::string, BudgetGbConfig::MAX_RECENT_ROMS> buffer;
			std::copy(m_config.recentRoms.begin(), m_config.recentRoms.end(), buffer.begin());

			std::size_t length = m_config.recentRoms.size();
			for (uint8_t i = 0; i < length; ++i)
			{
				ImGui::PushID(i);
				if (ImGui::MenuItem(buffer[i].c_str()))
					loadCartridge(buffer[i]);
				ImGui::PopID();
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();
		if (ImGui::Selectable("Quit"))
		{
			SDL_Event event{};
			event.type = SDL_EVENT_QUIT;
			SDL_PushEvent(&event);
		}
		ImGui::EndPopup();
	}

	// imgui & implot demo windows

#ifdef SHOW_IMGUI_DEMOS

	if (m_guiContext.flags & GuiContextFlags_SHOW_IMGUI_DEMO)
	{
		bool toggle = true;
		ImGui::ShowDemoWindow(&toggle);
		if (!toggle)
			m_guiContext.flags ^= GuiContextFlags_SHOW_IMGUI_DEMO;
	}

	if (m_guiContext.flags & GuiContextFlags_SHOW_IMPLOT_DEMO)
	{
		bool toggle = true;
		ImPlot::ShowDemoWindow(&toggle);
		if (!toggle)
			m_guiContext.flags ^= GuiContextFlags_SHOW_IMPLOT_DEMO;
	}

#endif

	if (m_guiContext.flags & GuiContextFlags_SHOW_CPU_VIEWER)
		guiCpuViewer();

	if (m_guiContext.flags & GuiContextFlags_SHOW_PALETTES)
		guiPalettes();

	if (m_patternTileViewport)
	{
		if (!m_patternTileViewport->drawViewportGui(m_renderContext))
		{
			m_patternTileViewport.reset();
			m_guiContext.flags &= ~GuiContextFlags_SHOW_TILES;
		}
	}

	if (m_guiContext.flags & GuiContextFlags_SHOW_AUDIO)
	{
		if (!AudioWidget::drawAudioWidget(m_apu, m_audioChannelToggle))
		{
			m_guiContext.flags &= ~GuiContextFlags_SHOW_AUDIO;
		}
	}

	if (showCartInfo)
		ImGui::OpenPopup("CartInfo");

	if (ImGui::BeginPopupModal("CartInfo", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const Mapper::CartInfo &cartInfo = m_cartridge.getCartInfo();

		ImGui::Text("Mapper: %s", Mapper::getMapperString(cartInfo.MbcType).data());
		ImGui::Text("Rom size: %u", cartInfo.RomSize);
		ImGui::Text("Ram size: %u", cartInfo.RamSize);
		ImGui::Text("Battery backed: %s", cartInfo.BatteryBacked ? "Yes" : "No");

		if (ImGui::Button("Ok"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	if (showBootromModal)
		ImGui::OpenPopup("DMG bootrom");

	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.9411f, 0.9411f, 0.9411f, 1.0f));
	if (ImGui::BeginPopupModal("DMG bootrom", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		m_guiContext.blockJoypadInputs |= ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		ImGui::InputText("##Bootrom path", &m_config.bootromPath);
		ImGui::SameLine();

		if (ImGui::Button("..."))
			SDL_ShowOpenFileDialog(loadBootromDialogCallback, &m_config, m_window, bootromFileFilter, 1, nullptr, false);

		ImGui::Checkbox("Enable bootrom", &m_config.useBootrom);

		if (ImGui::Button("Close", ImVec2(80, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(1);

	if (m_guiContext.flags & GuiContextFlags_SHOW_BOOTROM_ERROR)
	{
		ImGui::OpenPopup("Bootrom load error");
		m_guiContext.flags ^= GuiContextFlags_SHOW_BOOTROM_ERROR;
	}

	if (ImGui::BeginPopupModal("Bootrom load error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		m_guiContext.blockJoypadInputs |= ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		ImGui::Text("%s", m_cpu.m_bootrom.getErrorMsg().c_str());

		if (ImGui::Button("Ok", ImVec2(80, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void BudgetGB::guiCpuViewer()
{
	bool toggle = m_guiContext.flags & GuiContextFlags_SHOW_CPU_VIEWER;

	if (ImGui::Begin("CPU Viewer", &toggle))
	{
		m_guiContext.blockJoypadInputs |= ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		if (ImGui::BeginTable("CPU Viewer Table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();

			ImVec2 tableSize = {0.0f, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - ImGui::GetTextLineHeightWithSpacing()};
			ImGui::TableSetColumnIndex(0);
			if (ImGui::BeginTable("CPU Instruction Log", 1, ImGuiTableFlags_ScrollY, tableSize))
			{
				std::size_t position   = m_cpu.m_opcodeLogger.bufferPosition();
				std::size_t bufferSize = m_cpu.m_opcodeLogger.bufferSize();

				if (m_guiContext.guiCpuViewer_snapInstructionScrollY)
				{
					ImGui::SetScrollFromPosY(ImGui::GetTextLineHeightWithSpacing() * bufferSize);
					m_guiContext.guiCpuViewer_snapInstructionScrollY = false;
				}

				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("CPU Instructions");
				ImGui::TableHeadersRow();

				ImGuiListClipper clipper;
				clipper.Begin((int)bufferSize);
				while (clipper.Step())
				{
					for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
					{
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("%s", m_cpu.m_opcodeLogger.getLogAt((position + row + 1) % bufferSize));
					}
				}

				ImGui::EndTable();
			}

			ImGui::TableSetColumnIndex(1);

			ImGui::Text("CPU Registers");
			if (ImGui::BeginTable("CPU Registers", 2, ImGuiTabBarFlags_NoTooltip))
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("PC: %04X", m_cpu.m_programCounter);
				ImGui::Text("SP: %04X", m_cpu.m_stackPointer);
				ImGui::Text("AF: %04X", m_cpu.m_registerAF.get_u16());
				ImGui::Text("BC: %04X", m_cpu.m_registerBC.get_u16());
				ImGui::Text("DE: %04X", m_cpu.m_registerDE.get_u16());
				ImGui::Text("HL: %04X", m_cpu.m_registerHL.get_u16());

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Z: %u", m_cpu.m_registerAF.flags.Z);
				ImGui::Text("N: %u", m_cpu.m_registerAF.flags.N);
				ImGui::Text("H: %u", m_cpu.m_registerAF.flags.H);
				ImGui::Text("C: %u", m_cpu.m_registerAF.flags.C);

				ImGui::EndTable();
			}

			ImGui::NewLine();

			constexpr ImVec4 buttonColor = ImVec4(0.260f, 0.590f, 0.980f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);

			// unpause
			if (m_guiContext.flags & GuiContextFlags_PAUSE)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
				if (ImGui::Button("Pause"))
					m_guiContext.flags &= ~GuiContextFlags_PAUSE;
				ImGui::PopStyleColor(1);
			}
			// pause
			else if (ImGui::Button("Pause"))
			{
				if (m_cartridge.isLoaded())
				{
					m_disassembler.setProgramCounter(m_cpu.m_programCounter);
					m_disassembler.step();
				}
				m_guiContext.flags |= GuiContextFlags_PAUSE;
			}

			ImGui::BeginDisabled(!m_cartridge.isLoaded() || !(m_guiContext.flags & GuiContextFlags_PAUSE));
			if (ImGui::Button("Instruction Step"))
			{
				m_cpu.instructionStep();
				m_disassembler.setProgramCounter(m_cpu.m_programCounter);
				m_disassembler.step();
			}
			ImGui::EndDisabled();

			ImGui::BeginDisabled(!m_cartridge.isLoaded());
			if (ImGui::Button("Reset"))
			{
				resetBudgetGB();
			}
			ImGui::EndDisabled();

			ImGui::NewLine();

			ImGui::Text("Lines to Log");
			auto &selectedIndex = m_cpu.m_opcodeLogger.m_selectedOptionIdx;
			ImGui::BeginDisabled(m_guiContext.flags & GuiContextFlags_TOGGLE_INSTRUCTION_LOG);
			if (ImGui::BeginCombo("", OpcodeLogger::LOGGER_OPTIONS[selectedIndex].label))
			{
				for (uint8_t n = 0; n < OpcodeLogger::LOGGER_OPTIONS.size(); ++n)
				{
					const bool selected = n == selectedIndex;
					if (ImGui::Selectable(OpcodeLogger::LOGGER_OPTIONS[n].label, selected))
						selectedIndex = n;

					if (selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::EndDisabled();

			if (m_guiContext.flags & GuiContextFlags_TOGGLE_INSTRUCTION_LOG)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
				if (ImGui::Button("Stop Logging"))
				{
					m_guiContext.flags &= ~GuiContextFlags_TOGGLE_INSTRUCTION_LOG;
					m_cpu.m_logEnable = m_guiContext.flags & GuiContextFlags_TOGGLE_INSTRUCTION_LOG;
					m_cpu.m_opcodeLogger.stopLog();
				}
				ImGui::PopStyleColor(1);
			}
			else if (ImGui::Button("Start Logging"))
			{
				m_guiContext.guiCpuViewer_snapInstructionScrollY = true;
				m_guiContext.flags |= GuiContextFlags_TOGGLE_INSTRUCTION_LOG;
				m_cpu.m_opcodeLogger.startLog();
				m_cpu.m_logEnable = m_guiContext.flags & GuiContextFlags_TOGGLE_INSTRUCTION_LOG;
			}

			ImGui::PopStyleColor(2);
			ImGui::NewLine();

			if (ImGui::BeginTable("Next Instructions", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX))
			{
				ImGui::TableSetupColumn("Next Instructions");
				ImGui::TableHeadersRow();

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				constexpr ImVec4 RED = {0.9686f, 0.1843f, 0.1843f, 1.0f};

				ImGui::TextColored(RED, "%s", m_disassembler.getDisassemblyAt(0));
				ImGui::Text("%s", m_disassembler.getDisassemblyAt(1));
				ImGui::Text("%s", m_disassembler.getDisassemblyAt(2));
				ImGui::Text("%s", m_disassembler.getDisassemblyAt(3));
				ImGui::Text("%s", m_disassembler.getDisassemblyAt(4));
				ImGui::Text("%s", m_disassembler.getDisassemblyAt(5));

				ImGui::EndTable();
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();

	m_guiContext.flags = toggle ? (m_guiContext.flags | GuiContextFlags_SHOW_CPU_VIEWER) : (m_guiContext.flags & ~GuiContextFlags_SHOW_CPU_VIEWER);
}

void BudgetGB::guiPalettes()
{
	bool toggle = m_guiContext.flags & GuiContextFlags_SHOW_PALETTES;

	if (ImGui::Begin("Palettes", &toggle))
	{
		m_guiContext.blockJoypadInputs |= ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		ImGui::BeginDisabled(m_config.palettes.size() >= BudgetGbConfig::MAX_PALETTES);
		if (ImGui::Button("Add Palette"))
		{
			using namespace BudgetGbConfig;
			m_config.palettes.insert(m_config.palettes.begin(), {"New Palette", DEFAULT_GB_PALETTE[0], DEFAULT_GB_PALETTE[1], DEFAULT_GB_PALETTE[2], DEFAULT_GB_PALETTE[3]});

			if (m_guiContext.guiPalettes_activePalette != -1)
				m_guiContext.guiPalettes_activePalette += 1;

			if (m_guiContext.guiPalettes_selectedPalette != -1)
				m_guiContext.guiPalettes_selectedPalette += 1;
		}
		ImGui::EndDisabled();

		ImGui::SameLine();

		std::array<char, 16> paletteCountText{};
		fmt::format_to_n(paletteCountText.data(), paletteCountText.size(), "{}/{}", m_config.palettes.size(), BudgetGbConfig::MAX_PALETTES);
		ImGui::Text(paletteCountText.data());

		ImGui::BeginChild("Palette List", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		{
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs;

			// default palette

			ImGui::ColorEdit3("Default C0", m_config.defaultPalette.color0.data(), flags);
			ImGui::SameLine();
			ImGui::ColorEdit3("Default C1", m_config.defaultPalette.color1.data(), flags);
			ImGui::SameLine();
			ImGui::ColorEdit3("Default C2", m_config.defaultPalette.color2.data(), flags);
			ImGui::SameLine();
			ImGui::ColorEdit3("Default C3", m_config.defaultPalette.color3.data(), flags);
			ImGui::SameLine();

			if (ImGui::Selectable(m_config.defaultPalette.name.c_str(), m_guiContext.guiPalettes_activePalette < 0, ImGuiSelectableFlags_AllowDoubleClick))
			{
				m_guiContext.guiPalettes_selectedPalette = -1;
				if (ImGui::IsMouseDoubleClicked(0))
				{
					m_guiContext.guiPalettes_activePalette = -1;
				}
			}

			// rest of user palettes

			for (int i = 0; static_cast<unsigned long long>(i) < m_config.palettes.size(); ++i)
			{
				ImGui::PushID(i);

				ImGui::ColorEdit3("C0", m_config.palettes[i].color0.data(), flags);
				ImGui::SameLine();
				ImGui::ColorEdit3("C1", m_config.palettes[i].color1.data(), flags);
				ImGui::SameLine();
				ImGui::ColorEdit3("C2", m_config.palettes[i].color2.data(), flags);
				ImGui::SameLine();
				ImGui::ColorEdit3("C3", m_config.palettes[i].color3.data(), flags);
				ImGui::SameLine();

				if (ImGui::Selectable(m_config.palettes[i].name.c_str(), m_guiContext.guiPalettes_activePalette == i, ImGuiSelectableFlags_AllowDoubleClick))
				{
					m_guiContext.guiPalettes_selectedPalette = i;
					if (ImGui::IsMouseDoubleClicked(0))
					{
						m_guiContext.guiPalettes_activePalette = i;
					}
				}

				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("Palette Dropzone", &i, sizeof(int));
					ImGui::Text("%s", m_config.palettes[i].name.c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Palette Dropzone"))
					{
						IM_ASSERT(payload->DataSize == sizeof(int));
						int payload_i = *(const int *)payload->Data;
						std::swap(m_config.palettes[i], m_config.palettes[payload_i]);

						if (payload_i == m_guiContext.guiPalettes_activePalette)
							m_guiContext.guiPalettes_activePalette = i;
						else if (i == m_guiContext.guiPalettes_activePalette)
							m_guiContext.guiPalettes_activePalette = payload_i;

						if (payload_i == m_guiContext.guiPalettes_selectedPalette)
							m_guiContext.guiPalettes_selectedPalette = i;
					}

					ImGui::EndDragDropTarget();
				}

				bool toggleRenamePopup = false;

				if (ImGui::BeginPopupContextItem())
				{
					if ((toggleRenamePopup = ImGui::Selectable("Rename")))
					{
						m_guiContext.guiPalettes_renameBuffer = m_config.palettes[i].name;
					}

					if (ImGui::Selectable("Delete"))
					{
						m_config.palettes.erase(m_config.palettes.begin() + i);

						if (m_config.palettes.size() == 0)
						{
							m_guiContext.guiPalettes_selectedPalette = -1;
							m_guiContext.guiPalettes_activePalette   = -1;
						}
						else
						{
							if (static_cast<unsigned long long>(m_guiContext.guiPalettes_selectedPalette) == m_config.palettes.size() || m_guiContext.guiPalettes_selectedPalette > i)
							{
								m_guiContext.guiPalettes_selectedPalette -= 1;
							}

							if (static_cast<unsigned long long>(m_guiContext.guiPalettes_activePalette) == m_config.palettes.size() || m_guiContext.guiPalettes_activePalette > i)
							{
								m_guiContext.guiPalettes_activePalette -= 1;
							}
						}
					}

					ImGui::EndPopup();
				}

				if (toggleRenamePopup)
					ImGui::OpenPopup("Rename Palette");

				ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.9411f, 0.9411f, 0.9411f, 1.0f));
				if (ImGui::BeginPopupModal("Rename Palette", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{

					ImGui::InputText("##New Palette Name", &m_guiContext.guiPalettes_renameBuffer);

					if (ImGui::Button("Ok"))
					{
						m_config.palettes[i].name = m_guiContext.guiPalettes_renameBuffer;
						m_guiContext.guiPalettes_renameBuffer.clear();

						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel"))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
				ImGui::PopStyleColor(1);

				ImGui::PopID();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("Color Picker", ImVec2(0, 0));
		{
			ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayHex;

			if (m_guiContext.guiPalettes_selectedPalette < 0)
			{
				ImGui::Text(m_config.defaultPalette.name.c_str());
				ImGui::ColorEdit3("Default C0", m_config.defaultPalette.color0.data(), flags);
				ImGui::ColorEdit3("Default C1", m_config.defaultPalette.color1.data(), flags);
				ImGui::ColorEdit3("Default C2", m_config.defaultPalette.color2.data(), flags);
				ImGui::ColorEdit3("Default C3", m_config.defaultPalette.color3.data(), flags);

				if (ImGui::Button("Reset Defaults"))
				{
					m_config.defaultPalette.color0 = BudgetGbConfig::DEFAULT_GB_PALETTE[0];
					m_config.defaultPalette.color1 = BudgetGbConfig::DEFAULT_GB_PALETTE[1];
					m_config.defaultPalette.color2 = BudgetGbConfig::DEFAULT_GB_PALETTE[2];
					m_config.defaultPalette.color3 = BudgetGbConfig::DEFAULT_GB_PALETTE[3];
				}
			}
			else
			{
				ImGui::Text(m_config.palettes[m_guiContext.guiPalettes_selectedPalette].name.c_str());
				ImGui::ColorEdit3("C0", m_config.palettes[m_guiContext.guiPalettes_selectedPalette].color0.data(), flags);
				ImGui::ColorEdit3("C1", m_config.palettes[m_guiContext.guiPalettes_selectedPalette].color1.data(), flags);
				ImGui::ColorEdit3("C2", m_config.palettes[m_guiContext.guiPalettes_selectedPalette].color2.data(), flags);
				ImGui::ColorEdit3("C3", m_config.palettes[m_guiContext.guiPalettes_selectedPalette].color3.data(), flags);
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	m_guiContext.flags = toggle ? (m_guiContext.flags | GuiContextFlags_SHOW_PALETTES) : (m_guiContext.flags & ~GuiContextFlags_SHOW_PALETTES);
}

static void SDLCALL loadRomDialogCallback(void *userdata, const char *const *filelist, int filter)
{
	(void)filter;
	BudgetGB *gameboy = (BudgetGB *)userdata;

	if (!filelist)
	{
		SDL_LogError(0, "Failed to select file! %s", SDL_GetError());
		return;
	}
	// canceled file dialog, no file selected
	else if (!*filelist)
		return;

	gameboy->loadCartridge(std::string(*filelist));
}

static void SDLCALL loadBootromDialogCallback(void *userdata, const char *const *filelist, int filter)
{
	(void)filter;
	BudgetGbConfig::Config *config = (BudgetGbConfig::Config *)userdata;

	if (!filelist)
	{
		SDL_LogError(0, "Failed to select file! %s", SDL_GetError());
		return;
	}
	else if (!*filelist)
		return;

	config->bootromPath = *filelist;
}
