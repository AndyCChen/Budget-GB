#include "glad/glad.h"
#include "renderer.h"

#include <cstdlib>

struct RendererGB::RenderContext
{
	SDL_GLContext glContext;

	RenderContext()
	{
		glContext = NULL;
		SDL_Log("Constructing glContext");
	}

	~RenderContext()
	{
		SDL_Log("Destructing glContext");
	}
};

void RendererGB::init(SDL_Window *&window, RenderContext *&renderContext)
{
	renderContext = new RenderContext;
	SDL_GLContext &glContext = renderContext->glContext;

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

	SDL_WindowFlags windowFlags = static_cast<SDL_WindowFlags>(SDL_WINDOW_OPENGL);
	window = SDL_CreateWindow("Budget Gameboy", 600, 600, windowFlags);
	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	glContext = SDL_GL_CreateContext(window);
	if (!glContext)
	{
		SDL_LogError(0, "Failed to create openGL context! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_GL_MakeCurrent(window, glContext);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		SDL_LogError(0, "Failed to load initialize glad!");
		std::exit(1);
	}

	SDL_GL_SetSwapInterval(1); // vsync
	SDL_Log("OpenGl Version: %s", (char*)glGetString(GL_VERSION));
}

void RendererGB::render(SDL_Window *window)
{
	glClearColor(0.8784f, 0.9725f, 0.8156f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
}

void RendererGB::free(SDL_Window *&window, RenderContext *&renderContext)
{
	SDL_GL_DestroyContext(renderContext->glContext);
	delete renderContext;
	SDL_DestroyWindow(window);
}
