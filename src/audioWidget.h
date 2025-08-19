#pragma once

#include "apu.h"
#include "audioLogBuffer.h"

class AudioWidget
{
  public:
	static bool drawAudioWidget(Apu &apu, Apu::AudioChannelToggle &audioChannelToggle);
};