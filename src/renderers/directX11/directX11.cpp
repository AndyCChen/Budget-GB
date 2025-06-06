#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
#include "renderer.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
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
	mwrl::ComPtr<ID3D11Buffer> m_vertexBuffer;
	mwrl::ComPtr<ID3D11Buffer> m_indexBuffer;

	mwrl::ComPtr<ID3D11VertexShader> m_vertexShader;
	mwrl::ComPtr<ID3D11PixelShader>  m_pixelShader;
	mwrl::ComPtr<ID3D11InputLayout>  m_inputLayout;

	mwrl::ComPtr<ID3D11Texture2D>          m_viewportTexture;
	mwrl::ComPtr<ID3D11ShaderResourceView> m_viewportShaderResourceView;
	mwrl::ComPtr<ID3D11SamplerState>       m_textureSampler;

	Utils::struct_Vec2<uint32_t> m_viewportSize;
	Utils::struct_Vec2<uint32_t> m_viewportXY;

	/**
	 * @brief Set up vertex buffers and shaders for rendering the main viewport
	 * @param device
	 * @param deviceContext
	 */
	void initMainViewport(const mwrl::ComPtr<ID3D11Device> &device, const mwrl::ComPtr<ID3D11DeviceContext> &deviceContext);
};

} // namespace

struct RendererGB::RenderContext
{
	mwrl::ComPtr<ID3D11Device>           m_device;
	mwrl::ComPtr<IDXGISwapChain>         m_swapChain;
	mwrl::ComPtr<ID3D11DeviceContext>    m_deviceContext;
	mwrl::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

	GbMainViewport m_mainViewport;

	RenderContext(HWND hwnd);
};

bool RendererGB::initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale)
{

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Budget Gameboy", BudgetGB::LCD_WIDTH * windowScale, BudgetGB::LCD_HEIGHT * windowScale, 0);
	SDL_SetWindowMinimumSize(window, BudgetGB::LCD_WIDTH, BudgetGB::LCD_HEIGHT);

	if (!window)
	{
		SDL_LogError(0, "Failed to create SDL window! SDL error: %s", SDL_GetError());
		return false;
	}

	HWND hwnd     = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	renderContext = new RenderContext(hwnd);

	{
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		GbMainViewport &mainViewport  = renderContext->m_mainViewport;
		mainViewport.m_viewportXY.x   = 0;
		mainViewport.m_viewportXY.y   = 0;
		mainViewport.m_viewportSize.x = width;
		mainViewport.m_viewportSize.y = height;
	}

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
	renderContext->m_renderTargetView.Reset();

	HRESULT result = renderContext->m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	CHECK_HR(result);

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = renderContext->m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	CHECK_HR(result);

	result = renderContext->m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderContext->m_renderTargetView.GetAddressOf());
	CHECK_HR(result);

	GbMainViewport &mainViewport  = renderContext->m_mainViewport;
	mainViewport.m_viewportXY.x   = x;
	mainViewport.m_viewportXY.y   = y;
	mainViewport.m_viewportSize.x = width;
	mainViewport.m_viewportSize.y = height;
}

void RendererGB::drawMainViewport(const std::vector<Utils::array_u8Vec4> &pixelBuffer, RenderContext *renderContext, SDL_Window *window)
{
	(void)window;
	GbMainViewport &mainViewport = renderContext->m_mainViewport;

	/*renderContext->m_deviceContext->UpdateSubresource(
	    mainViewport.m_viewportTexture.Get(),
	    0,
	    nullptr,
	    pixelBuffer.data(),
	    BudgetGB::LCD_WIDTH * sizeof(Utils::array_u8Vec4),
	    pixelBuffer.size() * sizeof(Utils::array_u8Vec4)
	);*/

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	renderContext->m_deviceContext->Map(mainViewport.m_viewportTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	std::memcpy(mappedResource.pData, pixelBuffer.data(), pixelBuffer.size() * sizeof(Utils::array_u8Vec4));
	renderContext->m_deviceContext->Unmap(mainViewport.m_viewportTexture.Get(), 0);

	constexpr float colors[] = {0 / 255.0f, 0 / 255.0f, 0 / 255.0f, 1.0f};
	renderContext->m_deviceContext->ClearRenderTargetView(renderContext->m_renderTargetView.Get(), colors);

	D3D11_VIEWPORT vp{};
	vp.Width    = (FLOAT)mainViewport.m_viewportSize.x;
	vp.Height   = (FLOAT)mainViewport.m_viewportSize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (FLOAT)mainViewport.m_viewportXY.x;
	vp.TopLeftY = (FLOAT)mainViewport.m_viewportXY.y;
	renderContext->m_deviceContext->RSSetViewports(1, &vp);

	UINT vertexOffset = 0;
	UINT vertexStride = 8 * sizeof(float);
	renderContext->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderContext->m_deviceContext->IASetInputLayout(mainViewport.m_inputLayout.Get());
	renderContext->m_deviceContext->IASetVertexBuffers(0, 1, mainViewport.m_vertexBuffer.GetAddressOf(), &vertexStride, &vertexOffset);
	renderContext->m_deviceContext->IASetIndexBuffer(mainViewport.m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	renderContext->m_deviceContext->VSSetShader(mainViewport.m_vertexShader.Get(), nullptr, 0);

	renderContext->m_deviceContext->PSSetShader(mainViewport.m_pixelShader.Get(), nullptr, 0);
	renderContext->m_deviceContext->PSSetShaderResources(0, 1, mainViewport.m_viewportShaderResourceView.GetAddressOf());
	renderContext->m_deviceContext->PSSetSamplers(0, 1, mainViewport.m_textureSampler.GetAddressOf());

	renderContext->m_deviceContext->OMSetRenderTargets(1, renderContext->m_renderTargetView.GetAddressOf(), nullptr);
	renderContext->m_deviceContext->DrawIndexed(6, 0, 0);
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

RendererGB::RenderContext::RenderContext(HWND hwnd)
{
	// setup swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	SDL_zero(swapChainDesc);
	swapChainDesc.BufferDesc.Width                   = 0;
	swapChainDesc.BufferDesc.Height                  = 0;
	swapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Numerator   = 0;
	swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count                   = 1;
	swapChainDesc.SampleDesc.Quality                 = 0;
	swapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount                        = 1;
	swapChainDesc.OutputWindow                       = hwnd;
	swapChainDesc.Windowed                           = TRUE;
	swapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

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
		&swapChainDesc, 
		&m_swapChain, 
		&m_device,
		nullptr, 
		&m_deviceContext
	);
	CHECK_HR(result);
	// clang-format on

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	CHECK_HR(result);

	result = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
	CHECK_HR(result);

	m_mainViewport.initMainViewport(m_device, m_deviceContext);
}

void GbMainViewport::initMainViewport(const mwrl::ComPtr<ID3D11Device> &device, const mwrl::ComPtr<ID3D11DeviceContext> &deviceContext)
{
	HRESULT result;

	// clang-format off
	float vertices[] = 
	{
		-1.0f, -1.0f, 0.0f,  8/255.0f,    24/255.0f,  32/255.0f,   0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,  224/255.0f,  248/255.0f, 208/255.0f,  0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f,  136/ 255.0f, 192/255.0f, 122/255.0f,  1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f,  52/255.0f,   104/255.0f, 86/255.0f,   1.0f, 0.0f,
	};

	unsigned int indices[] = 
	{
		0, 1, 2,
		2, 3, 0,
	};
	// clang-format on

	// create vertex buffer

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth           = sizeof(vertices);
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = 8 * sizeof(vertices[0]);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = vertices;

	result = device->CreateBuffer(&vertexBufferDesc, &subResourceData, &m_vertexBuffer);
	CHECK_HR(result);

	// create index buffer

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth           = sizeof(indices);
	indexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.StructureByteStride = 3 * sizeof(indices[0]);

	SDL_zero(subResourceData);
	subResourceData.pSysMem = indices;

	result = device->CreateBuffer(&indexBufferDesc, &subResourceData, &m_indexBuffer);
	CHECK_HR(result);

	// compile, create shaders

	mwrl::ComPtr<ID3DBlob> vertexBlob;
	mwrl::ComPtr<ID3DBlob> pixelBlob;
	mwrl::ComPtr<ID3DBlob> errorBlob;

	result = D3DCompileFromFile(
		L"resources/shaders/directX11/viewport.vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_main",
		"vs_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
		0,
		&vertexBlob,
		&errorBlob
	);

	if (FAILED(result))
	{
		if (errorBlob.Get())
		{
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		}
		CHECK_HR(result);
	}

	result = D3DCompileFromFile(
		L"resources/shaders/directX11/viewport.ps.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps_main",
		"ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
		0,
		&pixelBlob,
		&errorBlob
	);

	if (FAILED(result))
	{
		if (errorBlob.Get())
		{
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		}
		CHECK_HR(result);
	}

	result = device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, &m_vertexShader);
	CHECK_HR(result);

	result = device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, &m_pixelShader);
	CHECK_HR(result);

	// set input layout

	D3D11_INPUT_ELEMENT_DESC inputElemDesc[] = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TextureCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = device->CreateInputLayout(inputElemDesc, _countof(inputElemDesc), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &m_inputLayout);
	CHECK_HR(result);

	// set up texture

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width            = BudgetGB::LCD_WIDTH;
	textureDesc.Height           = BudgetGB::LCD_HEIGHT;
	textureDesc.MipLevels        = 1;
	textureDesc.ArraySize        = 1;
	textureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage            = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags        = 0;

	unsigned char colorPallete[][3] = {
		{8, 24, 32}, {52, 104, 86}, {136, 192, 112}, {224, 248, 208}, {255, 255, 255}
	};

	std::vector<Utils::array_u8Vec4> pixels(BudgetGB::LCD_WIDTH * BudgetGB::LCD_HEIGHT);

	SDL_zero(subResourceData);
	subResourceData.pSysMem     = pixels.data();
	subResourceData.SysMemPitch = BudgetGB::LCD_WIDTH * sizeof(Utils::array_u8Vec4);

	result = device->CreateTexture2D(&textureDesc, nullptr, &m_viewportTexture);
	CHECK_HR(result);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format                    = textureDesc.Format;
	shaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels       = 1;

	result = device->CreateShaderResourceView(m_viewportTexture.Get(), &shaderResViewDesc, &m_viewportShaderResourceView);
	CHECK_HR(result);

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter   = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	result = device->CreateSamplerState(&samplerDesc, &m_textureSampler);
	CHECK_HR(result);

	// set rasterizer state

	mwrl::ComPtr<ID3D11RasterizerState> rasterizerState;

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.FillMode              = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode              = D3D11_CULL_BACK;

	device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
	deviceContext->RSSetState(rasterizerState.Get());
}
