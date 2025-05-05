#include "bus.h"
#include "BudgetGB.h"
#include "fmt/base.h"
#include "sm83.h"

Bus::Bus(Cartridge &cartridge, Sm83 &cpu)
	: m_cartridge(cartridge), m_cpu(cpu)
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
	std::fill(m_vram.begin(), m_vram.end(), static_cast<uint8_t>(0));
	std::fill(m_hram.begin(), m_hram.end(), static_cast<uint8_t>(0));
}

void Bus::clearWram()
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
}

uint8_t Bus::cpuReadNoTick(uint16_t position)
{
	if (position < CARTRIDGE_ROM_END)
	{
		return m_cartridge.cartridgeRead(position);
	}
	else if (position < VRAM_END)
	{
		return m_vram[position & 0x1FFF];
	}
	else if (position < EXTERNAL_RAM_END)
	{
		return 0;
	}
	else if (position < ECHO_RAM_END)
	{
		return m_wram[(position & 0xDFFF) & 0x1FFF];
	}
	else if (position < UNUSABLE_END)
	{
		return 0;
	}
	else if (position < IO_REGISTERS_END)
	{
		return 0;
	}
	else if (position < HRAM_END)
	{
		return m_hram[position & 0x7F];
	}
	// interrupt enable register at 0xFFFF
	else
	{
		return 0;
	}
}

uint8_t Bus::cpuRead(uint16_t position)
{
	uint8_t out = 0;

	if (position < CARTRIDGE_ROM_END)
	{
		out = m_cartridge.cartridgeRead(position);
	}
	else if (position < VRAM_END)
	{
		out = m_vram[position & 0x1FFF];
	}
	else if (position < EXTERNAL_RAM_END)
	{
		out = 0;
	}
	else if (position < ECHO_RAM_END)
	{
		out = m_wram[(position & 0xDFFF) & 0x1FFF];
	}
	else if (position < UNUSABLE_END)
	{
		out = 0;
	}
	else if (position < IO_REGISTERS_END)
	{
		out = readIO(position);
	}
	else if (position < HRAM_END)
	{
		out = m_hram[position & 0x7F];
	}
	// interrupt enable register at 0xFFFF
	else
	{
		out = m_cpu.m_interrupts.m_interruptEnable;
	}

	tickM();
	return out;
}

void Bus::cpuWrite(uint16_t position, uint8_t data)
{
	if (position < CARTRIDGE_ROM_END)
	{
	}
	else if (position < VRAM_END)
	{
		m_vram[position & 0x1FFF] = data;
	}
	else if (position < EXTERNAL_RAM_END)
	{
	}
	else if (position < ECHO_RAM_END)
	{
		m_wram[(position & 0xDFFF) & 0x1FFF] = data;
	}
	else if (position < UNUSABLE_END)
	{
	}
	else if (position < IO_REGISTERS_END)
	{
		writeIO(position, data);
	}
	else if (position < HRAM_END)
	{
		m_hram[position & 0x7F] = data;
	}
	// interrupt enable register at 0xFFFF
	else
	{
		m_cpu.m_interrupts.m_interruptEnable = data;
	}
	tickM();
}

void Bus::tickM()
{
	// one m-cycle is 4 t-cycles

	m_tCyclePerFrame += 4;
	m_tCycles += 4;

	m_cpu.m_timer.tick(m_cpu.m_interrupts.m_interruptFlags);
}

void Bus::onUpdate()
{
	// number of ticks that should pass per 1/60 of a second
	constexpr unsigned int TICKS_PER_FRAME = BudgetGB::CLOCK_RATE_T / 60;

	while (m_tCyclePerFrame < TICKS_PER_FRAME)
		m_cpu.instructionStep();

	m_tCyclePerFrame -= TICKS_PER_FRAME;
}

void Bus::writeIO(uint16_t position, uint8_t data)
{
	switch (position)
	{
	case IORegisters::SERIAL_SB:
		fmt::print("{:c}", static_cast<char>(data));
		break;

	case IORegisters::INTERRUPT_IF:
		m_cpu.m_interrupts.m_interruptFlags = data;
		break;

	case IORegisters::TIMER_DIV:
		 m_cpu.m_timer.setDivider();
		 break;

	case IORegisters::TIMER_TIMA:
		m_cpu.m_timer.setTimerCounter(data);
		break;

	case IORegisters::TIMER_TMA:
		m_cpu.m_timer.m_timerModulo = data;
		break;

	case IORegisters::TIMER_TAC:
		m_cpu.m_timer.m_timerControl = data;
		break;

	default:
		break;
	}
}

uint8_t Bus::readIO(uint16_t position)
{
	switch (position)
	{
	case IORegisters::INTERRUPT_IF:
		return m_cpu.m_interrupts.m_interruptFlags;

	case IORegisters::TIMER_DIV:
		return m_cpu.m_timer.getDivider();

	case IORegisters::TIMER_TIMA:
		return m_cpu.m_timer.getTimerCounter();

	case IORegisters::TIMER_TMA:
		return m_cpu.m_timer.m_timerModulo;

	case IORegisters::TIMER_TAC:
		return m_cpu.m_timer.m_timerControl;

	default:
		return 0;
	}
}
