#include "BoxFilter.h"
#include "SDL3/SDL_timer.h"
#include "audioLogBuffer.h"
#include "emulatorConstants.h"

#include <cmath>

BoxFilter::BoxFilter(uint32_t sampleRate)
	: SAMPLE_RATE(sampleRate),
	  SAMPLES_PER_AVERAGE(static_cast<float>(BudgetGbConstants::CLOCK_RATE_T) / static_cast<float>(SAMPLE_RATE) / 4.0f),
	  BOX_WIDTH(static_cast<uint32_t>(SAMPLES_PER_AVERAGE))
{
	const int SIZE = (SAMPLE_RATE / 60) * 8;
	m_buffer.resize(SIZE);
}

bool BoxFilter::pushSample(const Samples &samples, AudioLogging::AudioLogBuffers &buffers)
{
	bool status = false;

	m_runningSum.Pulse1 += samples.Pulse1;
	m_runningSum.Pulse2 += samples.Pulse2;
	m_runningSum.Wave += samples.Wave;
	m_runningSum.Noise += samples.Noise;

	uint32_t widthWithError = BOX_WIDTH + static_cast<uint32_t>(m_error);

	if (++m_sampleCountInBox == widthWithError)
	{
		m_samplesAvail     = std::min(m_samplesAvail + 1, (uint32_t)m_buffer.size());
		m_sampleCountInBox = 0;

		float pulse1Average = static_cast<float>(m_runningSum.Pulse1) / widthWithError;
		float pulse2Average = static_cast<float>(m_runningSum.Pulse2) / widthWithError;
		float waveAverage   = static_cast<float>(m_runningSum.Wave) / widthWithError;
		float noiseAverage  = static_cast<float>(m_runningSum.Noise) / widthWithError;

		m_buffer[m_head] = m_channelHighPasses.All(pulse1Average + pulse2Average + waveAverage + noiseAverage);

		buffers.All.AddPoint(m_buffer[m_head]);
		buffers.Pulse1.AddPoint(m_channelHighPasses.Pulse1(pulse1Average));
		buffers.Pulse2.AddPoint(m_channelHighPasses.Pulse2(pulse2Average));
		buffers.Wave.AddPoint(m_channelHighPasses.Wave(waveAverage));
		buffers.Noise.AddPoint(m_channelHighPasses.Noise(noiseAverage));

		m_runningSum = RunningChannelSums{};

		m_head = (m_head + 1) % m_buffer.size();
		if (m_head == m_tail)
		{
			m_tail = (m_tail + 1) % m_buffer.size();
		}

		m_error -= static_cast<uint32_t>(m_error);
		m_error += std::fabsf(SAMPLES_PER_AVERAGE - BOX_WIDTH);

		status = true;
	}

	return status;
}

uint32_t BoxFilter::readSamples(float *buffer, uint32_t size)
{
	uint32_t        count         = 0;
	constexpr float MASTER_VOLUME = 0.05f;

	while (count < size && m_samplesAvail > 0)
	{
		--m_samplesAvail;
		buffer[count++] = m_buffer[m_tail] * MASTER_VOLUME;

		if (m_tail != m_head)
			m_tail = (m_tail + 1) % m_buffer.size();
	}

	return count;
}