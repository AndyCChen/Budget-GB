#pragma once

#include "apu.h"
#include "audioLogBuffer.h"

class AudioWidget
{
  public:
	static bool draw(Apu &apu, Apu::AudioChannelToggle &audioChannelToggle);
};