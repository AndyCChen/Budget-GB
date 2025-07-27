#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include "emulatorConstants.h"
#include "glad/glad.h"
#include "renderer.h"
#include "shader.h"
#include "utils/vec.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <string>

#ifndef USE_GL_VERSION_410
#define ENABLE_GL_DEBUG_CALLBACK // debug callback only available on opengl version 4.3 and onwards which means no mac
#endif

namespace
{

#ifdef ENABLE_GL_DEBUG_CALLBACK
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#endif

struct Vertex
{
	std::array<float, 3> Position;
	std::array<float, 2> TextureCoord;
};

struct GbMainViewport
{
	Utils::Vec2<uint32_t> ViewportSize;
	Utils::Vec2<uint32_t> ViewportTopLeft;
};

} // namespace

// RenderContext can only be instantiated after a valid sdl window and sdl opengl context has been created
struct RendererGB::RenderContext
{
	SDL_GLContext  GlContext = nullptr;
	GbMainViewport MainViewport;

	Shader MainShaders;
	GLuint BufferEBO;

	RenderContext(SDL_GLContext glContext);
};

struct RendererGB::TextureRenderTarget
{
	GLuint TextureID               = 0;
	GLuint RenderTargetFrameBuffer = 0;

	~TextureRenderTarget()
	{
		glDeleteTextures(1, &TextureID);
		glDeleteFramebuffers(1, &RenderTargetFrameBuffer);
	}
};

struct RendererGB::TexturedQuad
{
	GLuint Vao          = 0;
	GLuint BufferVertex = 0;
	GLuint TextureID    = 0;

	const Utils::Vec2<float> TextureSize;

	TexturedQuad(const Utils::Vec2<float> &textureSize)
		: TextureSize(textureSize)
	{
	}

	~TexturedQuad()
	{
		glDeleteVertexArrays(1, &Vao);
		glDeleteBuffers(1, &BufferVertex);
		glDeleteTextures(1, &TextureID);
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

	window = SDL_CreateWindow("Budget Gameboy", BudgetGbConstants::LCD_WIDTH * windowScale, BudgetGbConstants::LCD_HEIGHT * windowScale, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		return false;
	}

	SDL_SetWindowMinimumSize(window, BudgetGbConstants::LCD_WIDTH, BudgetGbConstants::LCD_HEIGHT);
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

	renderContext = new RenderContext(glContext); // create renderContext only after window and opengl context are setup

	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	renderContext->MainViewport.ViewportSize.x    = width;
	renderContext->MainViewport.ViewportSize.y    = height;
	renderContext->MainViewport.ViewportTopLeft.x = 0;
	renderContext->MainViewport.ViewportTopLeft.y = 0;

	SDL_ShowWindow(window);

	return true;
}

void RendererGB::newFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void RendererGB::mainViewportResize(RenderContext *renderContext, int x, int y, int width, int height)
{
	renderContext->MainViewport.ViewportSize.x    = width;
	renderContext->MainViewport.ViewportSize.y    = height;
	renderContext->MainViewport.ViewportTopLeft.x = x;
	renderContext->MainViewport.ViewportTopLeft.y = y;
}

void RendererGB::mainViewportSetRenderTarget(RenderContext *renderContext)
{
	renderContext->MainShaders.useProgram();
	GLint uniformInvertedYLocation = glGetUniformLocation(renderContext->MainShaders.ID(), "invertedYTextureCoord");
	glUniform1ui(uniformInvertedYLocation, false);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(renderContext->MainViewport.ViewportTopLeft.x, renderContext->MainViewport.ViewportTopLeft.y, renderContext->MainViewport.ViewportSize.x, renderContext->MainViewport.ViewportSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RendererGB::setGlobalPalette(RenderContext *renderContext, const BudgetGbConfig::Palette &palette)
{
	renderContext->MainShaders.useProgram();
	GLint uniformPaletteLocation = glGetUniformLocation(renderContext->MainShaders.ID(), "PALETTE");

	const std::array<std::array<float, 3>, 4> newPalette{palette.color0, palette.color1, palette.color2, palette.color3};

	glUniform3fv(uniformPaletteLocation, 4, newPalette[0].data());
}

void RendererGB::endFrame(SDL_Window *window, RenderContext *renderContext)
{
	(void)renderContext;
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
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

	SDL_GL_DestroyContext(renderContext->GlContext);
	delete renderContext;
	SDL_DestroyWindow(window);
	SDL_Quit();
}

RendererGB::RenderContext::RenderContext(SDL_GLContext glContext)
	: GlContext(glContext), MainShaders("resources/shaders/opengl/viewport.vert", "resources/shaders/opengl/viewport.frag")
{
	// clang-format off
	GLuint indices[] =
	{
		0, 1, 2,
		2, 3, 0,
	};
	// clang-format on

	glGenBuffers(1, &BufferEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
}

RendererGB::TextureRenderTargetUniquePtr RendererGB::textureRenderTargetCreate(RenderContext *renderContext, const Utils::Vec2<float> &size)
{
	(void)renderContext;

	TextureRenderTargetUniquePtr renderTargetTexture(new TextureRenderTarget());

	glGenFramebuffers(1, &renderTargetTexture->RenderTargetFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderTargetTexture->RenderTargetFrameBuffer);

	glGenTextures(1, &renderTargetTexture->TextureID);
	glBindTexture(GL_TEXTURE_2D, renderTargetTexture->TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargetTexture->TextureID, 0);

	return renderTargetTexture;
}

void RendererGB::textureRenderTargetFree(TextureRenderTarget *&renderTargetTexture)
{
	delete renderTargetTexture;
}

void RendererGB::textureRenderTargetSet(RenderContext *renderContext, TextureRenderTarget *renderTargetTexture, const Utils::Vec2<float> &viewport)
{
	(void)renderContext;

	// Drawing to framebuffer texture is flipped for some reason even though our texture is loaded bottom to top.
	// I invert the Y texture coord when drawing to framebuffer to fix this problem.

	renderContext->MainShaders.useProgram();
	GLint uniformInvertedYLocation = glGetUniformLocation(renderContext->MainShaders.ID(), "invertedYTextureCoord");
	glUniform1ui(uniformInvertedYLocation, true);

	glBindFramebuffer(GL_FRAMEBUFFER, renderTargetTexture->RenderTargetFrameBuffer);
	glViewport(0, 0, (GLsizei)viewport.x, (GLsizei)viewport.y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RendererGB::textureRenderTargetResize(RenderContext *renderContext, TextureRenderTarget *renderTargetTexture, const Utils::Vec2<float> &size)
{
	(void)renderContext;
	glBindTexture(GL_TEXTURE_2D, renderTargetTexture->TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

ImTextureID RendererGB::textureRenderTargetGetTextureID(TextureRenderTarget *renderTargetTexture)
{
	return (ImTextureID)(intptr_t)renderTargetTexture->TextureID;
}

RendererGB::TexturedQuadUniquePtr RendererGB::texturedQuadCreate(RenderContext *renderContext, const Utils::Vec2<float> &textureSize)
{
	TexturedQuadUniquePtr texturedQuad(new TexturedQuad(textureSize));

	// clang-format off
	Vertex vertices[] = 
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
	};
	// clang-format-on

	glGenVertexArrays(1, &texturedQuad->Vao);
	glBindVertexArray(texturedQuad->Vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderContext->BufferEBO);

	glGenBuffers(1, &texturedQuad->BufferVertex);
	glBindBuffer(GL_ARRAY_BUFFER, texturedQuad->BufferVertex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, (GLint)vertices->Position.size(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, (GLint)vertices->TextureCoord.size(), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (vertices->Position.size() * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenTextures(1, &texturedQuad->TextureID);
	glBindTexture(GL_TEXTURE_2D, texturedQuad->TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, (GLsizei)textureSize.x, (GLsizei)textureSize.y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);

	return texturedQuad;
}

void RendererGB::texturedQuadFree(TexturedQuad*& texturedQuad)
{
	delete texturedQuad;
}

void RendererGB::texturedQuadUpdateTexture(RenderContext* renderContext, TexturedQuad* texturedQuad, const uint8_t* const data, const std::size_t size)
{
	(void) renderContext;
	assert(texturedQuad->TextureSize.x * texturedQuad->TextureSize.y == size && "Data size does not match texture dimensions");

	glBindTexture(GL_TEXTURE_2D, texturedQuad->TextureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)texturedQuad->TextureSize.x, (GLsizei)texturedQuad->TextureSize.y, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
}

void RendererGB::texturedQuadDraw(RenderContext* renderContext, TexturedQuad* texturedQuad)
{
	glBindVertexArray(texturedQuad->Vao);
	glBindTexture(GL_TEXTURE_2D, texturedQuad->TextureID);

	renderContext->MainShaders.useProgram();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

namespace
{

#ifdef ENABLE_GL_DEBUG_CALLBACK
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
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

	SDL_LogError(0, "%d: %s of %s severity, raised from %s: %s", id, _type.c_str(), _severity.c_str(), _source.c_str(), message);
}
#endif
} // namespace
