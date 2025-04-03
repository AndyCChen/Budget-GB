#include "BudgetGB.h"
#include "bus.h"
#include "sm83.h"
#include "sm83JsonTest.h"
#include "cartridge.h"


int main()
{
	BudgetGB gameboy("testRoms/11-op a,(hl).gb");
	gameboy.run();
	/*Cartridge cartridge;
	Bus bus(&cartridge, Bus::BusMode::SM83_TEST);
	Sm83 cpu(&bus);
	Sm83JsonTest::runJsonTest(cpu, "sm83/v1/10.json");*/
}


