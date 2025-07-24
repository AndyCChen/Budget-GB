#include "patternTileView.h"
#include "fmt/core.h"
#include "imgui.h"

bool PatternTileView::drawViewportGui(RendererGB::RenderContext *renderContext)
{
	bool toggle = true;

	if (ImGui::Begin("Tile View", &toggle))
	{
		ImGui::BeginChild("Tile Viewport", ImVec2(ImGui::GetWindowSize().x * 0.75f, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, 0.0f));

			ImVec2 childSize = ImGui::GetWindowSize();

			// resize image texture if window size changes
			if (updateWindowSize(childSize.x, childSize.y))
				RendererGB::tileViewResize(renderContext, m_patternTileViewport.get(), m_tileViewportSize);

			ImTextureID my_tex_id = RendererGB::tileViewGetTextureID(m_patternTileViewport.get());
			ImVec2      uv_min    = ImVec2(0.0f, 0.0f); // Top-left
			ImVec2      uv_max    = ImVec2(1.0f, 1.0f);

			ImGui::ImageWithBg(my_tex_id, ImVec2(m_tileViewportSize.x, m_tileViewportSize.y), uv_min, uv_max, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

			ImGui::PopStyleVar(2);
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("Tile Info");
		{
			ImGui::Text("tile id");
		}
		ImGui::EndChild();
	}
	ImGui::End();

	updateTilePixelBuffer();
	RendererGB::tileViewDraw(renderContext, m_patternTileViewport.get(), m_tilePixelBuffer, Utils::Vec2<float>{m_tileViewportSize.x, m_tileViewportSize.y});

	return toggle;
}

bool PatternTileView::updateWindowSize(float width, float height)
{
	bool isResized = false;

	if (m_tileViewportSize.x != width || m_tileViewportSize.y != height)
		isResized = true;

	m_tileViewportSize.x = width;
	m_tileViewportSize.y = height;

	return isResized;
}

void PatternTileView::updateTilePixelBuffer()
{
	auto &vram = m_ppu.getVram();

	// T TTTT TTTT YYYP
	// | |||| |||| |||+- P: Bit plane selection
	// | |||| |||| +++-- Y: Fine y offset (row within a 8 pixel high tile)
	// +-++++-++++------ T: Tile index (16 bytes per tile)

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
