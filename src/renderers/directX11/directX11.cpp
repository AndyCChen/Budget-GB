#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
#include "renderer.h"

#include <d3d11.h>

struct RendererGB::RenderContext
{
	ID3D11Device           *m_device           = nullptr;
	IDXGISwapChain         *m_swapChain        = nullptr;
	ID3D11DeviceContext    *m_deviceContext    = nullptr;
	ID3D11RenderTargetView *m_renderTargetView = nullptr;

	~RenderContext()
	{
		if (m_device != nullptr)
			m_device->Release();

		if (m_swapChain != nullptr)
			m_swapChain->Release();

		if (m_deviceContext != nullptr)
			m_deviceContext->Release();

		if (m_renderTargetView != nullptr)
			m_renderTargetView->Release();
	}
};

bool RendererGB::initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale)
{
	renderContext = new RenderContext();

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Budget Gameboy", BudgetGB::LCD_WIDTH * BudgetGB::INITIAL_WINDOW_SCALE,
	                          BudgetGB::LCD_HEIGHT * windowScale, 0);

	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		return false;
	}

	HWND hwnd =
		(HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

	// setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	SDL_zero(sd);
	sd.BufferDesc.Width                   = 0;
	sd.BufferDesc.Height                  = 0;
	sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.RefreshRate.Numerator   = 0;
	sd.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count                   = 1;
	sd.SampleDesc.Quality                 = 0;
	sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount                        = 1;
	sd.OutputWindow                       = hwnd;
	sd.Windowed                           = TRUE;
	sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT createDeviceFlags = 0;
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

	// clang-format off
	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr, 
		D3D_DRIVER_TYPE_HARDWARE, 
		nullptr, 
		createDeviceFlags, 
		nullptr, 
		0,
		D3D11_SDK_VERSION, 
		&sd, 
		&renderContext->m_swapChain, 
		&renderContext->m_device,
		nullptr, 
		&renderContext->m_deviceContext
	);
	// clang-format on

	if (result != S_OK)
	{
		SDL_LogError(0, "Failed to create D3D11 device and swap chain. Error code: 0x%08X", (unsigned int)result);
		return false;
	}

	ID3D11Resource *backBuffer = nullptr;
	renderContext->m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	renderContext->m_device->CreateRenderTargetView(backBuffer, nullptr, &renderContext->m_renderTargetView);
	backBuffer->Release();

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

	ImGui_ImplSDL3_InitForD3D(window);
	ImGui_ImplDX11_Init(renderContext->m_device, renderContext->m_deviceContext);

	return true;
}

void RendererGB::newFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}
void RendererGB::setMainViewportSize(RenderContext *renderContext, int x, int y, int width, int height)
{
}
void RendererGB::drawMainViewport(std::vector<Utils::array_u8Vec3> &pixelBuffer, RenderContext *renderContext)
{
}
void RendererGB::endFrame(SDL_Window *window, RenderContext *renderContext)
{
	(void)window;
	ImGui::Render();

	const float colors[] = {0.3f, 0.6f, 0.7f, 1.0f};
	renderContext->m_deviceContext->OMSetRenderTargets(1, &renderContext->m_renderTargetView, nullptr);
	renderContext->m_deviceContext->ClearRenderTargetView(renderContext->m_renderTargetView, colors);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	renderContext->m_swapChain->Present(1, 0);
}
void RendererGB::freeWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext)
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	delete renderContext;
	SDL_DestroyWindow(window);
	SDL_Quit();
}
