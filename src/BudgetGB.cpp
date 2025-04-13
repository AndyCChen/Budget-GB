#include "BudgetGB.h"
#include "fmt/base.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

#include "glad/glad.h"

#include <stdexcept>

static unsigned char colorPallete[][3] = {
	{8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208}, {255, 255, 255}};

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler), m_gen(m_rd()),
	  m_palleteRange(0, 4)
{
	m_lcdPixelBuffer.resize(LCD_WIDTH * LCD_HEIGHT);

	if (!RendererGB::initWindowWithRenderer(m_window, m_renderContext))
	{
		throw std::runtime_error("Failed to initialize window with renderer!");
	}

	if (romPath != "")
	{
		m_cartridge.loadRomFromPath(romPath);
	}
}

BudgetGB::~BudgetGB()
{
	RendererGB::freeWindowWithRenderer(m_window, m_renderContext);
}

void BudgetGB::onUpdate(float deltaTime)
{
	m_accumulatedDeltaTime += deltaTime;

	if (m_accumulatedDeltaTime > 1.0f)
	{
		m_accumulatedDeltaTime -= 1.0f;
		for (std::size_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); ++i)
		{
			unsigned char colorIdx = m_palleteRange(m_gen);
			m_lcdPixelBuffer[i][0] = colorPallete[colorIdx][0];
			m_lcdPixelBuffer[i][1] = colorPallete[colorIdx][1];
			m_lcdPixelBuffer[i][2] = colorPallete[colorIdx][2];
		}
	}

	m_cpu.runInstruction();

	RendererGB::newFrame();
	RendererGB::drawMainViewport(m_lcdPixelBuffer, m_renderContext);

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

SDL_AppResult BudgetGB::processEvent(SDL_Event *event)
{
	ImGui_ImplSDL3_ProcessEvent(event);

	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;
	if (event->window.windowID == SDL_GetWindowID(m_window) && event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		return SDL_APP_SUCCESS;

	if (event->window.windowID == SDL_GetWindowID(m_window) && event->type == SDL_EVENT_WINDOW_RESIZED)
		resizeViewport();

	if (!ImGui::GetIO().WantCaptureMouse && event->type == SDL_EVENT_MOUSE_BUTTON_UP)
		if (event->button.button == 3) // right mouse button
			m_options.openMenu = true;

	return SDL_APP_CONTINUE;
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
