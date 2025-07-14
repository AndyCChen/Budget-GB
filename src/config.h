#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace BudgetGbConfig
{

static constexpr const char *CONFIG_FILE_NAME = "config.json";
static constexpr uint32_t    MAX_RECENT_ROMS  = 10;
static constexpr uint32_t    MAX_PALETTES     = 5;

static constexpr std::array<std::array<float, 3>, 4> DEFAULT_GB_PALETTE = {{
	{232.0f / 255.0f, 252.0f / 255.0f, 204.0f / 255.0f},
	{172.0f / 255.0f, 212.0f / 255.0f, 144.0f / 255.0f},
	{84.0f / 255.0f, 140.0f / 255.0f, 112.0f / 255.0f},
	{20.0f / 255.0f, 44.0f / 255.0f, 56.0f / 255.0f},
}};

enum class WindowScale
{
	WindowScale_1x1 = 1,
	WindowScale_1x2,
	WindowScale_1x3,
	WindowScale_1x4,
	WindowScale_1x5,
	WindowScale_1x6,
};

enum class FullscreenMode
{
	FIT = 0,
	STRETCHED,
};

struct Palette
{
	std::string          name;
	std::array<float, 3> color0;
	std::array<float, 3> color1;
	std::array<float, 3> color2;
	std::array<float, 3> color3;
};

struct Config
{
	Config()
	{
		loadConfig();

		palettes.push_back({"Budget Palette 1", {0.914f, 0.937f, 0.925f}, {0.627f, 0.627f, 0.545f}, {0.333f, 0.333f, 0.408f}, {0.129f, 0.118f, 0.125f}});
		palettes.push_back({"Budget Palette 1", {0.2f, 0.1f, 0.6f}, {0.2f, 0.5f, 0.6f}, {0.2f, 0.7f, 0.6f}, {0.2f, 0.5f, 0.6f}});
		palettes.push_back({"Budget Palette 3", {0.2f, 0.5f, 0.6f}, {0.2f, 0.5f, 0.6f}, {0.2f, 0.5f, 0.6f}, {0.2f, 0.5f, 0.6f}});
		palettes.push_back({"Budget Palette 4", {0.2f, 0.5f, 0.6f}, {0.2f, 0.9f, 0.6f}, {0.2f, 0.5f, 0.6f}, {0.2f, 0.5f, 0.6f}});
		palettes.push_back({"Budget Palette 5", {0.2f, 0.5f, 0.6f}, {0.2f, 0.5f, 0.6f}, {0.2f, 0.5f, 0.6f}, {0.5f, 0.5f, 0.6f}});

		defaultPalette.name   = "Default";
		defaultPalette.color0 = DEFAULT_GB_PALETTE[0];
		defaultPalette.color1 = DEFAULT_GB_PALETTE[1];
		defaultPalette.color2 = DEFAULT_GB_PALETTE[2];
		defaultPalette.color3 = DEFAULT_GB_PALETTE[3];
	}

	~Config()
	{
		saveConfig();
	}

	bool                     useBootrom = false;
	std::string              bootromPath;
	std::vector<std::string> recentRoms;
	WindowScale              windowScale    = WindowScale::WindowScale_1x4;
	FullscreenMode           fullscreenMode = FullscreenMode::STRETCHED;
	std::vector<Palette>     palettes;
	Palette                  defaultPalette;
	int                      activePalette = -1;

	void loadConfig();
	void saveConfig();
};

} // namespace BudgetGbConfig
