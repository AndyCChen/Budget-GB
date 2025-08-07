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
	if (!(m_audioCallbackData.AudioThreadCtx.Mutex = SDL_CreateMutex()))
		fmt::println("{}", SDL_GetError());

	if (!m_audioStream)
		fmt::println("{}", SDL_GetError());

	SDL_AudioSpec audioSpec{};
	audioSpec.format   = SDL_AUDIO_F32;
	audioSpec.freq     = sampleRate;
	audioSpec.channels = 1;

	m_audioCallbackData.Buffer = &m_boxFilter;

	m_audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, audioDeviceStreamCallback, (void *)&m_audioCallbackData);
}

bool Apu::beginAudioFrame()
{
	SDL_LockMutex(m_audioCallbackData.AudioThreadCtx.Mutex);

	if (m_boxFilter.getSamplesAvail() > m_boxFilter.getAudioFrameSize() * 3)
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
			m_wave.clockWaveLength();
			m_noise.clockNoiseLength();
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
			m_noise.clockNoiseEnvelope();
			m_apuDivider = 0;
		}
	}

	mixAudio();
	updateChannelStatus();

	clockPulsePeriod(m_pulse1.PeriodAndDuty, m_pulse1.Registers.PeriodLo, m_pulse1.Registers.PeriodHiAndControl);
	clockPulsePeriod(m_pulse2.PeriodAndDuty, m_pulse2.Registers.PeriodLo, m_pulse2.Registers.PeriodHiAndControl);

	// wave period clocks at 2097152hz
	m_wave.clockWavePeriod(m_waveRam);
	m_wave.clockWavePeriod(m_waveRam);

	m_noise.clockNoisePeriod();

	m_prevDivider = divider;
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

		if (!m_pulse1.Registers.VolumeAndEnvelope.isDacOn())
			m_audioControl.Pulse1Status = 0;

		break;

	case IORegisters::NR13:
		m_pulse1.Registers.PeriodLo.PeriodLo8Bits = data;
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

		if (!m_pulse2.Registers.VolumeAndEnvelope.isDacOn())
			m_audioControl.Pulse2Status = 0;

		break;

	case IORegisters::NR23:
		m_pulse2.Registers.PeriodLo.PeriodLo8Bits = data;
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

	case IORegisters::NR30:
		m_wave.Registers.DacEnable.set(data);

		if (m_wave.Registers.DacEnable.get() == 0)
			m_audioControl.WaveStatus = 0;

		break;

	case IORegisters::NR31:
		m_wave.Registers.WaveLength.LengthPeriod = data;
		break;

	case IORegisters::NR32:
		m_wave.Registers.OutLevel.set(data);

		switch (m_wave.Registers.OutLevel.OutputLevel)
		{
		case 0:
			m_wave.Volume = 4;
			break;
		case 1:
			m_wave.Volume = 0;
			break;
		case 2:
			m_wave.Volume = 1;
			break;
		case 3:
			m_wave.Volume = 2;
			break;
		default:
			break;
		}

		break;

	case IORegisters::NR33:
		m_wave.Registers.PeriodLo.PeriodLo8Bits = data;
		break;

	case IORegisters::NR34:
		m_wave.Registers.PeriodHiAndControl.set(data);

		if (m_wave.Registers.PeriodHiAndControl.Trigger)
		{
			m_audioControl.WaveStatus = m_wave.Registers.DacEnable.get() != 0;
			m_wave.LengthPeriod       = m_wave.islengthExpired() ? 0 : m_wave.Registers.WaveLength.LengthPeriod;
			m_wave.PeriodDivider      = (m_wave.Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | (m_wave.Registers.PeriodLo.PeriodLo8Bits);
			m_wave.WaveRamIndex       = 0;
		}

		break;

	case IORegisters::NR41:
		m_noise.Registers.InitialLength = data;
		break;

	case IORegisters::NR42:
		m_noise.Registers.VolumeAndEnvelope.set(data);
		break;

	case IORegisters::NR43:
		m_noise.Registers.FreqAndRand.set(data);
		break;

	case IORegisters::NR44:
		m_noise.Registers.Control.set(data);

		if (m_noise.Registers.Control.Trigger)
		{
			m_audioControl.NoiseStatus = m_noise.Registers.VolumeAndEnvelope.isDacOn();
			m_noise.LengthTimer        = m_noise.isLengthExpired() ? 0 : m_noise.LengthTimer;
			m_noise.EnvelopeTimer      = 0;
			m_noise.Volume             = m_noise.Registers.VolumeAndEnvelope.InitialVolume;
			m_noise.LFSR               = 0;
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

	case IORegisters::NR14:
		return m_pulse1.Registers.PeriodHiAndControl.get();

	case IORegisters::NR21:
		return m_pulse2.Registers.LengthAndDuty.get();

	case IORegisters::NR22:
		return m_pulse2.Registers.VolumeAndEnvelope.get();

	case IORegisters::NR24:
		return m_pulse2.Registers.PeriodHiAndControl.get();

	case IORegisters::NR30:
		return m_wave.Registers.DacEnable.get();

	case IORegisters::NR32:
		return m_wave.Registers.OutLevel.get();

	case IORegisters::NR34:
		return m_wave.Registers.PeriodHiAndControl.get();

	case IORegisters::NR42:
		return m_noise.Registers.VolumeAndEnvelope.get();

	case IORegisters::NR43:
		return m_noise.Registers.FreqAndRand.get();

	case IORegisters::NR44:
		return m_noise.Registers.Control.get();

	case IORegisters::NR50:
		return m_masterVolume.get();

	case IORegisters::NR52:
		return m_audioControl.get();

	default:
		return 0xFF;
	}
}

void Apu::writeWaveRam(uint16_t position, uint8_t data)
{
	m_waveRam[position & 0xF] = data;
}

uint8_t Apu::readWaveRam(uint16_t position) const
{
	return m_waveRam[position & 0xF];
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

	m_pulse1 = Pulse1{};
	m_pulse2 = Pulse2{};
	m_wave   = Wave{};
	m_noise  = Noise{};

	m_waveRam.fill(0);

	if (!useBootrom)
	{
		m_audioControl.set(0xF1);
		m_masterVolume.set(0x77);

		m_pulse1.Registers.FrequencySweep.set(0x80);
		m_pulse1.Registers.LengthAndDuty.set(0xBF);
		m_pulse1.Registers.VolumeAndEnvelope.set(0xF3);
		m_pulse1.Registers.PeriodLo.PeriodLo8Bits = 0xFF;
		m_pulse1.Registers.PeriodHiAndControl.set(0xBF);

		m_pulse2.Registers.LengthAndDuty.set(0x3F);
		m_pulse2.Registers.VolumeAndEnvelope.set(0x00);
		m_pulse2.Registers.PeriodLo.PeriodLo8Bits = 0xFF;
		m_pulse2.Registers.PeriodHiAndControl.set(0xBF);

		m_wave.Registers.DacEnable.set(0x7F);
		m_wave.Registers.WaveLength.LengthPeriod = 0xFF;
		m_wave.Registers.OutLevel.set(0x9F);
		m_wave.Registers.PeriodLo.PeriodLo8Bits = 0xFF;
		m_wave.Registers.PeriodHiAndControl.set(0xBF);

		m_noise.Registers.InitialLength = 0xFF;
		m_noise.Registers.VolumeAndEnvelope.set(0x00);
		m_noise.Registers.FreqAndRand.set(0x00);
		m_noise.Registers.Control.set(0xBF);
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
		if (Registers.PeriodHiAndControl.LengthEnable && Length.isLengthCounterExpired())
			return sample;

		sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][PeriodAndDuty.DutyCycleIndex];
		sample *= VolumeEnvelope.Volume;
	}

	return sample;
}

uint8_t Apu::Pulse2::outputSample() const
{
	uint8_t sample = 0;

	if (Registers.VolumeAndEnvelope.isDacOn())
	{
		if (Registers.PeriodHiAndControl.LengthEnable && Length.isLengthCounterExpired())
			return sample;

		sample = Apu::WAVE_DUTIES[Registers.LengthAndDuty.WaveDuty][PeriodAndDuty.DutyCycleIndex];
		sample *= VolumeEnvelope.Volume;
	}

	return sample;
}

void Apu::mixAudio()
{
	uint8_t pulse1Sample = m_pulse1.outputSample();
	uint8_t pulse2Sample = m_pulse2.outputSample();
	uint8_t waveSample   = m_wave.outputSample();
	uint8_t noiseSample  = m_noise.outputSample();

	m_boxFilter.pushSample(pulse1Sample + pulse2Sample + waveSample + noiseSample);
}

void Apu::updateChannelStatus()
{
	if (m_pulse1.Registers.PeriodHiAndControl.LengthEnable && m_pulse1.Length.isLengthCounterExpired())
	{
		m_audioControl.Pulse1Status = 0;
	}

	if (m_pulse2.Registers.PeriodHiAndControl.LengthEnable && m_pulse2.Length.isLengthCounterExpired())
	{
		m_audioControl.Pulse2Status = 0;
	}

	if (m_wave.Registers.PeriodHiAndControl.LengthEnable && m_wave.islengthExpired())
	{
		m_audioControl.WaveStatus = 0;
	}

	if (m_noise.Registers.Control.LengthEnable && m_noise.isLengthExpired())
	{
		m_audioControl.NoiseStatus = 0;
	}
}

void Apu::clockPulsePeriod(PulsePeriodDivider &in, RegisterPulsePeriodLo &periodLo, RegisterPulsePeriodHighAndControl &periodHi)
{
	if (++in.PeriodDivider > 0x7FF)
	{
		// divider overflowed 0x7FF
		in.DutyCycleIndex = (in.DutyCycleIndex + 1) % 8;
		in.PeriodDivider  = (periodHi.PeriodHi3Bits << 8) | periodLo.PeriodLo8Bits;
	}
}

void Apu::clockPulseVolumeEnvelope(PulseEnvelope &in, const RegisterVolumeAndEnvelope &reg)
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
	if (in.LengthPeriod < LENGTH_COUNTER_MAX)
	{
		++in.LengthPeriod;
	}
}

void Apu::Wave::clockWaveLength()
{
	if (LengthPeriod < LENGTH_COUNTER_MAX)
	{
		++LengthPeriod;
	}
}

void Apu::Wave::clockWavePeriod(const std::array<uint8_t, 16> &waveRam)
{
	if (++PeriodDivider > 0x7FF)
	{
		PeriodDivider = (Registers.PeriodHiAndControl.PeriodHi3Bits << 8) | Registers.PeriodLo.PeriodLo8Bits;
		WaveRamIndex  = (WaveRamIndex + 1) % (waveRam.size() * 2);

		SampleBuffer = waveRam[(WaveRamIndex >> 1)];
		if (WaveRamIndex & 1)
			SampleBuffer &= 0x0F;
		else
			SampleBuffer >>= 4;
	}
}

uint8_t Apu::Wave::outputSample() const
{
	uint8_t sample = 0;

	if (Registers.DacEnable.get())
	{
		if (Registers.PeriodHiAndControl.LengthEnable && islengthExpired())
			return sample;

		sample = SampleBuffer;
		sample >>= Volume;
	}

	return sample;
}

void Apu::Noise::clockNoisePeriod()
{
	if (++PeriodDivider >= Registers.FreqAndRand.FrequencyPeriod)
	{
		PeriodDivider = 0;

		uint8_t bit0 = LFSR & 1;
		uint8_t bit1 = (LFSR >> 1) & 1;

		uint8_t feedback = !(bit1 ^ bit0);
		LFSR |= (feedback << 15);

		if (Registers.FreqAndRand.LfsrWidth)
			LFSR |= (feedback << 7);

		LFSR >>= 1;
	}
}

void Apu::Noise::clockNoiseLength()
{
	if (LengthTimer < LENGTH_COUNTER_MAX)
	{
		++LengthTimer;
	}
}

void Apu::Noise::clockNoiseEnvelope()
{
	// envelope period of zero means envelope is disabled
	if (Registers.VolumeAndEnvelope.Period == 0)
		return;

	if (++EnvelopeTimer == Registers.VolumeAndEnvelope.Period)
	{
		EnvelopeTimer = 0;

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

uint8_t Apu::Noise::outputSample() const
{
	uint8_t sample = 0;

	if (Registers.VolumeAndEnvelope.isDacOn())
	{
		if (Registers.Control.LengthEnable && isLengthExpired())
			return sample;

		sample = LFSR & 1;
		sample *= Volume;
	}

	return sample;
}
