#include "sm83JsonTest.h"
#include "sm83.h"
#include "bus.h"
#include "BudgetGB.h"


int main()
{
	Bus bus(Bus::BusMode::SM83_TEST);
	Sm83 cpu(&bus);
	Sm83JsonTest::runAllJsonTests(cpu);
	
	
}


