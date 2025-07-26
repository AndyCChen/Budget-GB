#include "patternTileView.h"
#include "fmt/core.h"
#include "imgui.h"

PatternTileView::PatternTileView(const PPU &ppu, RendererGB::RenderContext *renderContext)
	: m_ppu(ppu)
{
	Utils::Vec2<float> size{BudgetGbConstants::TILE_VIEW_WIDTH, BudgetGbConstants::TILE_VIEW_HEIGHT};

	RendererGB::TextureRenderTarget *tileViewRenderTarget = nullptr;
	RendererGB::textureRenderTargetCreate(renderContext, tileViewRenderTarget, size);

	RendererGB::TexturedQuad *tileViewQuad = nullptr;
	RendererGB::texturedQuadCreate(renderContext, tileViewQuad, size);

	m_tileViewRenderTarget.reset(tileViewRenderTarget);
	m_tileViewQuad.reset(tileViewQuad);
}

bool PatternTileView::drawViewportGui(RendererGB::RenderContext *renderContext)
{
	bool toggle = true;

	if (ImGui::Begin("Tile View", &toggle))
	{
		ImGui::BeginChild("Tile Viewport", ImVec2(ImGui::GetWindowSize().x - 250.0f, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
		{
			ImVec2 childSize = ImGui::GetWindowSize();
			// resize image texture if window size changes
			if (updateWindowSize(childSize.x, childSize.y))
				RendererGB::textureRenderTargetResize(renderContext, m_tileViewRenderTarget.get(), Utils::Vec2<float>{m_tileViewportSize.x, m_tileViewportSize.y});

			ImTextureID tileTextureID = RendererGB::textureRenderTargetGetTextureID(m_tileViewRenderTarget.get());

			ImGui::Image(tileTextureID, ImVec2(m_tileViewportSize.x, m_tileViewportSize.y));
			if (ImGui::IsItemHovered())
			{
				ImVec2 pos  = ImGui::GetCursorScreenPos();
				ImVec2 mPos = ImGui::GetMousePos();

				m_tileX = static_cast<uint8_t>((mPos.x - pos.x) / m_tileViewportSize.x * 16.0f);
				m_tileY = static_cast<uint8_t>(((mPos.y - pos.y) + 5) / m_tileViewportSize.y * 24.0f) * -1;
				m_tileY = 23 - m_tileY;
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("Tile Info");
		{
			ImGui::Text("Tile X: %d", m_tileX);
			ImGui::Text("Tile Y: %d", m_tileY);

			uint16_t address = 0x8000 | (m_tileY << 8) | (m_tileX << 4);
			ImGui::Text("Tile Address: $%04X - $%04X", address, address + 0xF);
		}
		ImGui::EndChild();
	}
	ImGui::End();

	updateTilePixelBuffer();

	RendererGB::textureRenderTargetSet(renderContext, m_tileViewRenderTarget.get(), m_tileViewportSize);

	RendererGB::texturedQuadUpdateTexture(renderContext, m_tileViewQuad.get(), m_tilePixelBuffer.data(), m_tilePixelBuffer.size());
	RendererGB::texturedQuadDraw(renderContext, m_tileViewQuad.get());

	return toggle;
}

bool PatternTileView::updateWindowSize(float width, float height)
{
	bool isResized = false;

	constexpr uint8_t            OFFSET = 16;
	constexpr Utils::Vec2<float> MIN_SIZE{64, 64};

	float diffX = width - m_tileViewportSize.x;
	float diffY = height - m_tileViewportSize.y;

	if (diffX > OFFSET)
	{
		isResized            = true;
		m_tileViewportSize.x = width;
	}
	else if (diffX < 0 && m_tileViewportSize.x >= MIN_SIZE.x)
	{
		isResized = true;
		m_tileViewportSize.x -= OFFSET;
	}

	if (diffY > OFFSET)
	{
		isResized            = true;
		m_tileViewportSize.y = height;
	}
	else if (diffY < 0 && m_tileViewportSize.y >= MIN_SIZE.y)
	{
		isResized = true;
		m_tileViewportSize.y -= OFFSET;
	}

	return isResized;
}

void PatternTileView::updateTilePixelBuffer()
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

				m_tilePixelBuffer[topLeft + ((7 - fineY) * 128) + fineX] = colorIndex;
				lo <<= 1;
				hi <<= 1;
			}
		}
	}
}
