#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
#include "fmt/base.h"
#include "glad/glad.h"
#include "renderer.h"
#include "shader.h"

#include <cstdint>
#include <string>

namespace
{

struct GbMainViewport
{
	GLuint m_viewportVAO, m_viewportVBO, m_viewportEBO, m_viewportTexture;
	Shader m_viewportShader;

	Utils::struct_Vec2<uint32_t> m_viewportSize;
	Utils::struct_Vec2<uint32_t> m_viewportXY;

	GbMainViewport();
	~GbMainViewport();
	void draw();
};

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                     const GLchar *message, const void *userParam);

} // namespace

// RenderContext can only be instantiated after a valid sdl window and sdl opengl context has been created
struct RendererGB::RenderContext
{
	SDL_GLContext  m_glContext;
	GbMainViewport m_mainViewport;

	RenderContext()
	{
		m_glContext = NULL;
	}
};

bool RendererGB::initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale)
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		return false;
	}

#ifdef USE_GL_VERSION_410
	// opengl 4.1 with glsl 4.10
	const char *glsl_version = "#version 410";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#else
// debug callback only available on opengl version 4.3 and onwards which means no mac
#define ENABLE_GL_DEBUG_CALLBACK

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

	window = SDL_CreateWindow("Budget Gameboy", BudgetGB::LCD_WIDTH * BudgetGB::INITIAL_WINDOW_SCALE,
	                          BudgetGB::LCD_HEIGHT * windowScale, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		return false;
	}

	SDL_SetWindowMinimumSize(window, BudgetGB::LCD_WIDTH, BudgetGB::LCD_HEIGHT);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if (!glContext)
	{
		SDL_LogError(0, "Failed to create openGL context! SDL error: %s", SDL_GetError());
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		SDL_LogError(0, "Failed to load initialize glad!");
		return false;
	}

	SDL_GL_MakeCurrent(window, glContext);
	SDL_GL_SetSwapInterval(1); // vsync
	SDL_Log("OpenGl Version: %s", (char *)glGetString(GL_VERSION));

	// set up ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	// io.ConfigViewportsNoAutoMerge = true;
	io.ConfigViewportsNoDefaultParent = true;

	io.Fonts->AddFontFromFileTTF("resources/fonts/MononokiNerdFont-Regular.ttf", 16.0);
	ImGui::StyleColorsLight();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init(glsl_version);

#ifdef ENABLE_GL_DEBUG_CALLBACK
	glDebugMessageCallback(GLDebugMessageCallback, NULL);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	renderContext = new RenderContext; // create renderContext only after window and opengl context are setup
	renderContext->m_glContext = glContext;
	// SDL_AddEventWatch(eventWatchCallback, (void *)renderContext);
	SDL_ShowWindow(window);

	return true;
}

void RendererGB::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void RendererGB::setMainViewportSize(RenderContext *renderContext, int x, int y, int width, int height)
{
	renderContext->m_mainViewport.m_viewportXY.x = x;
	renderContext->m_mainViewport.m_viewportXY.y = y;
	renderContext->m_mainViewport.m_viewportSize.x = width;
	renderContext->m_mainViewport.m_viewportSize.y = height;
}

void RendererGB::drawMainViewport(std::vector<Utils::array_u8Vec3> &pixelBuffer, RenderContext *renderContext)
{
	glBindTexture(GL_TEXTURE_2D, renderContext->m_mainViewport.m_viewportTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BudgetGB::LCD_WIDTH, BudgetGB::LCD_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
	                pixelBuffer.data());

	renderContext->m_mainViewport.m_viewportShader.useProgram();
	renderContext->m_mainViewport.draw();
}

void RendererGB::endFrame(SDL_Window *window, RenderContext *renderContext)
{
	(void) renderContext;
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		// SDL_Window   *backupCurrentWindow  = SDL_GL_GetCurrentWindow();
		SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(window, backupCurrentContext);
	}

	SDL_GL_SwapWindow(window);
}

void RendererGB::freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DestroyContext(renderContext->m_glContext);
	delete renderContext;
	SDL_DestroyWindow(window);
	SDL_Quit();
}

namespace
{

GbMainViewport::GbMainViewport()
	: m_viewportShader("resources/shaders/opengl/viewport.vert", "resources/shaders/opengl/viewport.frag")
{
	{
		SDL_Window *window = SDL_GL_GetCurrentWindow();

		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		m_viewportSize.x = width;
		m_viewportSize.y = height;
		m_viewportXY.x   = 0;
		m_viewportXY.y   = 0;
	}

	// clang-format off
	// quad vertices with texure coordinates
	float quad[] =
	{
		-1.0f, -1.0f, 0.5f,   0.0f, 0.0f, // bottom left
		 1.0f, -1.0f, 0.5f,   1.0f, 0.0f, // bottom right
		 1.0f,  1.0f, 0.5f,   1.0f, 1.0f, // top right
		-1.0f,  1.0f, 0.5f,   0.0f, 1.0f, // top left
	};

	GLuint indices[] =
	{
		0, 1, 2,
		2, 3, 0,
	};
	// clang-format on

	// set up buffers for main viewport mesh
	glGenVertexArrays(1, &m_viewportVAO);
	glGenBuffers(1, &m_viewportVBO);
	glGenBuffers(1, &m_viewportEBO);

	glBindVertexArray(m_viewportVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_viewportVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_viewportEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindVertexArray(0);

	// set up texture that will be rendered onto main viewport mesh
	glGenTextures(1, &m_viewportTexture);

	glBindTexture(GL_TEXTURE_2D, m_viewportTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, BudgetGB::LCD_WIDTH, BudgetGB::LCD_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE,
	             NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GbMainViewport::~GbMainViewport()
{
	glDeleteVertexArrays(1, &m_viewportVAO);
	glDeleteTextures(1, &m_viewportTexture);
	glDeleteBuffers(1, &m_viewportEBO);
	glDeleteBuffers(1, &m_viewportVBO);
}

void GbMainViewport::draw()
{
	glClearColor(0.3f, 0.6f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(m_viewportXY.x, m_viewportXY.y, m_viewportSize.x, m_viewportSize.y);
	glBindVertexArray(m_viewportVAO);
	glBindTexture(GL_TEXTURE_2D, m_viewportTexture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                     const GLchar *message, const void *userParam)
{
	(void)userParam;
	(void)length;
	std::string _source;
	std::string _type;
	std::string _severity;

	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:
		_source = "API";
		break;

	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM";
		break;

	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER";
		break;

	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY";
		break;

	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION";
		break;

	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN";
		break;

	default:
		_source = "UNKNOWN";
		break;
	}

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR";
		break;

	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY";
		break;

	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE";
		break;

	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER";
		break;

	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER";
		break;

	default:
		_type = "UNKNOWN";
		break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH";
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM";
		break;

	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW";
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION";
		break;

	default:
		_severity = "UNKNOWN";
		break;
	}

	SDL_LogError(0, "%d: %s of %s severity, raised from %s: %s", id, _type.c_str(), _severity.c_str(), _source.c_str(),
	             message);
}
} // namespace
