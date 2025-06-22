#include "bus.h"
#include "BudgetGB.h"
#include "fmt/base.h"
#include "sm83.h"

Bus::Bus(Cartridge &cartridge, Sm83 &cpu, std::vector<Utils::array_u8Vec4> &lcdPixelBuffer)
	: m_cartridge(cartridge), m_cpu(cpu), m_ppu(lcdPixelBuffer, m_cpu.m_interrupts.m_interruptFlags)
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
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
		return m_ppu.loggerReadVram(position);
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
		return readIO(position);
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
		out = m_ppu.readVram(position);
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
		m_ppu.writeVram(position, data);
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
	m_ppu.tick();
	m_ppu.tick();
	m_ppu.tick();
	m_ppu.tick();
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
	case IORegisters::JOYPAD:
		break;

	case IORegisters::SERIAL_SB:
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

	case IORegisters::LCD_CONTROL:
		m_ppu.r_lcdControl = data;
		break;

	case IORegisters::LCD_STAT:
		m_ppu.setLcdStatus(data);
		break;

	case IORegisters::LCD_LYC:
		m_ppu.r_LYC = data;
		break;

	case IORegisters::BGP:
		m_ppu.r_bgPaletteData = data;
		break;

	case IORegisters::LCD_SCX:
		m_ppu.r_scrollX = data;
		break;

	case IORegisters::LCD_SCY:
		m_ppu.r_scrollY = data;
		break;

	case IORegisters::LCD_WX:
		m_ppu.r_windowX = data;
		break;

	case IORegisters::LCD_WY:
		m_ppu.r_windowY = data;
		break;

	case IORegisters::INTERRUPT_IF:
		m_cpu.m_interrupts.m_interruptFlags = data;
		break;

	default:
		break;
	}
}

uint8_t Bus::readIO(uint16_t position)
{
	switch (position)
	{
	case IORegisters::JOYPAD:
		return 0x3F;

	case IORegisters::SERIAL_SB:
		return 0xFF;

	case IORegisters::TIMER_DIV:
		return m_cpu.m_timer.getDivider();

	case IORegisters::TIMER_TIMA:
		return m_cpu.m_timer.getTimerCounter();

	case IORegisters::TIMER_TMA:
		return m_cpu.m_timer.m_timerModulo;

	case IORegisters::TIMER_TAC:
		return m_cpu.m_timer.m_timerControl;

	case IORegisters::LCD_CONTROL:
		return m_ppu.r_lcdControl;

	case IORegisters::LCD_STAT:
		return m_ppu.getLcdStatus();

	case IORegisters::LCD_LY:
		return m_ppu.getLcdY();

	case IORegisters::LCD_LYC:
		return m_ppu.r_LYC;

	case IORegisters::BGP:
		return m_ppu.r_bgPaletteData;
		break;

	case IORegisters::LCD_SCX:
		return m_ppu.r_scrollX;
		break;

	case IORegisters::LCD_SCY:
		return m_ppu.r_scrollY;
		break;

	case IORegisters::LCD_WX:
		return m_ppu.r_windowX;
		break;

	case IORegisters::LCD_WY:
		return m_ppu.r_windowY;
		break;

	case IORegisters::INTERRUPT_IF:
		return m_cpu.m_interrupts.m_interruptFlags;

	default:
		return 0;
	}
}
