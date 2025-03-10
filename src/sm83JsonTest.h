#pragma once

#include <string>

#include "nlohmann/json.hpp"
#include "BudgetGB.h"

class Sm83JsonTest
{
public:
	static bool runJsonTest(BudgetGB& gameboy, const std::string& path);
	static bool runAllJsonTests(BudgetGB& gameboy);

private:
	/// <summary>
	/// Initialize cpu registers and ram contents base on the loaded json test.
	/// </summary>
	static void initState(BudgetGB& gameboy, nlohmann::json& item);

	/// <summary>
	/// Check gameboy state against the expected json test state.
	/// </summary>
	/// <returns>True: states match, False: mismatching states</returns>
	static bool checkState(BudgetGB& gameboy, nlohmann::json& item);

	/// <summary>
	/// Log the initial, expected and output states of emulator after test.
	/// </summary>
	static void logState(BudgetGB& gameboy, nlohmann::json& item);
};
