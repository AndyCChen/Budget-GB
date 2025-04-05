#include "glad/glad.h"
#include "renderer.h"

#include <cstdlib>
#include <cstdint>

namespace
{
SDL_GLContext m_glContext;
}

void RendererGB::init(SDL_Window *&window)
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
	window = SDL_CreateWindow("Budget Gameboy", 600, 600, windowFlags);
	if (window == nullptr)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	m_glContext = SDL_GL_CreateContext(window);
	if (m_glContext == nullptr)
	{
		SDL_LogError(0, "Failed to create openGL context! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_GL_MakeCurrent(window, m_glContext);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		SDL_LogError(0, "Failed to load initialize glad!");
		std::exit(1);
	}

	SDL_GL_SetSwapInterval(1); // vsync
}

void RendererGB::render(SDL_Window *window)
{
	glClearColor(0.8784f, 0.9725f, 0.8156f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
}

void RendererGB::free(SDL_Window *&window)
{
	SDL_GL_DestroyContext(m_glContext);
	SDL_DestroyWindow(window);
}
