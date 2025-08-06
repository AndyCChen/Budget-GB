#include "apu.h"
#include "IORegisters.h"
#include "fmt/core.h"

#include <algorithm>

namespace
{
void SDLCALL audioDeviceStreamCallback(void *userdata, SDL_AudioStream *audioStream, int additionalAmount, int totalAmount)
{
	(void)additionalAmount;

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
			clockPulseLength(m_pulse1.Length);
			clockPulseLength(m_pulse2.Length);
		}

		// clock pulse1 frequency sweep
		if (m_apuDivider % 4 == 0)
		{
			m_pulse1.clockPulseFrequencySweep();
		}

		// clock volume envelope
		if (m_apuDivider % 8 == 0)
		{
			clockPulseVolumeEnvelope(m_pulse1.VolumeEnvelope, m_pulse1.Registers.VolumeAndEnvelope);
			clockPulseVolumeEnvelope(m_pulse2.VolumeEnvelope, m_pulse2.Registers.VolumeAndEnvelope);
			m_apuDivider = 0;
		}
	}
	m_prevDivider = divider;

	mixAudio();

	updateChannelStatus();

	clockPulsePeriod(m_pulse1.PeriodAndDuty, m_pulse1.Registers.PeriodLo, m_pulse1.Registers.PeriodHiAndControl);
	clockPulsePeriod(m_pulse2.PeriodAndDuty, m_pulse2.Registers.PeriodLo, m_pulse2.Registers.PeriodHiAndControl);
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
			m_audioControl.Pulse1Status = m_pulse1.Registers.VolumeAndEnvelope.isDacOn() ? 1 : 0;

			m_pulse1.PeriodAndDuty.PeriodDivider  = (m_pulse1.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | m_pulse1.Registers.PeriodLo.PeriodLo8Bits;
			m_pulse1.PeriodAndDuty.DutyCycleIndex = 0;

			m_pulse1.Length.LengthPeriod = m_pulse1.Length.isLengthCounterExpired() ? m_pulse1.Registers.LengthAndDuty.InitialLength : m_pulse1.Length.LengthPeriod;

			m_pulse1.VolumeEnvelope.EnvelopePeriod = 0;
			m_pulse1.VolumeEnvelope.Volume         = m_pulse1.Registers.VolumeAndEnvelope.InitialVolume;

			m_pulse1.FrequencySweep.SweepPeriod       = 0;
			m_pulse1.FrequencySweep.TempPeriodDivider = (m_pulse1.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | m_pulse1.Registers.PeriodLo.PeriodLo8Bits;

			m_pulse1.computeFrequencySweep();
		}

		break;

	case IORegisters::NR21:
		m_pulse2.Registers.LengthAndDuty.set(data);
		break;

	case IORegisters::NR22:
		m_pulse2.Registers.VolumeAndEnvelope.set(data);
		break;

	case IORegisters::NR23:
		m_pulse2.Registers.PeriodLo.set(data);
		break;

	case IORegisters::NR24:
		m_pulse2.Registers.PeriodHiAndControl.set(data);

		if (m_pulse2.Registers.PeriodHiAndControl.Trigger)
		{
			m_audioControl.Pulse2Status = m_pulse2.Registers.VolumeAndEnvelope.isDacOn() ? 1 : 0;

			m_pulse2.PeriodAndDuty.PeriodDivider  = (m_pulse2.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | m_pulse2.Registers.PeriodLo.PeriodLo8Bits;
			m_pulse2.PeriodAndDuty.DutyCycleIndex = 0;

			m_pulse2.Length.LengthPeriod = m_pulse2.Length.isLengthCounterExpired() ? m_pulse2.Registers.LengthAndDuty.InitialLength : m_pulse2.Length.LengthPeriod;

			m_pulse2.VolumeEnvelope.EnvelopePeriod = 0;
			m_pulse2.VolumeEnvelope.Volume         = m_pulse2.Registers.VolumeAndEnvelope.InitialVolume;
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

	case IORegisters::NR21:
		return m_pulse2.Registers.LengthAndDuty.get();

	case IORegisters::NR22:
		return m_pulse2.Registers.VolumeAndEnvelope.get();

	case IORegisters::NR23:
		return m_pulse2.Registers.PeriodLo.get();

	case IORegisters::NR24:
		return m_pulse2.Registers.PeriodHiAndControl.get();

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
	m_audioControl = RegisterAudioMasterControl{};
	m_masterVolume = RegisterMasterVolume{};
	m_pulse1       = Pulse1{};
	m_pulse2       = Pulse2{};

	if (!useBootrom)
	{
		m_audioControl.set(0xF1);
		m_masterVolume.set(0x77);

		m_pulse1.Registers.FrequencySweep.set(0x80);
		m_pulse1.Registers.LengthAndDuty.set(0xBF);
		m_pulse1.Registers.VolumeAndEnvelope.set(0xF3);
		m_pulse1.Registers.PeriodLo.set(0xFF);
		m_pulse1.Registers.PeriodHiAndControl.set(0xBF);

		m_pulse2.Registers.LengthAndDuty.set(0x3F);
		m_pulse2.Registers.VolumeAndEnvelope.set(0x00);
		m_pulse2.Registers.PeriodLo.set(0xFF);
		m_pulse2.Registers.PeriodHiAndControl.set(0xBF);
	}

	m_prevDivider = 0;
	m_apuDivider  = 0;

	m_boxFilter.clear();
}

void Apu::Pulse1::clockPulseFrequencySweep()
{
	// do not perform freq sweep iterations if the sweep period is 0
	if (Registers.FrequencySweep.Period == 0)
		return;

	if (++FrequencySweep.SweepPeriod == Registers.FrequencySweep.Period)
	{
		FrequencySweep.SweepPeriod = 0;

		// check if freq sweep is enabled
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

	uint16_t offset = FrequencySweep.TempPeriodDivider >> Registers.FrequencySweep.ShiftSweep;

	int targetPeriod = 0;

	if (Registers.FrequencySweep.Negate)
	{
		targetPeriod = FrequencySweep.TempPeriodDivider - offset;
		targetPeriod = std::max(targetPeriod, 0); // target period cannnot underflow to negative value
	}
	else
	{
		targetPeriod = FrequencySweep.TempPeriodDivider + offset;
	}

	if (targetPeriod <= 0x7FF)
	{
		FrequencySweep.TempPeriodDivider = static_cast<uint16_t>(targetPeriod);

		Registers.PeriodLo.PeriodLo8Bits           = static_cast<uint8_t>(targetPeriod);
		Registers.PeriodHiAndControl.PeriodHi3Bits = targetPeriod >> 8;
	}
}

uint8_t Apu::Pulse1::outputSample() const
{
	uint8_t sample = 0;

	if (Registers.VolumeAndEnvelope.isDacOn() && !isFreqSweepForcingSilence())
	{
		if (Registers.PeriodHiAndControl.LengthEnable)
		{
			if (!Length.isLengthCounterExpired())
			{
				sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][PeriodAndDuty.DutyCycleIndex];
				sample *= VolumeEnvelope.Volume;
			}
		}
		else
		{
			sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][PeriodAndDuty.DutyCycleIndex];
			sample *= VolumeEnvelope.Volume;
		}
	}

	return sample;
}

uint8_t Apu::Pulse2::outputSample() const
{
	uint8_t sample = 0;

	if (Registers.VolumeAndEnvelope.isDacOn())
	{
		if (Registers.PeriodHiAndControl.LengthEnable)
		{
			if (!Length.isLengthCounterExpired())
			{
				sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][PeriodAndDuty.DutyCycleIndex];
				sample *= VolumeEnvelope.Volume;
			}
		}
		else
		{
			sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][PeriodAndDuty.DutyCycleIndex];
			sample *= VolumeEnvelope.Volume;
		}
	}

	return sample;
}

void Apu::mixAudio()
{
	uint8_t pulse1Sample = m_pulse1.outputSample();
	uint8_t pulse2Sample = m_pulse2.outputSample();

	m_boxFilter.pushSample(pulse1Sample + pulse2Sample);
}

void Apu::updateChannelStatus()
{
	if (m_pulse1.Registers.PeriodHiAndControl.LengthEnable && m_pulse1.Length.isLengthCounterExpired())
	{
		m_audioControl.Pulse1Status = 0;
	}
}

void Apu::clockPulsePeriod(PulsePeriodDivider &in, RegisterPulsePeriodLo &periodLo, RegisterPulsePeriodHighAndControl &periodHi)
{
	if (++in.PeriodDivider == 0x800)
	{
		// divider overflowed 0x7FF
		in.DutyCycleIndex = (in.DutyCycleIndex + 1) % 8;
		in.PeriodDivider  = (periodHi.PeriodHi3Bits << 8) | periodLo.PeriodLo8Bits;
	}
}

void Apu::clockPulseVolumeEnvelope(PulseEnvelope &in, const RegisterPulseVolumeAndEnvelope &reg)
{
	// envelope period of zero means envelope is disabled
	if (reg.Period == 0)
		return;

	if (++in.EnvelopePeriod == reg.Period)
	{
		in.EnvelopePeriod = 0;

		if (reg.Direction)
		{
			in.Volume = static_cast<uint8_t>(std::clamp(in.Volume + 1, 0x0, 0xF));
		}
		else
		{
			in.Volume = static_cast<uint8_t>(std::clamp(in.Volume - 1, 0x0, 0xF));
		}
	}
}

void Apu::clockPulseLength(PulseLength &in)
{
	if (in.LengthPeriod < Apu::LENGTH_COUNTER_MAX)
	{
		++in.LengthPeriod;
	}
}
