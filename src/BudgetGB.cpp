#include "BudgetGB.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

#include "glad/glad.h"

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

	std::random_device              rd;
	std::mt19937                    gen(rd());
	std::uniform_int_distribution<> palleteRange(0, 4);

	unsigned char colorPallete[][3] = {{8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208}, {255, 255, 255}};

	for (std::size_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); ++i)
	{
		unsigned char colorIdx = palleteRange(gen);
		m_lcdPixelBuffer[i][0] = colorPallete[colorIdx][0];
		m_lcdPixelBuffer[i][1] = colorPallete[colorIdx][1];
		m_lcdPixelBuffer[i][2] = colorPallete[colorIdx][2];
	}
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
		m_cpu.runInstruction();

		RendererGB::newFrame();

		if (m_showImGuiDemo)
			ImGui::ShowDemoWindow(&m_showImGuiDemo);

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
				resizeViewport();

			ImGui::Separator();
			ImGui::Selectable("Quit");
			ImGui::EndPopup();
		}

		RendererGB::drawMainViewport(m_lcdPixelBuffer, m_renderContext);
		RendererGB::endFrame(m_window);
	}
}

void BudgetGB::onUpdate()
{
	m_cpu.runInstruction();
	RendererGB::drawMainViewport(m_lcdPixelBuffer, m_renderContext);

	//if (!m_resizing)
	{
		RendererGB::newFrame();

		if (m_showImGuiDemo)
			ImGui::ShowDemoWindow(&m_showImGuiDemo);

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
				resizeViewport();

			ImGui::Separator();
			ImGui::Selectable("Quit");
			ImGui::EndPopup();
		}

		RendererGB::endFrame(m_window);
	}

	

	SDL_GL_SwapWindow(m_window);
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

		/*if (event.type == SDL_EVENT_WINDOW_EXPOSED && event.window.windowID == SDL_GetWindowID(m_window))
		    fmt::println("exposed");*/

		if (event.type == SDL_EVENT_WINDOW_RESIZED && event.window.windowID == SDL_GetWindowID(m_window))
		{
			resizeViewport();
		}

		

		if (!ImGui::GetIO().WantCaptureMouse && event.type == SDL_EVENT_MOUSE_BUTTON_UP)
			if (event.button.button == 3) // right mouse button
				m_options.openMenu = true;
	}
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

	glViewport(gl_viewportX, gl_viewportY, viewportSize.x, viewportSize.y);

	// clamp window size to perfectly fit the 10:9 aspect ratio if below threshold
	if (false)
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
	}
}
