#include "patternTileView.h"
#include "fmt/core.h"
#include "imgui.h"

PatternTileView::PatternTileView(const PPU &ppu, RendererGB::RenderContext *renderContext)
	: m_ppu(ppu)
{
	using namespace BudgetGbConstants;
	using namespace Utils;

	RendererGB::TextureRenderTarget *tileViewRenderTarget = nullptr;
	RendererGB::textureRenderTargetCreate(renderContext, tileViewRenderTarget, Vec2<float>{TILE_VIEW_WIDTH, TILE_VIEW_HEIGHT});

	RendererGB::TexturedQuad *tileViewQuad = nullptr;
	RendererGB::texturedQuadCreate(renderContext, tileViewQuad, Vec2<float>{TILE_VIEW_WIDTH, TILE_VIEW_HEIGHT});

	m_tileViewRenderTarget.reset(tileViewRenderTarget);
	m_tileViewQuad.reset(tileViewQuad);

	RendererGB::TextureRenderTarget *tileRenderTarget = nullptr;
	RendererGB::textureRenderTargetCreate(renderContext, tileRenderTarget, Vec2<float>{TILE_PREVIEW_SIZE.x, TILE_PREVIEW_SIZE.y});

	RendererGB::TexturedQuad *tileQuad = nullptr;
	RendererGB::texturedQuadCreate(renderContext, tileQuad, Vec2<float>{TILE_WIDTH, TILE_HEIGHT});

	m_tilePreviewRenderTarget.reset(tileRenderTarget);
	m_tilePreviewQuad.reset(tileQuad);
}

bool PatternTileView::drawViewportGui(RendererGB::RenderContext *renderContext)
{
	bool toggle = true;
	updateTileViewPixelBuffer();

	if (ImGui::Begin("Tile View", &toggle))
	{
		float tileViewportWidth = ImGui::GetWindowSize().x - 225.0f;

		ImGui::BeginChild("Tile Viewport", ImVec2(tileViewportWidth < MIN_TILE_VIEW_SIZE.x ? MIN_TILE_VIEW_SIZE.x : tileViewportWidth, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
		{
			ImVec2 childSize = ImGui::GetWindowSize();
			// resize image texture if window size changes
			if (updateWindowSize(childSize.x, childSize.y))
				RendererGB::textureRenderTargetResize(renderContext, m_tileViewRenderTarget.get(), Utils::Vec2<float>{m_tileViewportSize.x, m_tileViewportSize.y});

			ImTextureID tileTextureID = RendererGB::textureRenderTargetGetTextureID(m_tileViewRenderTarget.get());
			ImGui::Image(tileTextureID, ImVec2(m_tileViewportSize.x, m_tileViewportSize.y), ImVec2(0,0), ImVec2(1,1));

			if (ImGui::IsItemHovered())
			{
				ImVec2 pos  = ImGui::GetCursorScreenPos();
				ImVec2 mPos = ImGui::GetMousePos();

				m_tileX = static_cast<uint8_t>((mPos.x - pos.x) / m_tileViewportSize.x * 16.0f);
				m_tileY = 23 - (static_cast<uint8_t>(((mPos.y - pos.y) + 5) / m_tileViewportSize.y * 24.0f) * -1);

				updateTilePreviewPixelBuffer();
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("Tile Info");
		{
			ImTextureID tilePreviewTextureID = RendererGB::textureRenderTargetGetTextureID(m_tilePreviewRenderTarget.get());
			ImGui::Image(tilePreviewTextureID, ImVec2(TILE_PREVIEW_SIZE.x, TILE_PREVIEW_SIZE.y), ImVec2(0, 0), ImVec2(1, 1));

			ImGui::Text("Tile X: %d", m_tileX);
			ImGui::Text("Tile Y: %d", m_tileY);

			uint16_t address = 0x8000 | (m_tileY << 8) | (m_tileX << 4);
			ImGui::Text("Address: $%04X - $%04X", address, address + 0xF);
		}
		ImGui::EndChild();
	}
	ImGui::End();

	// draw main tile view

	RendererGB::textureRenderTargetSet(renderContext, m_tileViewRenderTarget.get(), m_tileViewportSize);
	RendererGB::texturedQuadUpdateTexture(renderContext, m_tileViewQuad.get(), m_tileViewPixelBuffer.data(), m_tileViewPixelBuffer.size());
	RendererGB::texturedQuadDraw(renderContext, m_tileViewQuad.get());

	// draw tile preview to display hovered tile

	RendererGB::textureRenderTargetSet(renderContext, m_tilePreviewRenderTarget.get(), TILE_PREVIEW_SIZE);
	RendererGB::texturedQuadUpdateTexture(renderContext, m_tilePreviewQuad.get(), m_tilePreviewPixelBuffer.data(), m_tilePreviewPixelBuffer.size());
	RendererGB::texturedQuadDraw(renderContext, m_tilePreviewQuad.get());

	return toggle;
}

bool PatternTileView::updateWindowSize(float width, float height)
{
	bool isResized = false;

	constexpr uint8_t OFFSET = 16;

	float diffX = width - m_tileViewportSize.x;
	float diffY = height - m_tileViewportSize.y;

	if (diffX > OFFSET)
	{
		isResized            = true;
		m_tileViewportSize.x = width;
	}
	else if (diffX < 0 && m_tileViewportSize.x >= MIN_TILE_VIEW_SIZE.x)
	{
		isResized = true;
		m_tileViewportSize.x -= OFFSET;
	}
	else if (m_tileViewportSize.x == 0)
	{
		isResized            = true;
		m_tileViewportSize.x = MIN_TILE_VIEW_SIZE.x;
	}

	if (diffY > OFFSET)
	{
		isResized            = true;
		m_tileViewportSize.y = height;
	}
	else if (diffY < 0 && m_tileViewportSize.y >= MIN_TILE_VIEW_SIZE.y)
	{
		isResized = true;
		m_tileViewportSize.y -= OFFSET;
	}
	else if (m_tileViewportSize.y == 0)
	{
		isResized            = true;
		m_tileViewportSize.y = MIN_TILE_VIEW_SIZE.y;
	}

	if (isResized)
	{
		// fmt::println("DIFF: {} {}", diffX, diffY);
		// fmt::println("{} {}", m_tileViewportSize.x, m_tileViewportSize.y);
	}

	return isResized;
}

void PatternTileView::updateTileViewPixelBuffer()
{
	auto &vram = m_ppu.getVram();

	// T TTTT TTTT YYYP
	// | |||| |||| |||+- P: Bit plane selection
	// | |||| |||| +++-- Y: Fine y offset (row within a 8 pixel high tile)
	// +-++++-++++------ T: Tile index (16 bytes per tile)

	// update tile pixel buffer which will be sent to shader for render
	// Texture is flipped also.

	for (uint32_t tile = 0; tile < 384; ++tile)
	{
		uint8_t tileX = tile & 0xF;
		uint8_t tileY = 23 - (tile >> 4);

		uint32_t topLeft = (tileY * 1024) + (tileX * 8);

		uint32_t baseAddress = tile << 4;

		for (uint8_t fineY = 0; fineY < 8; ++fineY)
		{
			uint8_t lo = vram[baseAddress | (fineY << 1)];
			uint8_t hi = vram[baseAddress | (fineY << 1) | 1];

			for (uint8_t fineX = 0; fineX < 8; ++fineX)
			{
				uint8_t colorIndex = ((hi & 0x80) >> 6) | ((lo & 0x80) >> 7);

				m_tileViewPixelBuffer[topLeft + ((7 - fineY) * 128) + fineX] = colorIndex;
				lo <<= 1;
				hi <<= 1;
			}
		}
	}
}

void PatternTileView::updateTilePreviewPixelBuffer()
{
	uint32_t topLeft = ((23 - m_tileY) * 1024) + (m_tileX * 8);

	for (uint8_t fineY = 0; fineY < 8; ++fineY)
	{
		for (uint8_t fineX = 0; fineX < 8; ++fineX)
		{
			m_tilePreviewPixelBuffer[((7 - fineY) * 8) + fineX] = m_tileViewPixelBuffer[topLeft + ((7 - fineY) * 128) + fineX];
		}
	}
}
