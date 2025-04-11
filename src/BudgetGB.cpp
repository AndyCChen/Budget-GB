#include "BudgetGB.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

BudgetGB::BudgetGB() : m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	m_lcdPixels.resize(BudgetGBConstants::LCD_WIDTH * BudgetGBConstants::LCD_HEIGHT * 3);
	RendererGB::initWindowWithRenderer(m_window, m_renderContext);
}

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	m_lcdPixels.resize(BudgetGBConstants::LCD_WIDTH * BudgetGBConstants::LCD_HEIGHT * 3);
	RendererGB::initWindowWithRenderer(m_window, m_renderContext);
	m_cartridge.loadRomFromPath(romPath);
}

BudgetGB::~BudgetGB()
{
	RendererGB::freeWindowWithRenderer(m_window, m_renderContext);
}

void BudgetGB::run()
{
	while (m_isRunning)
	{
		gbProcessEvent();

		RendererGB::newFrame();

		static bool flag = true;
		if (flag)
			ImGui::ShowDemoWindow(&flag);

		if (m_openMenu)
		{
			ImGui::OpenPopup("group1");
			m_openMenu = false;
		}

		if (ImGui::BeginPopup("group1", ImGuiWindowFlags_NoMove))
		{
			if (ImGui::Button("Close me"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		m_cpu.runInstruction();
		RendererGB::drawMainViewport(m_lcdPixels, m_renderContext);

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
