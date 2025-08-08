#include "BoxFilter.h"
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

void BoxFilter::pushSample(float sample)
{
	m_runningSum += sample;

	uint32_t widthWithError = BOX_WIDTH + static_cast<uint32_t>(m_error);

	if (++m_sampleCountInBox == widthWithError)
	{
		m_samplesAvail     = std::min(m_samplesAvail + 1, (uint32_t)m_buffer.size());
		m_sampleCountInBox = 0;

		float average = static_cast<float>(m_runningSum) / widthWithError;

		m_buffer[m_head] = m_highPass(average);
		m_runningSum     = 0;

		m_head = (m_head + 1) % m_buffer.size();
		if (m_head == m_tail)
		{
			m_tail = (m_tail + 1) % m_buffer.size();
		}

		m_error -= static_cast<uint32_t>(m_error);
		m_error += std::fabsf(SAMPLES_PER_AVERAGE - BOX_WIDTH);
	}
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