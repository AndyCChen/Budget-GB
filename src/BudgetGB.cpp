#include "renderer.h"
#include "BudgetGB.h"

#include <cstdio>
#include <cstdlib>

BudgetGB::BudgetGB() : m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	RendererGB::init(m_window);
}

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	RendererGB::init(m_window);
	m_cartridge.loadRomFromPath(romPath);
}

BudgetGB::~BudgetGB()
{
	RendererGB::free(m_window);
	SDL_Quit();
}

void BudgetGB::run()
{
	while (m_isRunning)
	{
		gbProcessEvent();

		m_cpu.runInstruction();

		RendererGB::render(m_window);
	}
}

void BudgetGB::gbProcessEvent()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
			m_isRunning = false;
		if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(m_window))
			m_isRunning = false;
	}
}
