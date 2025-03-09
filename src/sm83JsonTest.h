#pragma once

#include <string>

#include "nlohmann/json.hpp"
#include "BudgetGB.h"

class Sm83JsonTest
{
public:
	static bool runJsonTest(BudgetGB& gameboy, const std::string& path);

private:
	/// <summary>
	/// Initialize cpu registers and ram contents base on the loaded json test.
	/// </summary>
	/// <param name="gameboy">Gamboy instance to initialize</param>
	/// <param name="item">Parsed json init state</param>
	static void initState(BudgetGB& gameboy, nlohmann::json_abi_v3_11_3::json& item);

	/// <summary>
	/// Check gameboy state against the expected json test state.
	/// </summary>
	/// <returns>True: states match, False: mismatching states</returns>
	static bool checkState(BudgetGB& gameboy, nlohmann::json_abi_v3_11_3::json& item);
};
