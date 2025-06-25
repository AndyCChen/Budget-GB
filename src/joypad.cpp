#include "joypad.h"
#include "fmt/base.h"

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

void Joypad::processEvent(SDL_Event *event)
{
	if (event->type == SDL_EVENT_KEY_DOWN)
	{
		switch (event->key.scancode)
		{
		case SDL_SCANCODE_W:
			m_joypad.up = 0;
			break;

		case SDL_SCANCODE_A:
			m_joypad.left = 0;
			break;

		case SDL_SCANCODE_S:
			m_joypad.down = 0;
			break;

		case SDL_SCANCODE_D:
			m_joypad.right = 0;
			break;

		case SDL_SCANCODE_L:
			m_joypad.a = 0;
			break;

		case SDL_SCANCODE_K:
			m_joypad.b = 0;
			break;

		case SDL_SCANCODE_Q:
			m_joypad.start = 0;
			break;

		case SDL_SCANCODE_E:
			m_joypad.select = 0;
			break;

		default:
			break;
		}
	}
	else if (event->type == SDL_EVENT_KEY_UP)
	{
		switch (event->key.scancode)
		{
		case SDL_SCANCODE_W:
			m_joypad.up = 1;
			break;

		case SDL_SCANCODE_A:
			m_joypad.left = 1;
			break;

		case SDL_SCANCODE_S:
			m_joypad.down = 1;
			break;

		case SDL_SCANCODE_D:
			m_joypad.right = 1;
			break;

		case SDL_SCANCODE_L:
			m_joypad.a = 1;
			break;

		case SDL_SCANCODE_K:
			m_joypad.b = 1;
			break;

		case SDL_SCANCODE_Q:
			m_joypad.start = 1;
			break;

		case SDL_SCANCODE_E:
			m_joypad.select = 1;
			break;

		default:
			break;
		}
	}
}