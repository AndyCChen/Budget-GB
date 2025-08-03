#include "apu.h"
#include "IORegisters.h"
#include "fmt/core.h"

#include <algorithm>

namespace
{

void SDLCALL audioDeviceStreamCallback(void *userdata, SDL_AudioStream *audioStream, int additionalAmount, int totalAmount)
{
	(void)totalAmount;

	Apu::AudioCallbackData *callBackData = (Apu::AudioCallbackData *)userdata;
	totalAmount /= sizeof(float);

	// SDL_LockMutex(callBackData->AudioThreadCtx.Mutex);

	// if buffer is does not contain enough samples, block until main thread has pushed enough samples into buffer
	// if (callBackData->Buffer->getSamplesAvail() < totalAmount && !callBackData->StopAudioPlayback)
	{
		// SDL_WaitCondition(callBackData->AudioThreadCtx.CanConsume, callBackData->AudioThreadCtx.Mutex);
		// int a = 0;
	}

	if (!callBackData->StopAudioPlayback)
	{
		SDL_LockMutex(callBackData->AudioThreadCtx.Mutex);

		while (totalAmount > 0)
		{
			std::array<float, 128> samples;

			const int total = std::min(totalAmount, (int)samples.size());

			const uint32_t samplesRead = callBackData->Buffer->readSamples(samples.data(), total);

			if (samplesRead == 0)
				break;

			SDL_PutAudioStreamData(audioStream, samples.data(), samplesRead * sizeof(samples[0]));
			totalAmount -= samplesRead;
		}

		SDL_UnlockMutex(callBackData->AudioThreadCtx.Mutex);

		/*if (callBackData->AudioBuffer.getSamplesAvail() < 800 * 4 && SDL_TryLockMutex(callBackData->AudioThreadCtx.Mutex))
		{
			while (callBackData->AudioBuffer.getSamplesAvail() < 800 * 4)
			{
				std::array<float, 128> samples;

				const uint32_t samplesRead = callBackData->Buffer->readSamples(samples.data(), samples.size());

				if (samplesRead == 0)
					break;

				for (uint32_t i = 0; i < samplesRead; ++i)
				{
					callBackData->AudioBuffer.pushSample(samples[i]);
				}
			}
			SDL_UnlockMutex(callBackData->AudioThreadCtx.Mutex);
		}*/

		/*if (callBackData->AudioBuffer.getSamplesAvail() < totalAmount)
		{
			int ab = 0;
			SDL_LockMutex(callBackData->AudioThreadCtx.Mutex2);

			while (callBackData->AudioBuffer.getSamplesAvail() < totalAmount)
			{
				std::array<float, 128> samples;

				const uint32_t samplesRead = callBackData->Buffer->readSamples(samples.data(), samples.size());

				if (samplesRead == 0)
					break;

				for (uint32_t i = 0; i < samplesRead; ++i)
				{
					callBackData->AudioBuffer.pushSample(samples[i]);
				}
			}

			SDL_UnlockMutex(callBackData->AudioThreadCtx.Mutex2);
		}*/

		/*while (totalAmount > 0)
		{
			std::array<float, 128> samples;

			const int total = std::min(totalAmount, (int)samples.size());

			const uint32_t samplesRead = callBackData->AudioBuffer.readSamples(samples.data(), total);

			if (samplesRead == 0)
				break;

			SDL_PutAudioStreamData(audioStream, samples.data(), samplesRead * sizeof(samples[0]));
			totalAmount -= samplesRead;
		}*/
	}

	// SDL_SignalCondition(callBackData->AudioThreadCtx.CanProduce);
	// SDL_UnlockMutex(callBackData->AudioThreadCtx.Mutex);

	/*static int currentSineSample = 0;
	additionalAmount /= sizeof(float);
	while (additionalAmount > 0)
	{
	    float     samples[128];
	    const int total = std::min(additionalAmount, 128);

	    for (int i = 0; i < total; ++i)
	    {
	        const int   freq  = 240;
	        const float phase = currentSineSample * freq / 8000.0f;
	        samples[i]        = SDL_sinf(phase * 2 * SDL_PI_F);
	        currentSineSample += 1;
	    }
	    currentSineSample %= 8000;
	    SDL_PutAudioStreamData(audioStream, samples, total * sizeof(float));
	    additionalAmount -= total;
	}*/
}
} // namespace

Apu::Apu(uint32_t sampleRate)
	: m_boxFilter(sampleRate)
{
	SDL_AudioSpec audioSpec{};
	audioSpec.format   = SDL_AUDIO_F32;
	audioSpec.freq     = sampleRate;
	audioSpec.channels = 1;

	if (!(m_audioCallbackData.AudioThreadCtx.Mutex = SDL_CreateMutex()))
		fmt::println("{}", SDL_GetError());

	if (!(m_audioCallbackData.AudioThreadCtx.Mutex2 = SDL_CreateMutex()))
		fmt::println("{}", SDL_GetError());

	if (!(m_audioCallbackData.AudioThreadCtx.CanConsume = SDL_CreateCondition()))
		fmt::println("{}", SDL_GetError());

	if (!(m_audioCallbackData.AudioThreadCtx.CanProduce = SDL_CreateCondition()))
		fmt::println("{}", SDL_GetError());

	m_audioCallbackData.Buffer = &m_boxFilter;
	m_audioStream              = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, audioDeviceStreamCallback, (void *)&m_audioCallbackData);

	if (!m_audioStream)
		fmt::println("{}", SDL_GetError());

	SDL_ResumeAudioStreamDevice(m_audioStream);
}

Apu::~Apu()
{
	fmt::println("destruct audio");
}

bool Apu::beginAudioFrame()
{
	SDL_LockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);
	if (m_boxFilter.getSamplesAvail() > m_boxFilter.getAudioFrameSize() * 4)
	{
		endAudioFrame();
		return false;
	}
	else
		return true;
}

void Apu::endAudioFrame()
{
	SDL_SignalCondition(m_audioCallbackData.AudioThreadCtx.CanConsume);
	SDL_UnlockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);
}

void Apu::tick(uint8_t divider)
{
	// apu div is incremented on a falling edge for bit 4 for the divider
	if ((m_prevDivider & 0x10) && (divider & 0x10) == 0)
	{
		m_apuDivider += 1;
	}

	m_prevDivider = divider;

	switch (m_apuDivider)
	{
	// clock length timers
	case 2:
		break;

	// clock pulse1 frequency sweep
	case 4:
		break;

	// clock envelope sweep
	case 8:
		m_apuDivider = 0;
		break;

	default:
		break;
	}

	mixAudio();

	m_pulse1.clockPeriodDivider();
}

void Apu::writeIO(uint16_t position, uint8_t data)
{
	// register are read only if apu is turned off, except for NR52
	if (!m_audioControl.AudioOnOff && position != IORegisters::NR52)
	{
		return;
	}

	switch (position)
	{
	case IORegisters::NR10:
		m_pulse1.Registers.FrequencySweep.set(data);
		break;

	case IORegisters::NR11:
		m_pulse1.Registers.LengthAndDuty.set(data);
		break;

	case IORegisters::NR12:
		m_pulse1.Registers.VolumeAndEnvelope.set(data);
		break;

	case IORegisters::NR13:
		m_pulse1.Registers.PeriodLo.set(data);
		break;

	case IORegisters::NR14:
		m_pulse1.Registers.PeriodHiAndControl.set(data);
		if (m_pulse1.Registers.PeriodHiAndControl.Trigger)
		{
			m_audioControl.Pulse1Status = 1;
			m_pulse1.LengthPeriod       = m_pulse1.LengthPeriod == LENGTH_COUNTER_MAX ? m_pulse1.Registers.LengthAndDuty.InitialLength : m_pulse1.LengthPeriod;
			m_pulse1.PeriodDivider      = (m_pulse1.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | m_pulse1.Registers.PeriodLo.PeriodLo8Bits;
			m_pulse1.EnvelopePeriod     = 0;
			m_pulse1.Volume             = m_pulse1.Registers.VolumeAndEnvelope.InitialVolume;

			// todo handle sweep
		}
		break;

	case IORegisters::NR50:
		m_masterVolume.set(data);
		break;

	case IORegisters::NR52:
		m_audioControl.set(data);
		break;

	default:
		break;
	}
}

uint8_t Apu::readIO(uint16_t position)
{
	switch (position)
	{
	case IORegisters::NR10:
		return m_pulse1.Registers.FrequencySweep.get();

	case IORegisters::NR11:
		return m_pulse1.Registers.LengthAndDuty.get();

	case IORegisters::NR12:
		return m_pulse1.Registers.VolumeAndEnvelope.get();

	case IORegisters::NR13:
		return m_pulse1.Registers.PeriodLo.get();

	case IORegisters::NR14:
		return m_pulse1.Registers.PeriodHiAndControl.get();

	case IORegisters::NR50:
		return m_masterVolume.get();

	case IORegisters::NR52:
		return m_audioControl.get();

	default:
		return 0xFF;
	}
}

void Apu::pauseAudio()
{
	SDL_LockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);
	m_audioCallbackData.StopAudioPlayback = true;
	SDL_UnlockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);
	SDL_SignalCondition(m_audioCallbackData.AudioThreadCtx.CanConsume);
	SDL_PauseAudioStreamDevice(m_audioStream);
}

void Apu::resumeAudio()
{
	SDL_LockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);
	m_audioCallbackData.StopAudioPlayback = true;
	SDL_UnlockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);

	SDL_ResumeAudioStreamDevice(m_audioStream);
}

void Apu::mixAudio()
{
	uint8_t pulse1Sample = m_pulse1.outputSample();

	// stop pushing samples if buffer is full and let audio callback eat samples
	// if (m_boxFilter.isFull())
	{
		// SDL_WaitCondition(m_audioCallbackData.AudioThreadCtx.CanProduce, m_audioCallbackData.AudioThreadCtx.Mutex);
		// fmt::println("full");
	}

	m_boxFilter.pushSample(pulse1Sample);

	// signal to audio thread that it can continue consuming samples since buffer is full
	/*if (m_boxFilter.isFull())
	{
	    SDL_SignalCondition(m_audioCallbackData.AudioThreadCtx.CanConsume);
	}*/
}

uint8_t Apu::Pulse1::outputSample() const
{
	uint8_t sample = 0;

	if (isDacOn())
	{
		sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][DutyCycleIndex];
		sample *= Volume;
	}

	return sample;
}

void Apu::Pulse1::clockPeriodDivider()
{
	if (++PeriodDivider == 0x800)
	{
		// divider overflowed 0x7FF
		DutyCycleIndex = (DutyCycleIndex + 1) % 7;
		PeriodDivider  = (Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | Registers.PeriodLo.PeriodLo8Bits;
	}
}

Apu::BoxFilter::BoxFilter(uint32_t sampleRate)
	: SAMPLE_RATE(sampleRate),
	  SAMPLES_PER_AVERAGE(static_cast<float>(BudgetGbConstants::CLOCK_RATE_T) / static_cast<float>(SAMPLE_RATE) / 4.0f)
{
	m_boxWidth = static_cast<uint32_t>(SAMPLES_PER_AVERAGE);

	const int SIZE = (SAMPLE_RATE / 60) * 16;
	m_buffer.resize(SIZE);
}

Apu::BoxFilter::~BoxFilter()
{
	fmt::println("box filter destruct");
}

void Apu::BoxFilter::pushSample(uint8_t sample)
{
	m_runningSum += sample;

	if (++m_sampleCountInBox == m_boxWidth + static_cast<uint32_t>(m_error))
	{
		m_samplesAvail     = std::min(m_samplesAvail + 1, (uint32_t)m_buffer.size());
		m_sampleCountInBox = 0;

		float average    = static_cast<float>(m_runningSum) / (m_boxWidth + static_cast<uint32_t>(m_error));
		m_buffer[m_head] = average;
		m_runningSum     = 0;

		m_head = (m_head + 1) % m_buffer.size();
		if (m_head == m_tail)
		{
			m_tail = (m_tail + 1) % m_buffer.size();
		}

		m_error -= static_cast<uint32_t>(m_error);
		m_error += SDL_fabsf(SAMPLES_PER_AVERAGE - m_boxWidth);
	}
}

uint32_t Apu::BoxFilter::readSamples(float *buffer, uint32_t size)
{
	uint32_t count = 0;

	while (count < size && m_samplesAvail > 0)
	{
		--m_samplesAvail;
		buffer[count++] = (m_buffer[m_tail] - 7.5f) / 7.5f;

		if (m_tail != m_head)
			m_tail = (m_tail + 1) % m_buffer.size();
	}

	return count;
}

void Apu::AudioThreadSampleBuffer::pushSample(float sample)
{
	m_samplesAvail = std::min(m_samplesAvail + 1, (uint32_t)m_buffer.size());

	m_buffer[m_head] = sample;
	m_head           = (m_head + 1) % m_buffer.size();
	if (m_head == m_tail)
	{
		m_tail = (m_tail + 1) % m_buffer.size();
	}
}

uint32_t Apu::AudioThreadSampleBuffer::readSamples(float *buffer, uint32_t size)
{
	uint32_t count = 0;

	while (count < size && m_samplesAvail > 0)
	{
		--m_samplesAvail;
		buffer[count++] = m_buffer[m_tail];

		if (m_tail != m_head)
			m_tail = (m_tail + 1) % m_buffer.size();
	}

	return count;
}
