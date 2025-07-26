#include "imgui_impl_dx11.h"
#include "imgui_impl_sdl3.h"

#include "BudgetGB.h"
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

struct RendererGB::TextureRenderTarget
{
	D3D11_TEXTURE2D_DESC          TextureDesc{};
	mwrl::ComPtr<ID3D11Texture2D> RenderTargetTexture;

	D3D11_SHADER_RESOURCE_VIEW_DESC        ShaderResViewDesc{};
	mwrl::ComPtr<ID3D11ShaderResourceView> RenderTargetShaderResourceView;

	D3D11_RENDER_TARGET_VIEW_DESC        RenderTargetViewDesc{};
	mwrl::ComPtr<ID3D11RenderTargetView> RenderTargetView;
};

struct RendererGB::TexturedQuad
{
	mwrl::ComPtr<ID3D11Buffer> BufferVertex;

	mwrl::ComPtr<ID3D11Texture2D>          Texture;
	mwrl::ComPtr<ID3D11ShaderResourceView> TextureResourceView;
};

struct MainViewport
{
	Utils::Vec2<uint32_t> ViewportSize;
	Utils::Vec2<uint32_t> ViewportTopLeft;
};

struct RendererGB::RenderContext
{
	RenderContext(HWND hwnd);

	mwrl::ComPtr<ID3D11Device>           Device;
	mwrl::ComPtr<IDXGISwapChain>         SwapChain;
	mwrl::ComPtr<ID3D11DeviceContext>    DeviceContext;
	mwrl::ComPtr<ID3D11RenderTargetView> MainRenderTargetView; // main viewpport is rendered directly onto the main application window

	mwrl::ComPtr<ID3D11VertexShader> ShaderVertex;
	mwrl::ComPtr<ID3D11PixelShader>  ShaderPixel;
	mwrl::ComPtr<ID3D11InputLayout>  InputLayout;

	mwrl::ComPtr<ID3D11Buffer> BufferIndex;
	mwrl::ComPtr<ID3D11Buffer> BufferConstantPalettes;

	MainViewport MainViewport;
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
		MainViewport &mainViewport     = renderContext->MainViewport;
		mainViewport.ViewportTopLeft.x = 0;
		mainViewport.ViewportTopLeft.y = 0;
		mainViewport.ViewportSize.x    = width;
		mainViewport.ViewportSize.y    = height;
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
	ImGui_ImplDX11_Init(renderContext->Device.Get(), renderContext->DeviceContext.Get());

	return true;
}

void RendererGB::newFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

void RendererGB::mainViewportResize(RenderContext *renderContext, int x, int y, int width, int height)
{
	renderContext->MainRenderTargetView.Reset();

	HRESULT result = renderContext->SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	CHECK_HR(result);

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = renderContext->SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void **)backBuffer.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = renderContext->Device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderContext->MainRenderTargetView.GetAddressOf());
	CHECK_HR(result);

	MainViewport &mainViewport     = renderContext->MainViewport;
	mainViewport.ViewportTopLeft.x = x;
	mainViewport.ViewportTopLeft.y = y;
	mainViewport.ViewportSize.x    = width;
	mainViewport.ViewportSize.y    = height;
}

void RendererGB::mainViewportSetRenderTarget(RenderContext *renderContext)
{
	D3D11_VIEWPORT vp{};
	vp.Width    = (FLOAT)renderContext->MainViewport.ViewportSize.x;
	vp.Height   = (FLOAT)renderContext->MainViewport.ViewportSize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (FLOAT)renderContext->MainViewport.ViewportTopLeft.x;
	vp.TopLeftY = (FLOAT)renderContext->MainViewport.ViewportTopLeft.y;

	constexpr float CLEAR_COLOR[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	renderContext->DeviceContext->RSSetViewports(1, &vp);
	renderContext->DeviceContext->ClearRenderTargetView(renderContext->MainRenderTargetView.Get(), CLEAR_COLOR);
	renderContext->DeviceContext->OMSetRenderTargets(1, renderContext->MainRenderTargetView.GetAddressOf(), nullptr);
}

void RendererGB::setGlobalPalette(RenderContext *renderContext, const BudgetGbConfig::Palette &palette)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	renderContext->DeviceContext->Map(renderContext->BufferConstantPalettes.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	const std::array<std::array<float, 4>, 4> newPalette{{
		{palette.color0[0], palette.color0[1], palette.color0[2], 1.0f},
		{palette.color1[0], palette.color1[1], palette.color1[2], 1.0f},
		{palette.color2[0], palette.color2[1], palette.color2[2], 1.0f},
		{palette.color3[0], palette.color3[1], palette.color3[2], 1.0f},
	}};

	std::memcpy(mappedResource.pData, newPalette[0].data(), sizeof(newPalette));
	renderContext->DeviceContext->Unmap(renderContext->BufferConstantPalettes.Get(), 0);
}

void RendererGB::endFrame(SDL_Window *window, RenderContext *renderContext)
{
	(void)window;
	ImGui::Render();

	renderContext->DeviceContext->OMSetRenderTargets(1, renderContext->MainRenderTargetView.GetAddressOf(), nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	renderContext->SwapChain->Present(1, 0);
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
		&SwapChain, 
		&Device,
		nullptr, 
		&DeviceContext
	);
	CHECK_HR(result);
	// clang-format on

	mwrl::ComPtr<ID3D11Resource> backBuffer;

	result = SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), (void **)backBuffer.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = Device->CreateRenderTargetView(backBuffer.Get(), nullptr, MainRenderTargetView.ReleaseAndGetAddressOf());
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

	result = Device->CreateVertexShader(vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), nullptr, ShaderVertex.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = Device->CreatePixelShader(pixelBlob->GetBufferPointer(), pixelBlob->GetBufferSize(), nullptr, ShaderPixel.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set input layout

	D3D11_INPUT_ELEMENT_DESC inputElemDesc[] = {
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TextureCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	result = Device->CreateInputLayout(inputElemDesc, _countof(inputElemDesc), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), InputLayout.ReleaseAndGetAddressOf());
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

	result = Device->CreateBuffer(&indexBufferDesc, &subResourceData, BufferIndex.ReleaseAndGetAddressOf());
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

	result = Device->CreateBuffer(&constantBufferDesc, &subResourceData, BufferConstantPalettes.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	// set rasterizer state

	mwrl::ComPtr<ID3D11RasterizerState> rasterizerState;

	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthClipEnable       = TRUE;
	rasterizerDesc.FillMode              = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode              = D3D11_CULL_BACK;

	Device->CreateRasterizerState(&rasterizerDesc, rasterizerState.ReleaseAndGetAddressOf());
	DeviceContext->RSSetState(rasterizerState.Get());
}

void RendererGB::textureRenderTargetCreate(RenderContext *renderContext, TextureRenderTarget *&renderTargetTexture, const Utils::Vec2<float> &size)
{
	fmt::println("construct render target");
	renderTargetTexture = new TextureRenderTarget();

	renderTargetTexture->TextureDesc.Width            = (UINT)size.x;
	renderTargetTexture->TextureDesc.Height           = (UINT)size.y;
	renderTargetTexture->TextureDesc.MipLevels        = 1;
	renderTargetTexture->TextureDesc.ArraySize        = 1;
	renderTargetTexture->TextureDesc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTexture->TextureDesc.SampleDesc.Count = 1;
	renderTargetTexture->TextureDesc.Usage            = D3D11_USAGE_DEFAULT;
	renderTargetTexture->TextureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	renderTargetTexture->TextureDesc.CPUAccessFlags   = 0;
	renderTargetTexture->TextureDesc.MiscFlags        = 0;

	HRESULT result = renderContext->Device->CreateTexture2D(&renderTargetTexture->TextureDesc, nullptr, renderTargetTexture->RenderTargetTexture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	renderTargetTexture->ShaderResViewDesc.Format                    = renderTargetTexture->TextureDesc.Format;
	renderTargetTexture->ShaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	renderTargetTexture->ShaderResViewDesc.Texture2D.MostDetailedMip = 0;
	renderTargetTexture->ShaderResViewDesc.Texture2D.MipLevels       = 1;

	result = renderContext->Device->CreateShaderResourceView(renderTargetTexture->RenderTargetTexture.Get(), &renderTargetTexture->ShaderResViewDesc, renderTargetTexture->RenderTargetShaderResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	renderTargetTexture->RenderTargetViewDesc.Format             = renderTargetTexture->TextureDesc.Format;
	renderTargetTexture->RenderTargetViewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetTexture->RenderTargetViewDesc.Texture2D.MipSlice = 0;

	result = renderContext->Device->CreateRenderTargetView(renderTargetTexture->RenderTargetTexture.Get(), &renderTargetTexture->RenderTargetViewDesc, renderTargetTexture->RenderTargetView.ReleaseAndGetAddressOf());
	CHECK_HR(result);
}

void RendererGB::textureRenderTargetFree(TextureRenderTarget *&renderTargetTexture)
{
	fmt::println("destruct render target");
	delete renderTargetTexture;
}

void RendererGB::textureRenderTargetSet(RenderContext *renderContext, TextureRenderTarget *renderTargetTexture, const Utils::Vec2<float> &viewport)
{
	D3D11_VIEWPORT vp{};
	vp.Width    = viewport.x;
	vp.Height   = viewport.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;

	constexpr float CLEAR_COLOR[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	renderContext->DeviceContext->RSSetViewports(1, &vp);
	renderContext->DeviceContext->ClearRenderTargetView(renderTargetTexture->RenderTargetView.Get(), CLEAR_COLOR);
	renderContext->DeviceContext->OMSetRenderTargets(1, renderTargetTexture->RenderTargetView.GetAddressOf(), nullptr);
}

void RendererGB::textureRenderTargetResize(RenderContext *renderContext, TextureRenderTarget *renderTargetTexture, const Utils::Vec2<float> &size)
{
	renderTargetTexture->TextureDesc.Width  = (UINT)size.x;
	renderTargetTexture->TextureDesc.Height = (UINT)size.y;

	HRESULT result = renderContext->Device->CreateTexture2D(&renderTargetTexture->TextureDesc, nullptr, renderTargetTexture->RenderTargetTexture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = renderContext->Device->CreateShaderResourceView(renderTargetTexture->RenderTargetTexture.Get(), &renderTargetTexture->ShaderResViewDesc, renderTargetTexture->RenderTargetShaderResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	result = renderContext->Device->CreateRenderTargetView(renderTargetTexture->RenderTargetTexture.Get(), &renderTargetTexture->RenderTargetViewDesc, renderTargetTexture->RenderTargetView.ReleaseAndGetAddressOf());
	CHECK_HR(result);
}

ImTextureID RendererGB::textureRenderTargetGetTextureID(TextureRenderTarget *renderTargetTexture)
{
	return (ImTextureID)(intptr_t)renderTargetTexture->RenderTargetShaderResourceView.Get();
}

void RendererGB::texturedQuadCreate(RenderContext *renderContext, TexturedQuad *&texturedQuad, const Utils::Vec2<float> &textureSize)
{
	fmt::println("construct quad");
	texturedQuad = new TexturedQuad();

	// clang-format off
	Vertex vertices[] =
	{
		{{-1.0f, -1.0f, 0.0f}, {0.0f,                  0.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f,                  (float) textureSize.y}},
		{{ 1.0f,  1.0f, 0.0f}, {(float) textureSize.x, (float) textureSize.y}},
		{{ 1.0f, -1.0f, 0.0f}, {(float) textureSize.x, 0.0f}},
	};
	// clang-format on

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth           = sizeof(vertices);
	vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA subResourceData{};
	subResourceData.pSysMem = vertices;

	HRESULT result = renderContext->Device->CreateBuffer(&vertexBufferDesc, &subResourceData, texturedQuad->BufferVertex.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width            = (UINT)textureSize.x;
	textureDesc.Height           = (UINT)textureSize.y;
	textureDesc.MipLevels        = 1;
	textureDesc.ArraySize        = 1;
	textureDesc.Format           = DXGI_FORMAT_R8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage            = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
	textureDesc.MiscFlags        = 0;

	result = renderContext->Device->CreateTexture2D(&textureDesc, nullptr, texturedQuad->Texture.ReleaseAndGetAddressOf());
	CHECK_HR(result);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResViewDesc{};
	shaderResViewDesc.Format                    = textureDesc.Format;
	shaderResViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResViewDesc.Texture2D.MipLevels       = 1;

	result = renderContext->Device->CreateShaderResourceView(texturedQuad->Texture.Get(), &shaderResViewDesc, texturedQuad->TextureResourceView.ReleaseAndGetAddressOf());
	CHECK_HR(result);
}

void RendererGB::texturedQuadFree(TexturedQuad *&texturedQuad)
{
	fmt::println("delete quad");
	delete texturedQuad;
}

void RendererGB::texturedQuadUpdateTexture(RenderContext *renderContext, TexturedQuad *texturedQuad, const uint8_t *const data, const std::size_t size)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	D3D11_TEXTURE2D_DESC     textureDesc{};

	texturedQuad->Texture->GetDesc(&textureDesc);

	HRESULT result = renderContext->DeviceContext->Map(texturedQuad->Texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CHECK_HR(result);

	// perform direct memcpy only if the texture has no extra padding for alignment, else copy row by row
	if (textureDesc.Width == mappedResource.RowPitch)
		std::memcpy(mappedResource.pData, data, size);
	else
	{
		for (uint32_t row = 0; row < textureDesc.Height; ++row)
		{
			for (uint32_t col = 0; col < textureDesc.Width; ++col)
				static_cast<uint8_t *>(mappedResource.pData)[row * mappedResource.RowPitch + col] = data[row * textureDesc.Width + col];
		}
	}

	renderContext->DeviceContext->Unmap(texturedQuad->Texture.Get(), 0);
}

void RendererGB::texturedQuadDraw(RenderContext *renderContext, TexturedQuad *texturedQuad)
{
	UINT vertexOffset = 0;
	UINT vertexStride = sizeof(Vertex);

	renderContext->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderContext->DeviceContext->IASetInputLayout(renderContext->InputLayout.Get());
	renderContext->DeviceContext->IASetVertexBuffers(0, 1, texturedQuad->BufferVertex.GetAddressOf(), &vertexStride, &vertexOffset);
	renderContext->DeviceContext->IASetIndexBuffer(renderContext->BufferIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	renderContext->DeviceContext->VSSetShader(renderContext->ShaderVertex.Get(), nullptr, 0);

	renderContext->DeviceContext->PSSetShader(renderContext->ShaderPixel.Get(), nullptr, 0);
	renderContext->DeviceContext->PSSetShaderResources(0, 1, texturedQuad->TextureResourceView.GetAddressOf());
	renderContext->DeviceContext->PSSetConstantBuffers(0, 1, renderContext->BufferConstantPalettes.GetAddressOf());

	renderContext->DeviceContext->DrawIndexed(6, 0, 0);
}
