#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "BudgetGB.h"
#include "fmt/base.h"
#include "renderer.h"

#include <stdexcept>
#include <string>

// initialization
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	try
	{
		if (argc < 2)
			*appstate = new BudgetGB();
		else if (argc == 2)
			*appstate = new BudgetGB(std::string(argv[1]));
		else
		{
			fmt::println(stderr, "Invalid number of arguments, please provide only one path to rom file or none at all!");
			return SDL_APP_FAILURE;
		}
	}
	catch (const std::runtime_error &error)
	{
		fmt::println(stderr, "{}", error.what());
		return SDL_APP_FAILURE;
	}

	RendererGB::newFrame(); // begin initial new frame before entering gameloop
	return SDL_APP_CONTINUE;
}

// runs around once per frame
SDL_AppResult SDL_AppIterate(void *appstate)
{
	BudgetGB *gameboy = (BudgetGB *)appstate;
	gameboy->onUpdate();
	return SDL_APP_CONTINUE;
}

// runs when SDL event occurs
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	BudgetGB *gameboy = (BudgetGB *)appstate;
	return gameboy->processEvent(event);
}

// clean up on exit
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	(void)result;
	BudgetGB *gameboy = (BudgetGB *)appstate;
	delete gameboy;
}