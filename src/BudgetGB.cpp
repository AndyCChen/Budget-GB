#include "BudgetGB.h"

BudgetGB::BudgetGB() : m_bus(&m_cartridge), m_cpu(&m_bus)
{

}

BudgetGB::BudgetGB(const std::string &romPath) : m_bus(&m_cartridge), m_cpu(&m_bus)
{
	m_cartridge.loadRomFromPath(romPath);
}
