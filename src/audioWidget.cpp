#include "audioWidget.h"
#include "imgui.h"
#include "implot.h"

#include "fmt/core.h"

#include "utils/vec.h"
#include <cmath>
#include <vector>

namespace
{

constexpr ImVec4 Color4(const uint8_t r, const uint8_t g, const uint8_t b)
{
	return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

} // namespace

bool AudioWidget::drawAudioWidget(Apu &apu, Apu::AudioChannelToggle &audioChannelToggle)
{
	bool toggle = true;

	if (ImGui::Begin("Audio", &toggle))
	{
		// clang-format off
		if (ImGui::Checkbox("Pulse 1##Checkbox", &audioChannelToggle.Pulse1)){} ImGui::SameLine();
		if (ImGui::Checkbox("Pulse 2##Checkbox", &audioChannelToggle.Pulse2)){} ImGui::SameLine();
		if (ImGui::Checkbox("Wave##Checkbox", &audioChannelToggle.Wave)){} ImGui::SameLine();
		if (ImGui::Checkbox("Noise##Checkbox", &audioChannelToggle.Noise)){} ImGui::SameLine();
		ImGui::BeginDisabled(true);
		ImGui::Text("(?)");
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("F5 - F8 hotkeys");
		// clang-format on

		const AudioLogging::AudioLogBuffers &buffers = apu.getAudioLogBuffers();

		ImPlotFlags     plotFlags     = ImPlotFlags_NoLegend;
		ImPlotAxisFlags plotLineFlags = ImPlotAxisFlags_NoTickLabels;

		const ImVec2 plotSize = ImVec2(-1, 125);

		constexpr ImVec4 pulse1Color = Color4(0x23, 0x17, 0xA8);
		constexpr ImVec4 pulse2Color = Color4(0x5B, 0x10, 0x8D);
		constexpr ImVec4 waveColor   = Color4(0x10, 0x10, 0xFF);
		constexpr ImVec4 noiseColor  = Color4(0xFF, 0x60, 0xA8);
		constexpr ImVec4 allColor    = Color4(0xF6, 0x30, 0x30);

		if (ImPlot::BeginPlot("Pulse 1", plotSize, plotFlags))
		{
			ImPlot::SetupAxes(nullptr, nullptr, plotLineFlags, plotLineFlags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)buffers.Pulse1.Data.size(), ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
			ImPlot::PushStyleColor(ImPlotCol_Line, pulse1Color);
			ImPlot::PlotLine("Pulse 1", &buffers.Pulse1.Data[0].x, &buffers.Pulse1.Data[0].y, (int)buffers.Pulse1.Data.size(), ImPlotLineFlags_None, 0, sizeof(buffers.Pulse1.Data[0]));
			ImPlot::PopStyleColor(1);
			ImPlot::EndPlot();
		}

		if (ImPlot::BeginPlot("Pulse 2", plotSize, plotFlags))
		{
			ImPlot::SetupAxes(nullptr, nullptr, plotLineFlags, plotLineFlags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)buffers.Pulse2.Data.size(), ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
			ImPlot::PushStyleColor(ImPlotCol_Line, pulse2Color);
			ImPlot::PlotLine("Pulse 2", &buffers.Pulse2.Data[0].x, &buffers.Pulse2.Data[0].y, (int)buffers.Pulse2.Data.size(), ImPlotLineFlags_None, 0, sizeof(buffers.Pulse2.Data[0]));
			ImPlot::PopStyleColor(1);
			ImPlot::EndPlot();
		}

		if (ImPlot::BeginPlot("Wave", plotSize, plotFlags))
		{
			ImPlot::SetupAxes(nullptr, nullptr, plotLineFlags, plotLineFlags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)buffers.Wave.Data.size(), ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
			ImPlot::PushStyleColor(ImPlotCol_Line, waveColor);
			ImPlot::PlotLine("Wave", &buffers.Wave.Data[0].x, &buffers.Wave.Data[0].y, (int)buffers.Wave.Data.size(), ImPlotLineFlags_None, 0, sizeof(buffers.Wave.Data[0]));
			ImPlot::PopStyleColor(1);
			ImPlot::EndPlot();
		}

		if (ImPlot::BeginPlot("Noise", plotSize, plotFlags))
		{
			ImPlot::SetupAxes(nullptr, nullptr, plotLineFlags, plotLineFlags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)buffers.All.Data.size(), ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -2.0f, 2.0f);
			ImPlot::PushStyleColor(ImPlotCol_Line, noiseColor);
			ImPlot::PlotLine("Noise", &buffers.Noise.Data[0].x, &buffers.Noise.Data[0].y, (int)buffers.Noise.Data.size(), ImPlotLineFlags_None, 0, sizeof(buffers.Noise.Data[0]));
			ImPlot::PopStyleColor(1);
			ImPlot::EndPlot();
		}

		if (ImPlot::BeginPlot("All", plotSize, plotFlags))
		{
			ImPlot::SetupAxes(nullptr, nullptr, plotLineFlags, plotLineFlags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)buffers.All.Data.size(), ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, -4.0f, 4.0f);
			ImPlot::PushStyleColor(ImPlotCol_Line, allColor);
			ImPlot::PlotLine("All", &buffers.All.Data[0].x, &buffers.All.Data[0].y, (int)buffers.All.Data.size(), ImPlotLineFlags_None, 0, sizeof(buffers.All.Data[0]));
			ImPlot::PopStyleColor(1);
			ImPlot::EndPlot();
		}

		apu.setAudioChannelToggle(audioChannelToggle);
	}
	ImGui::End();

	return toggle;
}
