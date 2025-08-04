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

	//if (!callBackData->StopAudioPlayback)
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
	}
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

	m_audioCallbackData.Buffer = &m_boxFilter;
	m_audioStream              = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, audioDeviceStreamCallback, (void *)&m_audioCallbackData);

	if (!m_audioStream)
		fmt::println("{}", SDL_GetError());
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
	{
		return true;
	}
}

void Apu::endAudioFrame()
{
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
	SDL_PauseAudioStreamDevice(m_audioStream);
	SDL_ClearAudioStream(m_audioStream);

}

void Apu::resumeAudio()
{
	SDL_LockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);
	m_audioCallbackData.StopAudioPlayback = false;
	SDL_UnlockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);

	SDL_ResumeAudioStreamDevice(m_audioStream);
}

void Apu::init(bool useBootrom)
{
	if (useBootrom)
	{
		m_audioControl = AudioMasterControl{};
		m_masterVolume = MasterVolume{};
		m_pulse1       = Pulse1{};
	}
	else
	{
		m_audioControl.set(0xF1);
		m_masterVolume.set(0x77);

		m_pulse1.Registers.FrequencySweep.set(0x80);
		m_pulse1.Registers.LengthAndDuty.set(0xBF);
		m_pulse1.Registers.VolumeAndEnvelope.set(0xF3);
		m_pulse1.Registers.PeriodLo.set(0xFF);
		m_pulse1.Registers.PeriodHiAndControl.set(0xBF);
	}

	m_prevDivider = 0;
	m_apuDivider  = 0;

	m_boxFilter.clear();
}

void Apu::mixAudio()
{
	uint8_t pulse1Sample = m_pulse1.outputSample();

	m_boxFilter.pushSample(pulse1Sample);
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
	  SAMPLES_PER_AVERAGE(static_cast<float>(BudgetGbConstants::CLOCK_RATE_T) / static_cast<float>(SAMPLE_RATE) / 4.0f),
	  BOX_WIDTH(static_cast<uint32_t>(SAMPLES_PER_AVERAGE))
{
	const int SIZE = (SAMPLE_RATE / 60) * 8;
	m_buffer.resize(SIZE);
}

void Apu::BoxFilter::pushSample(uint8_t sample)
{
	m_runningSum += sample;

	if (++m_sampleCountInBox == BOX_WIDTH + static_cast<uint32_t>(m_error))
	{
		m_samplesAvail     = std::min(m_samplesAvail + 1, (uint32_t)m_buffer.size());
		m_sampleCountInBox = 0;

		float average    = static_cast<float>(m_runningSum) / (BOX_WIDTH + static_cast<uint32_t>(m_error));
		m_buffer[m_head] = average;
		m_runningSum     = 0;

		m_head = (m_head + 1) % m_buffer.size();
		if (m_head == m_tail)
		{
			m_tail = (m_tail + 1) % m_buffer.size();
		}

		m_error -= static_cast<uint32_t>(m_error);
		m_error += SDL_fabsf(SAMPLES_PER_AVERAGE - BOX_WIDTH);
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
