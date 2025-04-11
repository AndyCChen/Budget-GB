#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
#include "glad/glad.h"
#include "renderer.h"
#include "shader.h"

#include <string>

namespace
{

struct GbMainViewport
{
	GLuint m_viewportVAO, m_viewportVBO, m_viewportEBO, m_viewportTexture, m_viewportFBO,
		m_viewportFBOcolorAttachTexture;
	Shader m_viewportShader;

	GbMainViewport();
	~GbMainViewport();
	void draw();
};

} // namespace

struct RendererGB::RenderContext
{
	SDL_GLContext  glContext;
	GbMainViewport mainViewport;

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

namespace
{
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                     const GLchar *message, const void *userParam);
} // namespace

void RendererGB::initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext)
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		std::exit(1);
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

	window = SDL_CreateWindow("Budget Gameboy", 600, 600, SDL_WINDOW_OPENGL);
	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		std::exit(1);
	}

	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
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
	SDL_Log("OpenGl Version: %s", (char *)glGetString(GL_VERSION));

	// set up ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigViewportsNoDefaultParent = true;
	io.Fonts->AddFontFromFileTTF("resources/fonts/MononokiNerdFont-Regular.ttf", 18.0);
	ImGui::StyleColorsLight();

	ImGui_ImplSDL3_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init(glsl_version);

#ifdef ENABLE_GL_DEBUG_CALLBACK
	glDebugMessageCallback(GLDebugMessageCallback, NULL);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

	renderContext            = new RenderContext;
	renderContext->glContext = glContext;
}

void RendererGB::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void RendererGB::drawMainViewport(std::vector<Utils::vec3> &pixelBuffer, RenderContext *renderContext)
{
	glBindTexture(GL_TEXTURE_2D, renderContext->mainViewport.m_viewportTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, BudgetGBconstants::LCD_WIDTH, BudgetGBconstants::LCD_HEIGHT, GL_RGB,
	                GL_UNSIGNED_BYTE, pixelBuffer.data());

	renderContext->mainViewport.m_viewportShader.useProgram();
	renderContext->mainViewport.draw();
}

void RendererGB::endFrame(SDL_Window *window)
{
	ImGui::Render();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window   *backupCurrentWindow  = SDL_GL_GetCurrentWindow();
		SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
	}

	SDL_GL_SwapWindow(window);
}

void RendererGB::freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DestroyContext(renderContext->glContext);
	delete renderContext;
	SDL_DestroyWindow(window);
	SDL_Quit();
}

namespace
{

GbMainViewport::GbMainViewport()
	: m_viewportShader("resources/shaders/opengl/viewport.vert", "resources/shaders/opengl/viewport.frag")
{
	using namespace BudgetGBconstants;

	// clang-format off
	// quad vertices with texure coordinates
	float quad[] =
	{
		-1.0f, -1.0f, 0.5f,   0.0f, 0.0f, // bottom left
		 1.0f, -1.0f, 0.5f,   0.0f, 1.0f, // bottom right
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, LCD_WIDTH, LCD_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GbMainViewport::~GbMainViewport()
{
}

void GbMainViewport::draw()
{
	glClear(GL_COLOR_BUFFER_BIT);
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
