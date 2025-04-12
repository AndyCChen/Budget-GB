#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "BudgetGB.h"
#include "fmt/base.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"

 SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	BudgetGB *gameboy = new BudgetGB();
	*appstate         = gameboy;

	return SDL_APP_CONTINUE;
 }

 SDL_AppResult SDL_AppIterate(void *appstate)
{
	BudgetGB *gameboy = (BudgetGB *)appstate;

	gameboy->onUpdate();

	return SDL_APP_CONTINUE;
 }

 SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	ImGui_ImplSDL3_ProcessEvent(event);
	BudgetGB   *gameboy = (BudgetGB *)appstate;
	SDL_Window *window  = SDL_GL_GetCurrentWindow();

	if (event->type == SDL_EVENT_QUIT)
		return SDL_APP_SUCCESS;
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event->window.windowID == SDL_GetWindowID(window))
		return SDL_APP_SUCCESS;
	

	if (event->type == SDL_EVENT_WINDOW_RESIZED && event->window.windowID == SDL_GetWindowID(window))
	{
		gameboy->m_resizing = true;
		gameboy->resizeViewport();
	}

	if (!ImGui::GetIO().WantCaptureMouse && event->type == SDL_EVENT_MOUSE_BUTTON_UP)
		if (event->button.button == 3) // right mouse button
			gameboy->m_options.openMenu = true;

	return SDL_APP_CONTINUE;
 }

 void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	BudgetGB *gameboy = (BudgetGB *)appstate;
	delete gameboy;
 }