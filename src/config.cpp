#include "config.h"
#include "fmt/core.h"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

static std::string rgbFloatToHex(std::array<float, 3> colors)
{
	uint8_t r = static_cast<uint8_t>(std::round(colors[0] * 255.0f));
	uint8_t g = static_cast<uint8_t>(std::round(colors[1] * 255.0f));
	uint8_t b = static_cast<uint8_t>(std::round(colors[2] * 255.0f));

	return fmt::format("#{:02X}{:02X}{:02X}", r, g, b);
}

static std::array<float, 3> HexToRgbFloat(std::string &&hex)
{
	hex.erase(std::remove_if(hex.begin(), hex.end(), [](unsigned char c) { return c == '#' || std::isspace(c); }), hex.end());

	unsigned int r = 0, g = 0, b = 0;
	int          w = std::sscanf(hex.c_str(), "%02X%02X%02X", &r, &g, &b);
	return std::array<float, 3>{r / 255.0f, g / 255.0f, b / 255.0f};
}

void BudgetGbConfig::Config::saveConfig()
{
	nlohmann::json config;

	config["bootrom"]["useBootrom"] = useBootrom;
	config["bootrom"]["path"]       = bootromPath;
	config["recent roms"]           = recentRoms;
	config["window size"]           = windowScale;
	config["fullscreen mode"]       = fullscreenMode;

	for (const auto &item : palettes)
	{
		config["palettes"].push_back({
			{"name", item.name},
			{"color0", rgbFloatToHex(item.color0)},
			{"color1", rgbFloatToHex(item.color1)},
			{"color2", rgbFloatToHex(item.color2)},
			{"color3", rgbFloatToHex(item.color3)},
		});
	}

	config["active palette"] = activePalette;

	std::ofstream configFile(CONFIG_FILE_NAME);
	if (configFile.is_open())
	{
		configFile << config.dump(3);
		configFile.close();
	}
}

void BudgetGbConfig::Config::loadConfig()
{
	std::ifstream configFile(CONFIG_FILE_NAME);
	if (configFile.is_open())
	{
		using namespace BudgetGbConfig;

		nlohmann::json config = nlohmann::json::parse(configFile);

		useBootrom  = config["bootrom"]["useBootrom"];
		bootromPath = config["bootrom"]["path"];

		for (std::size_t i = 0; i < config["recent roms"].size() && i < MAX_RECENT_ROMS; ++i)
			recentRoms.push_back(std::string(config["recent roms"][i]));

		switch (static_cast<WindowScale>(config["window size"]))
		{
		case WindowScale::WindowScale_1x1:
			windowScale = WindowScale::WindowScale_1x1;
			break;
		case WindowScale::WindowScale_1x2:
			windowScale = WindowScale::WindowScale_1x2;
			break;
		case WindowScale::WindowScale_1x3:
			windowScale = WindowScale::WindowScale_1x3;
			break;
		case WindowScale::WindowScale_1x4:
			windowScale = WindowScale::WindowScale_1x4;
			break;
		case WindowScale::WindowScale_1x5:
			windowScale = WindowScale::WindowScale_1x5;
			break;
		case WindowScale::WindowScale_1x6:
			windowScale = WindowScale::WindowScale_1x6;
			break;
		default:
			break;
		}

		switch (static_cast<FullscreenMode>(config["fullscreen mode"]))
		{
		case FullscreenMode::FIT:
			fullscreenMode = FullscreenMode::FIT;
			break;
		case FullscreenMode::STRETCHED:
			fullscreenMode = FullscreenMode::STRETCHED;
			break;
		default:
			break;
		}

		for (std::size_t i = 0; i < config["palettes"].size() && i < MAX_PALETTES; ++i)
		{
			auto &item = config["palettes"][i];

			std::array<float, 3> color0{};
			std::array<float, 3> color1{};
			std::array<float, 3> color2{};
			std::array<float, 3> color3{};

			// clang-format off
			palettes.emplace_back(
				Palette {
					std::string(item["name"]), 
					HexToRgbFloat(std::string(item["color0"])), 
					HexToRgbFloat(std::string(item["color1"])), 
					HexToRgbFloat(std::string(item["color2"])), 
					HexToRgbFloat(std::string(item["color3"]))
				}
			);
			// clang-format on
		}

		int selectedPalette = config["active palette"];
		if (selectedPalette >= 0 && selectedPalette < palettes.size())
		{
			activePalette = selectedPalette;
		}

		configFile.close();
	}
}