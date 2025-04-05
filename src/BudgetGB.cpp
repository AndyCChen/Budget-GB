#include "glad/glad.h"

#include "BudgetGB.h"

#include <cstdio>
#include <cstdlib>

BudgetGB::BudgetGB() : m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	initSDL_GL();
}

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	initSDL_GL();
	m_cartridge.loadRomFromPath(romPath);
}

BudgetGB::~BudgetGB()
{
	SDL_GL_DestroyContext(m_glContext);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}

void BudgetGB::run()
{
	while (m_isRunning)
	{
		gbProcessEvent();

		m_cpu.runInstruction();

		glClearColor(0.8784, 0.9725, 0.8156, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(m_window);
	}
}

void BudgetGB::initSDL_GL()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	// opengl 4.3 with glsl 4.30
	// const char *glsl_version = "#version 430";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	uint32_t windowFlags = SDL_WINDOW_OPENGL;
	m_window = SDL_CreateWindow("Budget Gameboy", 600, 600, windowFlags);
	if (m_window == nullptr)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	m_glContext = SDL_GL_CreateContext(m_window);
	if (m_glContext == nullptr)
	{
		SDL_LogError(0, "Failed to create openGL context! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_GL_MakeCurrent(m_window, m_glContext);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		SDL_LogError(0, "Failed to load initialize glad!");
		std::exit(1);
	}

	SDL_GL_SetSwapInterval(1); // vsync
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
