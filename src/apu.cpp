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

		// clock length timers
		if (m_apuDivider % 2 == 0)
		{
			m_pulse1.clockLengthCounter();
		}

		// clock pulse1 frequency sweep
		if (m_apuDivider % 4 == 0)
		{
			m_pulse1.clockFrequencyCounter();
		}

		// clock envelope sweep
		if (m_apuDivider % 8 == 0)
		{
			m_pulse1.clockEnvelopeCounter();
			m_apuDivider = 0;
		}
	}
	m_prevDivider = divider;

	mixAudio();

	updateChannelStatus();
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
			m_audioControl.Pulse1Status = m_pulse1.isDacOn() ? 1 : 0;
			m_pulse1.LengthPeriod       = m_pulse1.isLengthCounterExpired() ? m_pulse1.Registers.LengthAndDuty.InitialLength : m_pulse1.LengthPeriod;
			m_pulse1.PeriodDivider      = (m_pulse1.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | m_pulse1.Registers.PeriodLo.PeriodLo8Bits;
			m_pulse1.EnvelopePeriod     = 0;
			m_pulse1.Volume             = m_pulse1.Registers.VolumeAndEnvelope.InitialVolume;

			m_pulse1.SweepPeriod       = 0;
			m_pulse1.TempPeriodDivider = (m_pulse1.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | m_pulse1.Registers.PeriodLo.PeriodLo8Bits;
			m_pulse1.computeFrequencySweep();
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

void Apu::updateChannelStatus()
{
	if (m_pulse1.Registers.PeriodHiAndControl.LengthEnable && m_pulse1.isLengthCounterExpired())
	{
		m_audioControl.Pulse1Status = 0;
	}
}

uint8_t Apu::Pulse1::outputSample() const
{
	uint8_t sample = 0;

	if (isDacOn() && !isFreqSweepForcingSilence())
	{
		if (Registers.PeriodHiAndControl.LengthEnable)
		{
			if (!isLengthCounterExpired())
			{
				sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][DutyCycleIndex];
				sample *= Volume;
			}
		}
		else
		{
			sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][DutyCycleIndex];
			sample *= Volume;
		}
	}

	return sample;
}

void Apu::Pulse1::clockLengthCounter()
{
	if (LengthPeriod < Apu::LENGTH_COUNTER_MAX)
	{
		++LengthPeriod;
	}
}

void Apu::Pulse1::clockEnvelopeCounter()
{
	// envelope period of zero means envelope is disabled
	if (Registers.VolumeAndEnvelope.Period == 0)
		return;

	if (++EnvelopePeriod == Registers.VolumeAndEnvelope.Period)
	{
		EnvelopePeriod = 0;

		if (Registers.VolumeAndEnvelope.Direction)
		{
			Volume = static_cast<uint8_t>(std::clamp(Volume + 1, 0x0, 0xF));
		}
		else
		{
			Volume = static_cast<uint8_t>(std::clamp(Volume - 1, 0x0, 0xF));
		}
	}
}

void Apu::Pulse1::clockFrequencyCounter()
{
	// do not perform freq sweep iterations if the sweep period is 0
	if (Registers.FrequencySweep.Period == 0)
		return;

	if (++SweepPeriod == Registers.FrequencySweep.Period)
	{
		SweepPeriod = 0;

		if (isFreqSweepEnabled())
		{
			computeFrequencySweep();
		}
	}
}

void Apu::Pulse1::computeFrequencySweep()
{
	if (Registers.FrequencySweep.ShiftSweep == 0)
		return;

	uint16_t offset = TempPeriodDivider >> Registers.FrequencySweep.ShiftSweep;

	int targetPeriod = 0;

	if (Registers.FrequencySweep.Negate)
	{
		targetPeriod = TempPeriodDivider - offset;
		targetPeriod = std::max(targetPeriod, 0); // target period cannnot underflow to negative value
	}
	else
	{
		targetPeriod = TempPeriodDivider + offset;
	}

	if (targetPeriod <= 0x7FF)
	{
		TempPeriodDivider = static_cast<uint16_t>(targetPeriod);

		Registers.PeriodLo.PeriodLo8Bits           = static_cast<uint8_t>(targetPeriod);
		Registers.PeriodHiAndControl.PeriodHi3Bits = targetPeriod >> 8;
	}
}

void Apu::Pulse1::clockPeriodDivider()
{
	if (++PeriodDivider == 0x800)
	{
		// divider overflowed 0x7FF
		DutyCycleIndex = (DutyCycleIndex + 1) % 8;
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
	uint32_t        count         = 0;
	constexpr float MASTER_VOLUME = 0.05f;

	while (count < size && m_samplesAvail > 0)
	{
		--m_samplesAvail;
		buffer[count++] = ((m_buffer[m_tail] - 7.5f) / 7.5f) * MASTER_VOLUME;

		if (m_tail != m_head)
			m_tail = (m_tail + 1) % m_buffer.size();
	}

	return count;
}
