#include "bus.h"
#include "BudgetGB.h"
#include "fmt/base.h"
#include "sm83.h"
#include "emulatorConstants.h"

Bus::Bus(Cartridge &cartridge, Sm83 &cpu, BudgetGbConstants::LcdColorBuffer &lcdColorBuffer)
	: m_cartridge(cartridge), m_cpu(cpu), m_ppu(lcdColorBuffer, m_cpu.m_interrupts.m_interruptFlags)
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
	std::fill(m_hram.begin(), m_hram.end(), static_cast<uint8_t>(0));
}

void Bus::clearWram()
{
	std::fill(m_wram.begin(), m_wram.end(), static_cast<uint8_t>(0));
}

uint8_t Bus::busReadRaw(uint16_t position)
{
	if (position < CARTRIDGE_ROM_END)
	{
		if (m_cpu.m_bootRomDisable)
		{
			return m_cartridge.cartridgeRead(position);
		}
		else if (position < BOOT_ROM_END)
		{
			return m_cpu.m_bootrom.read(position);
		}
		else
		{
			return m_cartridge.cartridgeRead(position);
		}
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
	else if (position < OAM_END)
	{
		return m_ppu.readOam(position);
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
		return m_cpu.m_interrupts.m_interruptEnable;
	}
}

uint8_t Bus::cpuRead(uint16_t position)
{
	uint8_t out = 0;

	if (position < CARTRIDGE_ROM_END)
	{
		if (m_cpu.m_bootRomDisable)
		{
			out = m_cartridge.cartridgeRead(position);
		}
		else if (position < BOOT_ROM_END)
		{
			out = m_cpu.m_bootrom.read(position);
		}
		else
		{
			out = m_cartridge.cartridgeRead(position);
		}
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
	else if (position < OAM_END)
	{
		out = m_ppu.readOam(position);
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
	else if (position < OAM_END)
	{
		m_ppu.writeOam(position, data);
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

	if (m_ppu.m_oamDmaController.dmaInProgress)
		handleOamDMA();

	m_cpu.m_timer.tick(m_cpu.m_interrupts.m_interruptFlags);
	m_ppu.tick();
	m_ppu.tick();
	m_ppu.tick();
	m_ppu.tick();
}

void Bus::onUpdate()
{
	// number of ticks that should pass per 1/60 of a second
	constexpr unsigned int TICKS_PER_FRAME = static_cast<unsigned int>(BudgetGbConstants::CLOCK_RATE_T / 59.7275);

	while (m_tCyclePerFrame < TICKS_PER_FRAME)
		m_cpu.instructionStep();

	m_tCyclePerFrame -= TICKS_PER_FRAME;
}

void Bus::writeIO(uint16_t position, uint8_t data)
{
	switch (position)
	{
	case IORegisters::JOYPAD:
		m_cpu.m_joypad.writeJoypad(data);
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

	case IORegisters::OAM_DMA:
		m_ppu.oamStartWrite(data);
		break;

	case IORegisters::BGP:
		m_ppu.r_bgPaletteData = data;
		break;

	case IORegisters::OBP0:
		m_ppu.r_objPaletteData0 = data;
		break;

	case IORegisters::OBP1:
		m_ppu.r_objPaletteData1 = data;
		break;

	case IORegisters::BOOT_ROM_ENABLE:
		m_cpu.m_bootRomDisable = data;
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
		return m_cpu.m_joypad.readJoypad();

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

	case IORegisters::OAM_DMA:
		return m_ppu.oamStartRead();

	case IORegisters::BGP:
		return m_ppu.r_bgPaletteData;

	case IORegisters::OBP0:
		return m_ppu.r_objPaletteData0;

	case IORegisters::OBP1:
		return m_ppu.r_objPaletteData1;

	case IORegisters::BOOT_ROM_ENABLE:
		return m_cpu.m_bootRomDisable;

	case IORegisters::LCD_SCX:
		return m_ppu.r_scrollX;

	case IORegisters::LCD_SCY:
		return m_ppu.r_scrollY;

	case IORegisters::LCD_WX:
		return m_ppu.r_windowX;

	case IORegisters::LCD_WY:
		return m_ppu.r_windowY;

	case IORegisters::INTERRUPT_IF:
		return m_cpu.m_interrupts.m_interruptFlags;

	default:
		return 0;
	}
}

void Bus::handleOamDMA()
{
	if (m_ppu.m_oamDmaController.byteCounter < 160)
	{
		uint8_t data = busReadRaw((m_ppu.r_oamStart << 8) | m_ppu.m_oamDmaController.byteCounter);
		m_ppu.writeOamDMA(m_ppu.m_oamDmaController.byteCounter, data);
		++m_ppu.m_oamDmaController.byteCounter;
	}
	else
	{
		m_ppu.m_oamDmaController.dmaInProgress = false;
		m_ppu.m_oamDmaController.byteCounter   = 0;
	}
}
