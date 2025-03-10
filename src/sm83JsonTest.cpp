#include "sm83JsonTest.h"

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>

bool Sm83JsonTest::runJsonTest(BudgetGB& gameboy, const std::string& path)
{
	std::cout << "Testing... " << path;
	std::ifstream json_file(path);

	if (!json_file.is_open())
	{
		std::cerr << "\tError, cannot open json file!" << std::endl;
		return false;
	}

	nlohmann::json data = nlohmann::json::parse(json_file);

	bool status = true;
	for (size_t i = 0; i < data.size(); ++i)
	{
		initState(gameboy, data[i]["initial"]);
		gameboy.m_cpu->runInstruction();
		if (!checkState(gameboy, data[i]["final"]))
		{
			logState(gameboy, data[i]);
			status = false;
			break;
		}
	}
	
	json_file.close();

	if (status)
		std::cout << "\tSuccess!" << std::endl;

	return status;
}

void Sm83JsonTest::initState(BudgetGB& gameboy, nlohmann::json& item)
{
	// initialize cpu registers
	gameboy.m_cpu->m_registerAF.accumulator = item["a"];
	gameboy.m_cpu->m_registerAF.flags.setFlagsU8(item["f"]);

	gameboy.m_cpu->m_programCounter         = item["pc"];
	gameboy.m_cpu->m_stackPointer           = item["sp"];

	gameboy.m_cpu->m_registerBC.hi          = item["b"];
	gameboy.m_cpu->m_registerBC.lo          = item["c"];

	gameboy.m_cpu->m_registerDE.hi          = item["d"];
	gameboy.m_cpu->m_registerDE.lo          = item["e"];

	gameboy.m_cpu->m_registerHL.hi          = item["h"];
	gameboy.m_cpu->m_registerHL.lo          = item["l"];

	// zero out ram before setting ram values
	gameboy.m_bus->clearWram();
	for (size_t i = 0; i < item["ram"].size(); ++i)
	{
		uint16_t address = static_cast<uint16_t>(item["ram"][i][0]);
		uint8_t data = static_cast<uint8_t>(item["ram"][i][1]);
		gameboy.m_bus->m_wram[address] = data;
	}
}

bool Sm83JsonTest::checkState(BudgetGB& gameboy, nlohmann::json& item)
{
	bool status = true;

#define SM83_CHECK(condition, msg)   \
{                                    \
	if (condition)                    \
	{                                 \
		std::cout << std::endl << msg ; \
		status = false;                \
	}                                 \
}

	SM83_CHECK(gameboy.m_cpu->m_registerAF.accumulator        != static_cast<uint8_t>(item["a"]), "Accumulator Mismatch")
	SM83_CHECK(gameboy.m_cpu->m_registerAF.flags.getFlagsU8() != static_cast<uint8_t>(item["f"]),      "Cpu flags mismatch!");
	
	SM83_CHECK(gameboy.m_cpu->m_programCounter != static_cast<uint16_t>(item["pc"]), "Program counter mismatch!");
	SM83_CHECK(gameboy.m_cpu->m_stackPointer   != static_cast<uint16_t>(item["sp"]), "Stack pointer mismatch!");

	SM83_CHECK(gameboy.m_cpu->m_registerBC.hi != static_cast<uint8_t>(item["b"]), "b flag mismatch!");
	SM83_CHECK(gameboy.m_cpu->m_registerBC.lo != static_cast<uint8_t>(item["c"]), "c flag mismatch!");

	SM83_CHECK(gameboy.m_cpu->m_registerDE.hi != static_cast<uint8_t>(item["d"]), "d flag mismatch!");
	SM83_CHECK(gameboy.m_cpu->m_registerDE.lo != static_cast<uint8_t>(item["e"]), "e flag mismatch!");

	SM83_CHECK(gameboy.m_cpu->m_registerHL.hi != static_cast<uint8_t>(item["h"]), "h flag mismatch!");
	SM83_CHECK(gameboy.m_cpu->m_registerHL.lo != static_cast<uint8_t>(item["l"]), "l flag mismatch!");

	for (size_t i = 0; i < item["ram"].size(); ++i)
	{
		uint16_t address = static_cast<uint16_t>(item["ram"][i][0]);
		uint8_t data = static_cast<uint8_t>(item["ram"][i][1]);
		SM83_CHECK(gameboy.m_bus->m_wram[address] != data, "Ram content mismatch!");
	}
	
#undef SM83_CHECK
	return status;
}

void Sm83JsonTest::logState(BudgetGB& gameboy, nlohmann::json& item)
{
	std::cout << "\nInitial state..." << std::endl;
	std::cout << std::setw(4) << item["initial"] << std::endl;

	std::cout << "\nExpected state..." << std::endl;
	std::cout << std::setw(4) << item["final"] << std::endl;

	std::vector<std::vector<uint16_t>> ram(item["final"]["ram"].size());
	for (size_t i = 0; i < ram.size(); ++i)
	{
		uint16_t address = static_cast<uint16_t>(item["final"]["ram"][i][0]);
		ram[i] = { address, gameboy.m_bus->m_wram[address]};
	}

	nlohmann::json output =
	{
		{"a", static_cast<uint8_t>(gameboy.m_cpu->m_registerAF.accumulator)},

		{"b", gameboy.m_cpu->m_registerBC.hi},
		{"c", gameboy.m_cpu->m_registerBC.lo},

		{"d", gameboy.m_cpu->m_registerDE.hi},
		{"e", gameboy.m_cpu->m_registerDE.lo},

		{"f", gameboy.m_cpu->m_registerAF.flags.getFlagsU8()},

		{"h", gameboy.m_cpu->m_registerHL.hi},
		{"l", gameboy.m_cpu->m_registerHL.lo},

		{"pc", gameboy.m_cpu->m_programCounter},
		{"ram", ram},
		{"sp", gameboy.m_cpu->m_stackPointer},
	};

	std::cout << "\nOutput state..." << std::endl;
	std::cout << std::setw(4) << output << std::endl;
}

bool Sm83JsonTest::runAllJsonTests(BudgetGB& gameboy)
{
	bool status = true;

	status &= Sm83JsonTest::runJsonTest(gameboy, "sm83/v1/00.json");
	status &= Sm83JsonTest::runJsonTest(gameboy, "sm83/v1/01.json");
	status &= Sm83JsonTest::runJsonTest(gameboy, "sm83/v1/02.json");
	status &= Sm83JsonTest::runJsonTest(gameboy, "sm83/v1/03.json");
	status &= Sm83JsonTest::runJsonTest(gameboy, "sm83/v1/04.json");

	return status;
}
