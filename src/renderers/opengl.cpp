#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

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
		SDL_Log("Deconstructing glContext");
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

#ifdef __APPLE__
	// opengl 4.1 with glsl 4.10
	const char *glsl_version = "#version 410";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

#ifdef _WIN32
	// opengl 4.3 with glsl 4.30
	const char *glsl_version = "#version 430";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#endif

	window = SDL_CreateWindow("Budget Gameboy", 600, 600, SDL_WINDOW_OPENGL);
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

	// set up ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigViewportsNoDefaultParent = true;

	ImGui::StyleColorsLight();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init(glsl_version);

	io.Fonts->AddFontFromFileTTF("resources/fonts/MononokiNerdFont-Regular.ttf", 18.0);
}

void RendererGB::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void RendererGB::render(SDL_Window *window)
{
	ImGui::Render();
	ImGuiIO& io = ImGui::GetIO();
	glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
	glClearColor(0.8784f, 0.9725f, 0.8156f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backupCurrentWindow = SDL_GL_GetCurrentWindow();
		SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
	}

	SDL_GL_SwapWindow(window);
}

void RendererGB::free(SDL_Window *&window, RenderContext *&renderContext)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DestroyContext(renderContext->glContext);
	delete renderContext;
	SDL_DestroyWindow(window);
}
