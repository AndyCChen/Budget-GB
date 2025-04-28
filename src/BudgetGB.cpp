#include "BudgetGB.h"
#include "fmt/base.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include <stdexcept>

static unsigned char colorPallete[][3] = {
	{8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208}, {255, 255, 255}
};

BudgetGB::BudgetGB(const std::string &cartridgePath)
	: m_cartridge(), m_bus(m_cartridge, m_cpu), m_cpu(m_bus), m_gen(m_rd()),
	  m_palleteRange(0, 4)
{
	m_lcdPixelBuffer.resize(LCD_WIDTH * LCD_HEIGHT);

	if (!RendererGB::initWindowWithRenderer(m_window, m_renderContext, INITIAL_WINDOW_SCALE))
	{
		throw std::runtime_error("Failed to initialize window with renderer!");
	}

	if (cartridgePath != "")
	{
		m_cartridge.loadCartridgeFromPath(cartridgePath);
	}

	for (std::size_t i = 0; i < m_lcdPixelBuffer.size(); ++i)
	{
		unsigned char colorIdx = (unsigned char)m_palleteRange(m_gen);
		m_lcdPixelBuffer[i][0] = colorPallete[colorIdx][0];
		m_lcdPixelBuffer[i][1] = colorPallete[colorIdx][1];
		m_lcdPixelBuffer[i][2] = colorPallete[colorIdx][2];
		m_lcdPixelBuffer[i][3] = 255;
	}
}

BudgetGB::~BudgetGB()
{
	RendererGB::freeWindowWithRenderer(m_window, m_renderContext);
}

void BudgetGB::onUpdate(float deltaTime)
{
	if (!(m_guiContext.flags & GuiContextFlags_PAUSE))
	{
		m_accumulatedDeltaTime += deltaTime;
		constexpr float time = 1.0f / 60.0f;
		if (m_accumulatedDeltaTime > time)
		{
			if (m_cartridge.isLoaded())
			{
				constexpr int ticksPerFrame = BudgetGB::CLOCK_RATE_T / 60;
				while (m_cpu.m_tCycleTicks < ticksPerFrame)
				{
					m_cpu.runInstruction();
				}

				m_cpu.m_tCycleTicks -= ticksPerFrame;
			}
			m_accumulatedDeltaTime -= time;
		}
	}
	else if (m_guiContext.flags & GuiContextFlags_INSTRUCTION_STEP)
	{
		m_guiContext.flags &= ~GuiContextFlags_INSTRUCTION_STEP;
		m_cpu.runInstruction();
	}

	guiMain();
	RendererGB::drawMainViewport(m_lcdPixelBuffer, m_renderContext, m_window);

	RendererGB::endFrame(m_window, m_renderContext);
	RendererGB::newFrame(); // begin new frame at end of this game loop
}

SDL_AppResult BudgetGB::processEvent(SDL_Event *event)
{
	ImGui_ImplSDL3_ProcessEvent(event);

	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;

	if (event->window.windowID == SDL_GetWindowID(m_window))
	{
		if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
			return SDL_APP_SUCCESS;

		else if (event->type == SDL_EVENT_WINDOW_RESIZED)
			resizeViewport();
	}

	if (!ImGui::GetIO().WantCaptureMouse && event->type == SDL_EVENT_MOUSE_BUTTON_UP)
		if (event->button.button == 3) // right mouse button brings up main menu
			m_guiContext.flags |= GuiContextFlags_SHOW_MAIN_MENU;

	if (event->type == SDL_EVENT_KEY_UP)
	{
		switch (event->key.scancode)
		{

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
			m_guiContext.flags ^= GuiContextFlags_PAUSE;
			break;

		default:
			break;
		}
	}

	return SDL_APP_CONTINUE;
}

bool BudgetGB::loadCartridge(const std::string &cartridgePath)
{
	m_bus.clearBus();
	m_cpu.cpuReset();
	return m_cartridge.loadCartridgeFromPath(cartridgePath);
}

void BudgetGB::resizeViewport()
{
	int resizedWidth, resizedHeight, gl_viewportX, gl_viewportY;
	SDL_GetWindowSize(m_window, &resizedWidth, &resizedHeight);

	// aspect ratio of gameboy display is 10:9

	float widthRatio  = (float)resizedWidth / 10.0f;
	float heightRatio = (float)resizedHeight / 9.0f;

	Utils::struct_Vec2<uint32_t> viewportSize;

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

	gl_viewportX = (resizedWidth - viewportSize.x) / 2;
	gl_viewportY = (resizedHeight - viewportSize.y) / 2;

	RendererGB::setMainViewportSize(m_renderContext, gl_viewportX, gl_viewportY, viewportSize.x, viewportSize.y);

	// clamp window size to perfectly fit the 10:9 aspect ratio if below threshold
	/*if (false)
	{
	    float calcThreshold = SDL_fabsf((resizedWidth / 10.0f) - (resizedHeight / 9.0f));

	    if (calcThreshold < 2.5f)
	    {
	        if (widthRatio < heightRatio)
	            resizedHeight = static_cast<uint32_t>(widthRatio * 9);
	        else
	            resizedWidth = static_cast<uint32_t>(heightRatio * 10);

	        SDL_SetWindowSize(m_window, resizedWidth, resizedHeight);
	    }
	}*/
}

void BudgetGB::resizeWindowFixed(WindowScale scale)
{
	if (m_guiContext.flags & GuiContextFlags_FULLSCREEN)
	{
		m_guiContext.flags &= ~GuiContextFlags_FULLSCREEN;
		SDL_SetWindowFullscreen(m_window, false);
		SDL_SyncWindow(m_window);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // re-enable viewports when exiting fullscreen
	}

	m_guiContext.windowSizeSelector = 0;
	m_guiContext.windowSizeSelector |= 1 << static_cast<uint32_t>(scale);

	SDL_SetWindowSize(m_window, static_cast<int>(scale) * BudgetGB::LCD_WIDTH, static_cast<int>(scale) * BudgetGB::LCD_HEIGHT);

	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	/*int newWidth, newHeight;
	SDL_GetWindowSize(m_window, &newWidth, &newHeight);
	RendererGB::setMainViewportSize(m_renderContext, 0, 0, newWidth, newHeight);*/
}

static void SDLCALL fileDialogCallback(void *userdata, const char *const *filelist, int filter)
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

void BudgetGB::guiMain()
{
	// imgui demo window
	if (m_guiContext.flags & GuiContextFlags_SHOW_IMGUI_DEMO)
	{
		bool toggle = true;
		ImGui::ShowDemoWindow(&toggle);
		if (!toggle)
			m_guiContext.flags ^= GuiContextFlags_SHOW_IMGUI_DEMO;
	}

	// cpu viewer window
	if (m_guiContext.flags & GuiContextFlags_SHOW_CPU_VIEWER)
	{
		bool toggle = true;
		guiCpuViewer(&toggle);
		if (!toggle)
			m_guiContext.flags ^= GuiContextFlags_SHOW_CPU_VIEWER;
	}

	if (m_guiContext.flags & GuiContextFlags_SHOW_MAIN_MENU)
	{
		ImGui::OpenPopup("Main Menu");
		m_guiContext.flags ^= GuiContextFlags_SHOW_MAIN_MENU;
	}

	if (ImGui::BeginPopup("Main Menu", ImGuiWindowFlags_NoMove))
	{
		if (ImGui::MenuItem("Pause", "P", m_guiContext.flags & GuiContextFlags_PAUSE))
			m_guiContext.flags ^= GuiContextFlags_PAUSE;

		if (ImGui::MenuItem("Load ROM..."))
		{
			SDL_DialogFileFilter filters[] = {
				{"*.gb", "gb;gb"},
			};
			SDL_ShowOpenFileDialog(fileDialogCallback, this, m_window, filters, 1, nullptr, false);
		}

		if (ImGui::BeginMenu("Window Sizes"))
		{

			if (ImGui::MenuItem("1x1", "", m_guiContext.windowSizeSelector & 0x2))
				resizeWindowFixed(WindowScale::WindowScale_1x1);
			if (ImGui::MenuItem("1x2", "", m_guiContext.windowSizeSelector & 0x4))
				resizeWindowFixed(WindowScale::WindowScale_1x2);
			if (ImGui::MenuItem("1x3", "", m_guiContext.windowSizeSelector & 0x8))
				resizeWindowFixed(WindowScale::WindowScale_1x3);
			if (ImGui::MenuItem("1x4", "", m_guiContext.windowSizeSelector & 0x10))
				resizeWindowFixed(WindowScale::WindowScale_1x4);
			if (ImGui::MenuItem("1x5", "", m_guiContext.windowSizeSelector & 0x20))
				resizeWindowFixed(WindowScale::WindowScale_1x5);
			if (ImGui::MenuItem("1x6", "", m_guiContext.windowSizeSelector & 0x40))
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

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Toggle Imgui Demo", "", m_guiContext.flags & GuiContextFlags_SHOW_IMGUI_DEMO))
			m_guiContext.flags ^= GuiContextFlags_SHOW_IMGUI_DEMO;

		if (ImGui::MenuItem("CPU Viewer", "", m_guiContext.flags & GuiContextFlags_SHOW_CPU_VIEWER))
			m_guiContext.flags ^= GuiContextFlags_SHOW_CPU_VIEWER;

		ImGui::Separator();
		if (ImGui::MenuItem("Quit"))
		{
			SDL_Event event{};
			event.type = SDL_EVENT_QUIT;
			SDL_PushEvent(&event);
		}
		ImGui::EndPopup();
	}
}

void BudgetGB::guiCpuViewer(bool *toggle)
{
	if (ImGui::Begin("CPU Viewer", toggle))
	{
		if (ImGui::BeginTable("CPU Viewer Table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			if (ImGui::BeginTable("CPU Instruction Log", 1, ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("CPU Instructions");
				ImGui::TableHeadersRow();

				std::size_t position   = m_cpu.m_opcodeLogger.bufferPosition();
				std::size_t bufferSize = m_cpu.m_opcodeLogger.bufferSize();

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

			if (m_guiContext.flags & GuiContextFlags_PAUSE)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
				if (ImGui::Button("Pause"))
					m_guiContext.flags &= ~GuiContextFlags_PAUSE;
				ImGui::PopStyleColor(1);
			}
			else if (ImGui::Button("Pause"))
				m_guiContext.flags |= GuiContextFlags_PAUSE;

			ImGui::BeginDisabled(!m_cartridge.isLoaded() || !(m_guiContext.flags & GuiContextFlags_PAUSE));
			if (ImGui::Button("Instruction Step"))
				m_guiContext.flags |= GuiContextFlags_INSTRUCTION_STEP;
			ImGui::EndDisabled();

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
				m_guiContext.flags |= GuiContextFlags_TOGGLE_INSTRUCTION_LOG;
				m_cpu.m_opcodeLogger.startLog();
				m_cpu.m_logEnable = m_guiContext.flags & GuiContextFlags_TOGGLE_INSTRUCTION_LOG;
			}

			ImGui::PopStyleColor(2);

			ImGui::EndTable();
		}
	}
	ImGui::End();
}
