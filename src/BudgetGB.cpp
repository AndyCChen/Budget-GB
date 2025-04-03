#include "BudgetGB.h"

BudgetGB::BudgetGB() : m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
}

BudgetGB::BudgetGB(const std::string &romPath)
	: m_cartridge(), m_bus(m_cartridge), m_disassembler(m_bus), m_cpu(m_bus, m_disassembler)
{
	m_cartridge.loadRomFromPath(romPath);
}

void BudgetGB::run()
{
	while (true)
	{
		m_cpu.runInstruction();
	}
}
