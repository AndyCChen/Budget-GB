#include "config.h"
#include "nlohmann/json.hpp"

#include <fstream>

void BudgetGbConfig::Config::saveConfig()
{
	nlohmann::json config;

	config["bootrom"]["useBootrom"] = useBootrom;
	config["bootrom"]["path"]       = bootromPath;
	config["recent roms"]           = recentRoms;
	config["window size"]           = windowScale;
	config["fullscreen mode"]       = fullscreenMode;

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

		configFile.close();
	}
}