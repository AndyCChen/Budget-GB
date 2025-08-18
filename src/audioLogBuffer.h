#pragma once

#include "utils/vec.h"

#include <utility>

namespace AudioLogging
{

template <int size>
struct AudioScrollingBuffer
{
	int Offset = 0;

	std::array<Utils::Vec2<float>, size> Data{};

	void AddPoint(float amp)
	{
		Data[Offset] = Utils::Vec2<float>{(float)Offset, amp};
		Offset       = (Offset + 1) % Data.size();
	}
};

struct AudioLogBuffers
{
	AudioScrollingBuffer<800> Pulse1;
	AudioScrollingBuffer<800> Pulse2;
	AudioScrollingBuffer<800> Wave;
	AudioScrollingBuffer<800> Noise;
	AudioScrollingBuffer<800> All;
};

} // namespace AudioLogging
