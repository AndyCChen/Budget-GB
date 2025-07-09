#pragma once

#include <string>
#include <vector>

namespace BudgetGbConfig
{

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

static constexpr char    *CONFIG_FILE_NAME = "config.json";
static constexpr uint32_t MAX_RECENT_ROMS  = 10;

struct Config
{
	Config()
	{
		recentRoms.reserve(MAX_RECENT_ROMS);
	}

	bool                     useBootrom = false;
	std::string              bootromPath;
	std::vector<std::string> recentRoms;
	WindowScale              windowScale    = WindowScale::WindowScale_1x4;
	FullscreenMode           fullscreenMode = FullscreenMode::STRETCHED;
};

} // namespace BudgetGbConfig