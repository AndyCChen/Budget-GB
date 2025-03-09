#include "sm83JsonTest.h"

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>

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

	bool status = true;
	for (size_t i = 0; i < 5; ++i)
	{
		initState(gameboy, data[i]["initial"]);
		gameboy.m_cpu->run_instruction();
		if (!checkState(gameboy, data[i]["final"]))
		{
			std::cout << "\nInitial state..." << std::endl;
			std::cout << std::setw(4) << data[i]["initial"] << std::endl;

			std::cout << "\nExpected state..." << std::endl;
			std::cout << std::setw(4) << data[i]["final"] << std::endl;

			std::vector<std::vector<uint16_t>> ram(data[i]["final"]["ram"].size());
			for (size_t idx = 0; idx < ram.size(); ++idx)
			{
				uint16_t address = static_cast<uint16_t>(data[i]["final"]["ram"][idx][0]);
				uint16_t val = static_cast<uint16_t>(data[i]["final"]["ram"][idx][1]);

				ram[idx] = { address, val };
			}

			nlohmann::json output = 
			{
				{"a", static_cast<unsigned int>(gameboy.m_cpu->m_registerAF.accumulator)},

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

			status = false;
			break;
		}
	}
	
	json_file.close();
	if (status)
		std::cout << "Success!" << std::endl;
	return status;
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

	// zero out ram before setting ram values
	gameboy.m_bus->clear_wram();
	for (size_t i = 0; i < item["ram"].size(); ++i)
	{
		uint16_t address = static_cast<uint16_t>(item["ram"][i][0]);
		uint8_t data = static_cast<uint8_t>(item["ram"][i][1]);
		gameboy.m_bus->m_wram[address] = data;
	}
}

bool Sm83JsonTest::checkState(BudgetGB& gameboy, nlohmann::json_abi_v3_11_3::json& item)
{
	bool status = true;

#define SM83_CHECK(condition, msg)   \
{                                    \
	if (condition)                    \
	{                                 \
		std::cout << msg << std::endl; \
		status = false;                \
	}                                 \
}

	SM83_CHECK(gameboy.m_cpu->m_registerAF.accumulator        != static_cast<unsigned int>(item["a"]), "Accumulator Mismatch")
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
