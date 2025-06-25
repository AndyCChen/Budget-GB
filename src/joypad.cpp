#include "joypad.h"
#include "SDL3/SDL.h"

uint8_t Joypad::readJoypad() const
{
	uint8_t joypadByte = 0xC0 | (m_joypad.selectButtons << 5) | (m_joypad.selectDpad << 4);

	if (m_joypad.selectDpad == 0 && m_joypad.selectButtons == 0)
	{
		uint8_t dpad    = (m_joypad.down << 3) | (m_joypad.up << 2) | (m_joypad.left << 1) | (m_joypad.right);
		uint8_t buttons = (m_joypad.start << 3) | (m_joypad.select << 2) | (m_joypad.b << 1) | (m_joypad.a);
		joypadByte |= dpad & buttons;
	}
	else if (m_joypad.selectDpad == 0)
		joypadByte |= (m_joypad.down << 3) | (m_joypad.up << 2) | (m_joypad.left << 1) | (m_joypad.right);

	else if (m_joypad.selectButtons == 0)
		joypadByte |= (m_joypad.start << 3) | (m_joypad.select << 2) | (m_joypad.b << 1) | (m_joypad.a);

	else if (m_joypad.selectDpad && m_joypad.selectButtons)
		joypadByte |= 0xF;

	return joypadByte;
}

void Joypad::writeJoypad(uint8_t data)
{
	m_joypad.selectDpad    = (data >> 4) & 1;
	m_joypad.selectButtons = (data >> 5) & 1;
}

void Joypad::processInputs()
{
	
}