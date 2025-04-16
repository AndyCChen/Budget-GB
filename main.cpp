#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "BudgetGB.h"
#include "renderer.h"
#include "fmt/base.h"

#include <stdexcept>
#include <string>

struct AppWithTimer
{
	BudgetGB gameboy;

	float currentTime  = 0.0f;
	float previousTime = 0.0f;
	float deltaTime    = 0.0f;

	AppWithTimer(const std::string &romPath = "") : gameboy(romPath)
	{
	}
};

// initialization
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	try
	{
		if (argc < 2)
			*appstate = new AppWithTimer();
		else if (argc == 2)
			*appstate = new AppWithTimer(std::string(argv[1]));
		else
		{
			fmt::println(stderr,
			             "Invalid number of arguments, please provide only one path to rom file or none at all!");
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
	AppWithTimer *app     = (AppWithTimer *)appstate;
	BudgetGB     &gameboy = app->gameboy;

	gameboy.onUpdate(app->currentTime - app->previousTime);

	app->previousTime = app->currentTime;
	app->currentTime  = SDL_GetTicks() / 1000.0f;

	return SDL_APP_CONTINUE;
}

// runs when SDL event occurs
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	BudgetGB &gameboy = ((AppWithTimer *)appstate)->gameboy;
	return gameboy.processEvent(event);
}

// clean up on exit
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	(void) result;
	AppWithTimer *app = (AppWithTimer *)appstate;
	delete app;
}