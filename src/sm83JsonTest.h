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

	static void initState(BudgetGB& gameboy, nlohmann::json& item);

	/**
	 * @brief check state of cpu against expected json output
	 * @param gameboy 
	 * @param item 
	 * @return 
	 */
	static bool checkState(BudgetGB& gameboy, nlohmann::json& item);

	static void logState(BudgetGB& gameboy, nlohmann::json& item);
};
