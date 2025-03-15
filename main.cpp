#include "BudgetGB.h"
#include "sm83JsonTest.h"

int main()
{
	BudgetGB gameboy;
	Sm83JsonTest::runAllJsonTests(gameboy);
}
