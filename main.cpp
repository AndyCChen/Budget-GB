#include "BudgetGB.h"
#include "bus.h"
#include "cartridge.h"
#include "disassembler.h"
#include "sm83.h"
#include "sm83JsonTest.h"

int main()
{
	BudgetGB gameboy("testRoms/11-op a,(hl).gb");
	gameboy.run();
	/*Cartridge cartridge;*/
	/*Bus bus(cartridge, Bus::BusMode::SM83_TEST);*/
	/*Disassembler disassembler(bus);*/
	/*Sm83 cpu(bus, disassembler);*/
	/*Sm83JsonTest::runJsonTest(cpu, "sm83/v1/cb f0.json");*/
}
