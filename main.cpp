#include <iostream>

#include "BudgetGB.h"
#include "sm83JsonTest.h"

int main()
{
	BudgetGB gameboy;

	//Sm83JsonTest::runAllJsonTests(gameboy);
	Sm83JsonTest::runJsonTest(gameboy, "sm83/v1/09.json");
}