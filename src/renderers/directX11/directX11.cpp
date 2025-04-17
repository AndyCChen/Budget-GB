#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
#include "renderer.h"

#include <cassert>
#include <cstdlib>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>

namespace mwrl = Microsoft::WRL;

#define CHECK_HR(hr)           \
	{                          \
		assert(SUCCEEDED(hr)); \
	}

namespace
{

struct GbMainViewport
{
	int i = 0;
};

} // namespace

struct RendererGB::RenderContext
{
	mwrl::ComPtr<ID3D11Device>           m_device;
	mwrl::ComPtr<IDXGISwapChain>         m_swapChain;
	mwrl::ComPtr<ID3D11DeviceContext>    m_deviceContext;
	mwrl::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	mwrl::ComPtr<ID3D11Buffer>       m_vertexBuffer;
	mwrl::ComPtr<ID3D11VertexShader> m_vertexShader;
	mwrl::ComPtr<ID3D11PixelShader>  m_pixelShader;
	mwrl::ComPtr<ID3D11InputLayout>  m_inputLayout;
};

bool RendererGB::initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale)
{
	renderContext = new RenderContext();

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Budget Gameboy", BudgetGB::LCD_WIDTH * windowScale,
	                          BudgetGB::LCD_HEIGHT * windowScale, 0);

	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		return false;
	}

	HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

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
	CHECK_HR(result);
	// clang-format on

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = renderContext->m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	CHECK_HR(result);

	result = renderContext->m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderContext->m_renderTargetView);
	CHECK_HR(result);

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
	ImGui_ImplDX11_Init(renderContext->m_device.Get(), renderContext->m_deviceContext.Get());

	// SET UP VERTEX BUFFER

	// clang-format off
	float vertices[] = 
	{
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f,
	};
	// clang-format on

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth           = sizeof(vertices);
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = 3 * sizeof(vertices[0]);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = vertices;

	result = renderContext->m_device->CreateBuffer(&vertexBufferDesc, &subResourceData, &renderContext->m_vertexBuffer);
	CHECK_HR(result);

	mwrl::ComPtr<ID3DBlob> vertexBlob;
	mwrl::ComPtr<ID3DBlob> pixelBlob;
	mwrl::ComPtr<ID3DBlob> errorBlob;

	// clang-format off
	result = D3DCompileFromFile(
		L"resources/shaders/directX11/viewport.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_main",
		"vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
		0,
		&vertexBlob,
		&errorBlob
	);
	// clang-format on

	if (FAILED(result))
	{
		if (errorBlob.Get())
		{
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		}
		CHECK_HR(result);
	}

	// clang-format off
	result = D3DCompileFromFile(
		L"resources/shaders/directX11/viewport.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps_main",
		"ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
		0,
		&pixelBlob,
		&errorBlob
	);
	// clang-format on

	if (FAILED(result))
	{
		if (errorBlob.Get())
		{
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		}
		CHECK_HR(result);
	}

	result = renderContext->m_device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &renderContext->m_vertexShader);
	CHECK_HR(result);

	result = renderContext->m_device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &renderContext->m_pixelShader);
	CHECK_HR(result);

	// input layout
	D3D11_INPUT_ELEMENT_DESC inputElemDesc[] = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = renderContext->m_device->CreateInputLayout(inputElemDesc, 1, vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &renderContext->m_inputLayout);
	CHECK_HR(result);

	mwrl::ComPtr<ID3D11RasterizerState> rs;

	D3D11_RASTERIZER_DESC rsDesc{};
	rsDesc.FrontCounterClockwise = TRUE;
	rsDesc.DepthClipEnable       = TRUE;
	rsDesc.FillMode              = D3D11_FILL_SOLID;
	rsDesc.CullMode              = D3D11_CULL_BACK;

	renderContext->m_device->CreateRasterizerState(&rsDesc, &rs);
	renderContext->m_deviceContext->RSSetState(rs.Get());

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
	renderContext->m_deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	renderContext->m_renderTargetView->Release();

	HRESULT result = renderContext->m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	CHECK_HR(result);

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = renderContext->m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	CHECK_HR(result);

	result = renderContext->m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderContext->m_renderTargetView.GetAddressOf());
	CHECK_HR(result);
}

void RendererGB::drawMainViewport(std::vector<Utils::array_u8Vec3> &pixelBuffer, RenderContext *renderContext, SDL_Window *window)
{
	const float colors[] = {136 / 255.0f, 192 / 255.0f, 112 / 255.0f, 1.0f};
	renderContext->m_deviceContext->ClearRenderTargetView(renderContext->m_renderTargetView.Get(), colors);

	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	D3D11_VIEWPORT vp{};
	vp.Width    = (FLOAT)width;
	vp.Height   = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	renderContext->m_deviceContext->RSSetViewports(1, &vp);

	UINT vertexOffset = 0;
	UINT vertexStride = 3 * sizeof(float);
	renderContext->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderContext->m_deviceContext->IASetInputLayout(renderContext->m_inputLayout.Get());
	renderContext->m_deviceContext->IASetVertexBuffers(0, 1, renderContext->m_vertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);

	renderContext->m_deviceContext->VSSetShader(renderContext->m_vertexShader.Get(), nullptr, 0);
	renderContext->m_deviceContext->PSSetShader(renderContext->m_pixelShader.Get(), nullptr, 0);

	renderContext->m_deviceContext->OMSetRenderTargets(1, renderContext->m_renderTargetView.GetAddressOf(), nullptr);
	renderContext->m_deviceContext->Draw(3, 0);
}
void RendererGB::endFrame(SDL_Window *window, RenderContext *renderContext)
{
	(void)window;
	ImGui::Render();

	renderContext->m_deviceContext->OMSetRenderTargets(1, renderContext->m_renderTargetView.GetAddressOf(), nullptr);
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
