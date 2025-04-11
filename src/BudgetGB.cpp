#include "BudgetGB.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

#include <random>

using namespace BudgetGBconstants;

BudgetGB::BudgetGB() : m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	m_lcdPixelBuffer.resize(LCD_WIDTH * LCD_HEIGHT);
	RendererGB::initWindowWithRenderer(m_window, m_renderContext);
}

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	m_lcdPixelBuffer.resize(LCD_WIDTH * LCD_HEIGHT);
	RendererGB::initWindowWithRenderer(m_window, m_renderContext);
	m_cartridge.loadRomFromPath(romPath);
}

BudgetGB::~BudgetGB()
{
	RendererGB::freeWindowWithRenderer(m_window, m_renderContext);
}

void BudgetGB::run()
{
	std::random_device              rd;
	std::mt19937                    gen(rd());
	std::uniform_int_distribution<> distrib(0, 4);

	unsigned char colorPallete[][3] = {{8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208}, {255, 255, 255}};

	for (std::size_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; ++i)
	{
		unsigned char colorIdx = distrib(gen);
		m_lcdPixelBuffer[i][0] = colorPallete[colorIdx][0];
		m_lcdPixelBuffer[i][1] = colorPallete[colorIdx][1];
		m_lcdPixelBuffer[i][2] = colorPallete[colorIdx][2];
	}

	while (m_isRunning)
	{
		gbProcessEvent();
		m_cpu.runInstruction();

		RendererGB::newFrame();

		static bool flag = true;
		if (flag)
			ImGui::ShowDemoWindow(&flag);

		if (m_openMenu)
		{
			ImGui::OpenPopup("Main Menu");
			m_openMenu = false;
		}

		if (ImGui::BeginPopup("Main Menu", ImGuiWindowFlags_NoMove))
		{
			ImGui::SeparatorText("Menu");
			if (ImGui::Button("Close me"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		RendererGB::drawMainViewport(m_lcdPixelBuffer, m_renderContext);
		RendererGB::endFrame(m_window);
	}
}

void BudgetGB::gbProcessEvent()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL3_ProcessEvent(&event);

		if (event.type == SDL_EVENT_QUIT)
			m_isRunning = false;
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(m_window))
			m_isRunning = false;

		if (!ImGui::GetIO().WantCaptureMouse && event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			if (event.button.button == 3) // right mouse button
				m_openMenu = true;
	}
}
