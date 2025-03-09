#include "sm83JsonTest.h"

#include <cstdint>
#include <iostream>
#include <fstream>

bool Sm83JsonTest::runJsonTest(BudgetGB& gameboy, const std::string& path)
{
	std::cout << "Testing... " << path << std::endl;
	std::ifstream json_file(path);

	if (!json_file.is_open())
	{
		std::cerr << "Error, cannot open json file!" << std::endl;
		return false;
	}

	nlohmann::json data = nlohmann::json::parse(json_file);

	for (size_t i = 0; i < data.size(); ++i)
	{
		initState(gameboy, data[i]["initial"]);
		gameboy.m_cpu->run_instruction();
	}
	
	json_file.close();
	return true;
}

void Sm83JsonTest::initState(BudgetGB& gameboy, nlohmann::json_abi_v3_11_3::json& item)
{
	// initialize cpu registers
	gameboy.m_cpu->m_registerAF.flags.setFlagsU8(item["f"]);
	gameboy.m_cpu->m_registerAF.accumulator = item["a"];
	gameboy.m_cpu->m_programCounter         = item["pc"];
	gameboy.m_cpu->m_stackPointer           = item["sp"];
	gameboy.m_cpu->m_registerBC.hi          = item["b"];
	gameboy.m_cpu->m_registerBC.lo          = item["c"];
	gameboy.m_cpu->m_registerDE.hi          = item["d"];
	gameboy.m_cpu->m_registerDE.lo          = item["e"];
	gameboy.m_cpu->m_registerHL.hi          = item["h"];
	gameboy.m_cpu->m_registerHL.lo          = item["l"];

	// intialize ram
	gameboy.m_bus->clear_wram();
	for (size_t i = 0; i < item["ram"].size(); ++i)
	{
		uint16_t address = item["ram"][i][0];
		uint8_t data = item["ram"][i][1];
		gameboy.m_bus->m_wram[address] = data;
	}
}

bool Sm83JsonTest::checkState(BudgetGB& gameboy, nlohmann::json_abi_v3_11_3::json& item)
{
	return false;
}
