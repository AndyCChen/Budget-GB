#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
#include "patternTileView.h"
#include "renderer.h"
#include "utils/vec.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <memory>
#include <vector>
#include <wrl.h>

namespace mwrl = Microsoft::WRL;

#define CHECK_HR(hr)           \
	{                          \
		assert(SUCCEEDED(hr)); \
	}

struct Vertex
{
	float position[3];
	float textureCoord[2];
};

struct GbMainViewport
{
	mwrl::ComPtr<ID3D11Buffer> m_bufferVertex;

	mwrl::ComPtr<ID3D11Texture2D>          m_colorIndexTexture;
	mwrl::ComPtr<ID3D11ShaderResourceView> m_viewportShaderResourceView;
	// mwrl::ComPtr<ID3D11SamplerState>       m_textureSampler;

	Utils::Vec2<uint32_t> m_viewportSize;
	Utils::Vec2<uint32_t> m_viewportXY;

	/**
	 * @brief Set up vertex buffers and shaders for rendering the main viewport
	 * @param device
	 * @param deviceContext
	 */
	void initMainViewport(const mwrl::ComPtr<ID3D11Device> &device);
};

struct RendererGB::PatternTileViewport
{
	PatternTileViewport(const mwrl::ComPtr<ID3D11Device> &device);
	~PatternTileViewport();

	mwrl::ComPtr<ID3D11Buffer> m_bufferVertex;

	mwrl::ComPtr<ID3D11Texture2D>          m_colorIndexTexture;
	mwrl::ComPtr<ID3D11ShaderResourceView> m_colorIndexShaderResourceView;

	mwrl::ComPtr<ID3D11Texture2D>          m_renderTargetTexture;
	mwrl::ComPtr<ID3D11ShaderResourceView> m_renderTargetShaderResourceView;

	mwrl::ComPtr<ID3D11RenderTargetView> m_renderTargetView; // we render into this view (texture) to be displayed with imgui image
};

struct RendererGB::RenderContext
{
	RenderContext(HWND hwnd);

	mwrl::ComPtr<ID3D11Device>           m_device;
	mwrl::ComPtr<IDXGISwapChain>         m_swapChain;
	mwrl::ComPtr<ID3D11DeviceContext>    m_deviceContext;
	mwrl::ComPtr<ID3D11RenderTargetView> m_mainRenderTargetView; // main viewpport is rendered directly onto the main application window

	mwrl::ComPtr<ID3D11VertexShader> m_shaderVertex;
	mwrl::ComPtr<ID3D11PixelShader>  m_shaderPixel;
	mwrl::ComPtr<ID3D11InputLayout>  m_inputLayout;

	mwrl::ComPtr<ID3D11Buffer> m_bufferIndex;
	mwrl::ComPtr<ID3D11Buffer> m_bufferConstantPalettes;

	GbMainViewport m_mainViewport;
};

bool RendererGB::initWindowWithRenderer(SDL_Window *&window, RenderContext *&renderContext, const uint32_t windowScale)
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
	{
		SDL_LogError(0, "Failed to init SDL video! SDL error: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Budget Gameboy", BudgetGbConstants::LCD_WIDTH * windowScale, BudgetGbConstants::LCD_HEIGHT * windowScale, 0);
	SDL_SetWindowMinimumSize(window, BudgetGbConstants::LCD_WIDTH, BudgetGbConstants::LCD_HEIGHT);

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
	renderContext->m_mainRenderTargetView.Reset();

	HRESULT result = renderContext->m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	CHECK_HR(result);

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = renderContext->m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void **)backBuffer.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = renderContext->m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderContext->m_mainRenderTargetView.GetAddressOf());
	CHECK_HR(result);

	GbMainViewport &mainViewport  = renderContext->m_mainViewport;
	mainViewport.m_viewportXY.x   = x;
	mainViewport.m_viewportXY.y   = y;
	mainViewport.m_viewportSize.x = width;
	mainViewport.m_viewportSize.y = height;
}

void RendererGB::setViewportPalette(RenderContext *renderContext, const BudgetGbConfig::Palette &palette)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	renderContext->m_deviceContext->Map(renderContext->m_bufferConstantPalettes.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	const std::array<std::array<float, 4>, 4> newPalette{{
		{palette.color0[0], palette.color0[1], palette.color0[2], 1.0f},
		{palette.color1[0], palette.color1[1], palette.color1[2], 1.0f},
		{palette.color2[0], palette.color2[1], palette.color2[2], 1.0f},
		{palette.color3[0], palette.color3[1], palette.color3[2], 1.0f},
	}};

	std::memcpy(mappedResource.pData, newPalette[0].data(), sizeof(newPalette));
	renderContext->m_deviceContext->Unmap(renderContext->m_bufferConstantPalettes.Get(), 0);
}

void RendererGB::tileViewResize(RenderContext *renderContext, PatternTileViewport *patternTileViewport, const Utils::Vec2<float> &size)
{
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width            = (UINT) size.x;
	textureDesc.Height           = (UINT) size.y;
	textureDesc.MipLevels        = 1;
	textureDesc.ArraySize        = 1;
	textureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage            = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags   = 0;
	textureDesc.MiscFlags        = 0;

	HRESULT result = renderContext->m_device->CreateTexture2D(&textureDesc, nullptr, patternTileViewport->m_renderTargetTexture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format                    = textureDesc.Format;
	shaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels       = 1;

	result = renderContext->m_device->CreateShaderResourceView(patternTileViewport->m_renderTargetTexture.Get(), &shaderResViewDesc, patternTileViewport->m_renderTargetShaderResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format             = textureDesc.Format;
	renderTargetViewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = renderContext->m_device->CreateRenderTargetView(patternTileViewport->m_renderTargetTexture.Get(), &renderTargetViewDesc, patternTileViewport->m_renderTargetView.ReleaseAndGetAddressOf());
	CHECK_HR(result);
}

void RendererGB::tileViewDraw(RenderContext *renderContext, PatternTileViewport *patternTileViewport, const BudgetGbConstants::TileColorBuffer &tileColorBuffer, const Utils::Vec2<float> &size)
{
	// update tile color index texture

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HRESULT                  result = renderContext->m_deviceContext->Map(patternTileViewport->m_colorIndexTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CHECK_HR(result);

	if (BudgetGbConstants::TILE_VIEW_WIDTH == mappedResource.RowPitch)
		std::memcpy(mappedResource.pData, tileColorBuffer.data(), sizeof(BudgetGbConstants::TileColorBuffer));
	else
	{
		for (uint32_t row = 0; row < BudgetGbConstants::TILE_VIEW_HEIGHT; ++row)
		{
			for (uint32_t col = 0; col < BudgetGbConstants::TILE_VIEW_WIDTH; ++col)
				static_cast<uint8_t *>(mappedResource.pData)[row * mappedResource.RowPitch + col] = tileColorBuffer[row * BudgetGbConstants::TILE_VIEW_WIDTH + col];
		}
	}

	renderContext->m_deviceContext->Unmap(patternTileViewport->m_colorIndexTexture.Get(), 0);

	constexpr float clearColor[4]{};
	renderContext->m_deviceContext->ClearRenderTargetView(patternTileViewport->m_renderTargetView.Get(), clearColor);

	D3D11_VIEWPORT vp{};
	vp.Width    = size.x;
	vp.Height   = size.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	renderContext->m_deviceContext->RSSetViewports(1, &vp);

	UINT vertexOffset = 0;
	UINT vertexStride = sizeof(Vertex);
	renderContext->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderContext->m_deviceContext->IASetInputLayout(renderContext->m_inputLayout.Get());
	renderContext->m_deviceContext->IASetVertexBuffers(0, 1, patternTileViewport->m_bufferVertex.GetAddressOf(), &vertexStride, &vertexOffset);
	renderContext->m_deviceContext->IASetIndexBuffer(renderContext->m_bufferIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	renderContext->m_deviceContext->VSSetShader(renderContext->m_shaderVertex.Get(), nullptr, 0);
	renderContext->m_deviceContext->VSSetConstantBuffers(0, 1, renderContext->m_bufferConstantPalettes.GetAddressOf());

	renderContext->m_deviceContext->PSSetShader(renderContext->m_shaderPixel.Get(), nullptr, 0);
	renderContext->m_deviceContext->PSSetShaderResources(0, 1, patternTileViewport->m_colorIndexShaderResourceView.GetAddressOf());
	renderContext->m_deviceContext->PSSetConstantBuffers(0, 1, renderContext->m_bufferConstantPalettes.GetAddressOf());

	renderContext->m_deviceContext->OMSetRenderTargets(1, patternTileViewport->m_renderTargetView.GetAddressOf(), nullptr);
	renderContext->m_deviceContext->DrawIndexed(6, 0, 0);
}

ImTextureID RendererGB::tileViewGetTextureID(PatternTileViewport *patternTileViewport)
{
	return (ImTextureID)(intptr_t)patternTileViewport->m_renderTargetShaderResourceView.Get();
}

void RendererGB::drawMainViewport(const BudgetGbConstants::LcdColorBuffer &pixelBuffer, RenderContext *renderContext, SDL_Window *window)
{
	(void)window;
	GbMainViewport &mainViewport = renderContext->m_mainViewport;

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HRESULT                  result = renderContext->m_deviceContext->Map(mainViewport.m_colorIndexTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CHECK_HR(result);

	// a row of texture bytes is aligned to 256 bytes so we can't do a direct memcpy
	for (uint32_t row = 0; row < BudgetGbConstants::LCD_HEIGHT; ++row)
	{
		for (uint32_t col = 0; col < BudgetGbConstants::LCD_WIDTH; ++col)
			static_cast<uint8_t *>(mappedResource.pData)[row * mappedResource.RowPitch + col] = pixelBuffer[row * BudgetGbConstants::LCD_WIDTH + col];
	}

	renderContext->m_deviceContext->Unmap(mainViewport.m_colorIndexTexture.Get(), 0);

	constexpr float clearColor[4]{};
	renderContext->m_deviceContext->ClearRenderTargetView(renderContext->m_mainRenderTargetView.Get(), clearColor);

	D3D11_VIEWPORT vp{};
	vp.Width    = (FLOAT)mainViewport.m_viewportSize.x;
	vp.Height   = (FLOAT)mainViewport.m_viewportSize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (FLOAT)mainViewport.m_viewportXY.x;
	vp.TopLeftY = (FLOAT)mainViewport.m_viewportXY.y;
	renderContext->m_deviceContext->RSSetViewports(1, &vp);

	UINT vertexOffset = 0;
	UINT vertexStride = sizeof(Vertex);
	renderContext->m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderContext->m_deviceContext->IASetInputLayout(renderContext->m_inputLayout.Get());
	renderContext->m_deviceContext->IASetVertexBuffers(0, 1, mainViewport.m_bufferVertex.GetAddressOf(), &vertexStride, &vertexOffset);
	renderContext->m_deviceContext->IASetIndexBuffer(renderContext->m_bufferIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	renderContext->m_deviceContext->VSSetShader(renderContext->m_shaderVertex.Get(), nullptr, 0);
	renderContext->m_deviceContext->VSSetConstantBuffers(0, 1, renderContext->m_bufferConstantPalettes.GetAddressOf());

	renderContext->m_deviceContext->PSSetShader(renderContext->m_shaderPixel.Get(), nullptr, 0);
	renderContext->m_deviceContext->PSSetShaderResources(0, 1, mainViewport.m_viewportShaderResourceView.GetAddressOf());
	renderContext->m_deviceContext->PSSetConstantBuffers(0, 1, renderContext->m_bufferConstantPalettes.GetAddressOf());

	renderContext->m_deviceContext->OMSetRenderTargets(1, renderContext->m_mainRenderTargetView.GetAddressOf(), nullptr);
	renderContext->m_deviceContext->DrawIndexed(6, 0, 0);
}

void RendererGB::endFrame(SDL_Window *window, RenderContext *renderContext)
{
	(void)window;
	ImGui::Render();

	renderContext->m_deviceContext->OMSetRenderTargets(1, renderContext->m_mainRenderTargetView.GetAddressOf(), nullptr);
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

	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void **)backBuffer.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_mainRenderTargetView.ReleaseAndGetAddressOf());
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
		vertexBlob.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf()
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
		pixelBlob.ReleaseAndGetAddressOf(),
		errorBlob.ReleaseAndGetAddressOf()
	);

	if (FAILED(result))
	{
		if (errorBlob.Get())
		{
			OutputDebugStringA((LPCSTR)errorBlob->GetBufferPointer());
		}
		CHECK_HR(result);
	}

	result = m_device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, m_shaderVertex.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = m_device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, m_shaderPixel.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set input layout

	D3D11_INPUT_ELEMENT_DESC inputElemDesc[] = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TextureCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = m_device->CreateInputLayout(inputElemDesc, _countof(inputElemDesc), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), m_inputLayout.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// create index buffer
	//
	// clang-format off

	uint32_t indices[] = 
	{
		0, 1, 2,
		2, 3, 0,
	};
	// clang-format on

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth           = sizeof(indices);
	indexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.StructureByteStride = 3 * sizeof(indices[0]);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = indices;

	result = m_device->CreateBuffer(&indexBufferDesc, &subResourceData, m_bufferIndex.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// create constant buffer for global palette color

	std::array<float, (BudgetGbConfig::DEFAULT_GB_PALETTE[0].size() + 1) * 4> cbufferInit{};
	cbufferInit.fill(1.0f);

	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.ByteWidth           = sizeof(cbufferInit); // width must be in 16 byte aligned
	constantBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
	constantBufferDesc.StructureByteStride = 0;

	SDL_zero(subResourceData);
	subResourceData.pSysMem = cbufferInit.data();

	result = m_device->CreateBuffer(&constantBufferDesc, &subResourceData, m_bufferConstantPalettes.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set rasterizer state

	mwrl::ComPtr<ID3D11RasterizerState> rasterizerState;

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.FillMode              = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode              = D3D11_CULL_BACK;

	m_device->CreateRasterizerState(&rasterizerDesc, rasterizerState.ReleaseAndGetAddressOf());
	m_deviceContext->RSSetState(rasterizerState.Get());

	// set up viewport texture

	m_mainViewport.initMainViewport(m_device);
}

void GbMainViewport::initMainViewport(const mwrl::ComPtr<ID3D11Device> &device)
{
	HRESULT result;

	// create vertex buffer

	// clang-format off

	Vertex vertices[] =
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f,                         0.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f,                         BudgetGbConstants::LCD_HEIGHT}},
		{{ 1.0f,  1.0f, 0.0f}, {BudgetGbConstants::LCD_WIDTH, BudgetGbConstants::LCD_HEIGHT}},
		{{ 1.0f, -1.0f, 0.0f}, {BudgetGbConstants::LCD_WIDTH, 0.0f}},
	};

	// clang-format on

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth           = sizeof(vertices);
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = vertices;

	result = device->CreateBuffer(&vertexBufferDesc, &subResourceData, m_bufferVertex.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set up texture for holding pixel color indices for the lcd display

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width            = BudgetGbConstants::LCD_WIDTH;
	textureDesc.Height           = BudgetGbConstants::LCD_HEIGHT;
	textureDesc.MipLevels        = 1;
	textureDesc.ArraySize        = 1;
	textureDesc.Format           = DXGI_FORMAT_R8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage            = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags        = 0;

	std::vector<uint8_t> pixels(BudgetGbConstants::LCD_WIDTH * BudgetGbConstants::LCD_HEIGHT);
	std::fill(pixels.begin(), pixels.end(), static_cast<uint8_t>(0));

	SDL_zero(subResourceData);
	subResourceData.pSysMem     = pixels.data();
	subResourceData.SysMemPitch = BudgetGbConstants::LCD_WIDTH;

	result = device->CreateTexture2D(&textureDesc, &subResourceData, m_colorIndexTexture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format                    = textureDesc.Format;
	shaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels       = 1;

	result = device->CreateShaderResourceView(m_colorIndexTexture.Get(), &shaderResViewDesc, m_viewportShaderResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);
}

#include <cstdlib>
#include <ctime>

PatternTileView::PatternTileView(const PPU &ppu, const RendererGB::RenderContext *renderContext)
	: m_ppu(ppu)
{
	m_patternTileViewport = std::make_unique<RendererGB::PatternTileViewport>(renderContext->m_device);

	srand(time(0));

	for (uint32_t i = 0; i < m_tilePixelBuffer.size(); ++i)
	{
		m_tilePixelBuffer[i] = rand() & 0x3;
	}
}

PatternTileView ::~PatternTileView() = default;

RendererGB::PatternTileViewport::PatternTileViewport(const mwrl::ComPtr<ID3D11Device> &device)
{

	// set up vertex buffer

	// clang-format off
	Vertex vertices[] =
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f,                               0.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f,                               BudgetGbConstants::TILE_VIEW_HEIGHT}},
		{{ 1.0f,  1.0f, 0.0f}, {BudgetGbConstants::TILE_VIEW_WIDTH, BudgetGbConstants::TILE_VIEW_HEIGHT}},
		{{ 1.0f, -1.0f, 0.0f}, {BudgetGbConstants::TILE_VIEW_WIDTH, 0.0f}},
	};
	// clang-format on

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth           = sizeof(vertices);
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = vertices;

	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &subResourceData, m_bufferVertex.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set up texture for holding tile pixel color indices

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width            = BudgetGbConstants::TILE_VIEW_WIDTH;
	textureDesc.Height           = BudgetGbConstants::TILE_VIEW_HEIGHT;
	textureDesc.MipLevels        = 1;
	textureDesc.ArraySize        = 1;
	textureDesc.Format           = DXGI_FORMAT_R8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage            = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags        = 0;

	result = device->CreateTexture2D(&textureDesc, nullptr, m_colorIndexTexture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set up shader resource view for tile pixel color index texture

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format                    = textureDesc.Format;
	shaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels       = 1;

	result = device->CreateShaderResourceView(m_colorIndexTexture.Get(), &shaderResViewDesc, m_colorIndexShaderResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set up render target texture

	SDL_zero(textureDesc);
	textureDesc.Width            = BudgetGbConstants::TILE_VIEW_WIDTH;
	textureDesc.Height           = BudgetGbConstants::TILE_VIEW_HEIGHT;
	textureDesc.MipLevels        = 1;
	textureDesc.ArraySize        = 1;
	textureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage            = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags   = 0;
	textureDesc.MiscFlags        = 0;

	result = device->CreateTexture2D(&textureDesc, nullptr, m_renderTargetTexture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set up shader resource view for render target texture

	SDL_zero(shaderResViewDesc);
	shaderResViewDesc.Format                    = textureDesc.Format;
	shaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels       = 1;

	result = device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResViewDesc, m_renderTargetShaderResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// create the render target view

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
	renderTargetViewDesc.Format             = textureDesc.Format;
	renderTargetViewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	result = device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, m_renderTargetView.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	fmt::println("contruct");
}

RendererGB::PatternTileViewport::~PatternTileViewport()
{
	fmt::println("destruct");
}
