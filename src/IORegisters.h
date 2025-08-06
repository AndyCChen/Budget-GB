#pragma once

namespace IORegisters
{
enum RegisterAdresses
{
	JOYPAD    = 0xFF00,
	SERIAL_SB = 0xFF01, // Serial transfer data

	TIMER_DIV  = 0xFF04, // divider register
	TIMER_TIMA = 0xFF05, // timer counter
	TIMER_TMA  = 0xFF06, // timer modulo (reload value for timer counter)
	TIMER_TAC  = 0xFF07, // timer control

	INTERRUPT_IF = 0xFF0F, // interrupt flag

	// APU registers

	NR10 = 0xFF10, // pulse 1 sweep
	NR11 = 0xFF11, // pulse 1 length timer & duty cycle
	NR12 = 0xFF12, // pulse 1 volume & envelope
	NR13 = 0xFF13, // pulse 1 period lo
	NR14 = 0xFF14, // pulse 1 period hi & control

	NR21 = 0xFF16, // pulse 2 length timer & duty cycle
	NR22 = 0xFF17, // pulse 2 volume & envelope
	NR23 = 0xFF18, // pulse 2 period lo
	NR24 = 0xFF19, // pulse 1 period hi & control

	NR50 = 0xFF24, // master volume control
	NR52 = 0xFF26, // audio master control

	// PPU registers
	LCD_CONTROL = 0xFF40,
	LCD_STAT    = 0xFF41,
	LCD_SCY     = 0xFF42,
	LCD_SCX     = 0xFF43,
	LCD_LY      = 0xFF44,
	LCD_LYC     = 0xFF45,
	OAM_DMA     = 0xFF46,
	BGP         = 0xFF47, // bg color palette
	OBP0        = 0xFF48, // obj color palette 0
	OBP1        = 0xFF49, // obj color palette 1
	LCD_WY      = 0xFF4A,
	LCD_WX      = 0xFF4B,

	BOOT_ROM_ENABLE = 0xFF50, // non zero unmaps boot rom
};
} // namespace IORegisters
