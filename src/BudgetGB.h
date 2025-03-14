#pragma once

#include <memory>

#include "sm83.h"
#include "bus.h"

class BudgetGB
{
	friend class Sm83JsonTest;

public:

	BudgetGB() 
	{
		m_bus = std::make_unique<Bus>();
		m_cpu = std::make_unique<Sm83>(m_bus.get());
	};

private:
	std::unique_ptr<Sm83> m_cpu;
	std::unique_ptr<Bus> m_bus;
};