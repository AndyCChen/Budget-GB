#pragma once

#include "SDL3/SDL.h"
#include <cstdint>

class Joypad
{
  private:
	struct JoypadButtons
	{
		uint8_t selectButtons : 1;
		uint8_t a : 1;
		uint8_t b : 1;
		uint8_t select : 1;
		uint8_t start : 1;

		uint8_t selectDpad : 1;
		uint8_t right : 1;
		uint8_t left : 1;
		uint8_t up : 1;
		uint8_t down : 1;
	} m_joypad;

  public:
	Joypad()
	{
		clear();
	}

	uint8_t readJoypad() const;
	void    writeJoypad(uint8_t data);
	void    processEvent(SDL_Event *event);

	void clear()
	{
		m_joypad.selectButtons = 1;
		m_joypad.a             = 1;
		m_joypad.b             = 1;
		m_joypad.select        = 1;
		m_joypad.start         = 1;

		m_joypad.selectDpad = 1;
		m_joypad.right      = 1;
		m_joypad.left       = 1;
		m_joypad.up         = 1;
		m_joypad.down       = 1;
	}
};