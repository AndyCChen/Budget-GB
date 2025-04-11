#pragma once

#include <string>

#include "nlohmann/json.hpp"
#include "sm83.h"

class Sm83JsonTest
{
  public:
	static bool runJsonTest(Sm83 &cpu, const std::string &path);
	static bool runAllJsonTests(Sm83 &cpu);

  private:
	static void initState(Sm83 &cpu, nlohmann::json &item);

	/**
	 * @brief check state of cpu against expected json output
	 * @param gameboy
	 * @param item
	 * @return
	 */
	static bool checkState(Sm83 &cpu, nlohmann::json &item);

	static void logState(Sm83 &cpu, nlohmann::json &item);
};
