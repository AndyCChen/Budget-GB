#include "BudgetGB.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

#include <random>

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	m_lcdPixelBuffer.resize(LCD_WIDTH * LCD_HEIGHT);
	RendererGB::initWindowWithRenderer(m_window, m_renderContext);

	if (romPath != "")
	{
		m_cartridge.loadRomFromPath(romPath);
	}
}

BudgetGB::~BudgetGB()
{
	RendererGB::freeWindowWithRenderer(m_window, m_renderContext);
}

void BudgetGB::run()
{
	std::random_device              rd;
	std::mt19937                    gen(rd());
	std::uniform_int_distribution<> palleteRange(0, 4);
	std::uniform_int_distribution<> pixelBufferRange(0, (LCD_WIDTH * LCD_HEIGHT) - 1);

	unsigned char colorPallete[][3] = {{8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208}, {255, 255, 255}};

	float currentTime  = SDL_GetTicks() / 1000.0f;
	float previousTime = 0;
	float deltaTime    = 0;

	bool showImGuiDemo = true;
	while (m_isRunning)
	{
		gbProcessEvent();
		m_cpu.runInstruction();

		RendererGB::newFrame();

		if (showImGuiDemo)
			ImGui::ShowDemoWindow(&showImGuiDemo);

		if (m_options.openMenu)
		{
			ImGui::OpenPopup("Main Menu");
			m_options.openMenu = false;
		}

		if (ImGui::BeginPopup("Main Menu", ImGuiWindowFlags_NoMove))
		{
			ImGui::Selectable("Pause");
			ImGui::Selectable("Load ROM...");

			if (ImGui::MenuItem("Toggle clamp", "", &m_options.toggleClamp) && m_options.toggleClamp)
				clampViewport();

			ImGui::Separator();
			ImGui::Selectable("Quit");
			ImGui::EndPopup();
		}

		if (deltaTime > 1.0f)
		{
			deltaTime -= 1.0f;
			for (std::size_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT) / 3; ++i)
			{
				unsigned char colorIdx         = palleteRange(gen);
				std::size_t   bufferIdx        = pixelBufferRange(gen);
				m_lcdPixelBuffer[bufferIdx][0] = colorPallete[colorIdx][0];
				m_lcdPixelBuffer[bufferIdx][1] = colorPallete[colorIdx][1];
				m_lcdPixelBuffer[bufferIdx][2] = colorPallete[colorIdx][2];
			}
		}

		RendererGB::drawMainViewport(m_lcdPixelBuffer, m_renderContext);
		RendererGB::endFrame(m_window);

		previousTime = currentTime;
		currentTime  = SDL_GetTicks() / 1000.0f;
		deltaTime += currentTime - previousTime;
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

		if (m_options.toggleClamp && event.type == SDL_EVENT_WINDOW_RESIZED &&
		    event.window.windowID == SDL_GetWindowID(m_window))
		{
			clampViewport();
		}

		if (!ImGui::GetIO().WantCaptureMouse && event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			if (event.button.button == 3) // right mouse button
				m_options.openMenu = true;
	}
}

void BudgetGB::clampViewport()
{
	int width, height;
	SDL_GetWindowSize(m_window, &width, &height);

	// clamp window size to perfectly fit the 10:9 aspect ratio if below threshold
	float calcThreshold = SDL_fabsf((width / 10.0f) - (height / 9.0f));

	if (calcThreshold < 2.5f)
	{
		float widthRatio  = (float)width / 10.0f;
		float heightRatio = (float)height / 9.0f;

		if (widthRatio < heightRatio)
			height = static_cast<uint32_t>(widthRatio * 9);
		else
			width = static_cast<uint32_t>(heightRatio * 10);

		SDL_SetWindowSize(m_window, width, height);
	}
}
