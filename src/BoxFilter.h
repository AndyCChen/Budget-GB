#pragma once

#include <cstdint>
#include <vector>

class BoxFilter
{
  public:
	BoxFilter(uint32_t sampleRate);

	bool pushSample(float sample);

	// samples are read into buffer and clamped into a 32 bit float sample in the range (-1.0f - 1.0f)
	uint32_t readSamples(float *buffer, uint32_t size);

	uint32_t getSamplesAvail() const
	{
		return m_samplesAvail;
	}

	uint32_t getAudioFrameSize() const
	{
		return SAMPLE_RATE / 60;
	}

	void clear()
	{
		std::fill(m_buffer.begin(), m_buffer.end(), (float)0);
		m_runningSum       = 0;
		m_sampleCountInBox = 0;
		m_error            = 0;
		m_head             = 0,
		m_tail             = 0,
		m_samplesAvail     = 0;
	}

  private:
	const int      SAMPLE_RATE;
	const float    SAMPLES_PER_AVERAGE;
	const uint32_t BOX_WIDTH;

	std::vector<float> m_buffer;

	float    m_runningSum       = 0;
	uint32_t m_sampleCountInBox = 0;
	float    m_error            = 0;

	uint32_t m_head = 0, m_tail = 0, m_samplesAvail = 0;

	class HighPass
	{
	  private:
		static constexpr float adjust = 0.996f;

		float prev = 0.0f;
		float out  = 0.0f;

	  public:
		float operator()(float in)
		{
			float delta = in - prev;
			prev        = in;

			out = (out * adjust) + delta;
			return out;

			/*float out = in - capacitor;
			capacitor = in - (out * 0.996f);

			return out;*/
		}
	};

	HighPass m_highPass;
};
