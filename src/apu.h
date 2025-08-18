#pragma once

#include <array>
#include <cstdint>
#include <utility>

#include "BoxFilter.h"
#include "SDL3/SDL.h"
#include "Utils/vec.h"
#include "audioLogBuffer.h"
#include "emulatorConstants.h"

class Apu
{
  public:
	Apu(uint32_t sampleRate);

	bool beginAudioFrame();
	void endAudioFrame();
	void tick(uint8_t divider);

	void    writeIO(uint16_t position, uint8_t data);
	uint8_t readIO(uint16_t position);

	void    writeWaveRam(uint16_t position, uint8_t data);
	uint8_t readWaveRam(uint16_t position) const;

	void pauseAudio();
	void resumeAudio();

	AudioLogging::AudioLogBuffers &getAudioLogBuffers()
	{
		return m_audioLogBuffers;
	}

	void init(bool useBootrom);

	static constexpr std::array<std::array<uint8_t, 8>, 4> WAVE_DUTIES = {{
		{0, 0, 0, 0, 0, 0, 0, 1},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 1},
	}};

	static constexpr uint8_t LENGTH_COUNTER_MAX = 64; // channel is silence once length counter reaches 64 (i.e. overflows 0x3F)

	struct RegisterAudioMasterControl
	{
		uint8_t Pulse1Status : 1;
		uint8_t Pulse2Status : 1;
		uint8_t WaveStatus : 1;
		uint8_t NoiseStatus : 1;
		uint8_t AudioOnOff : 1;

		uint8_t get() const
		{
			return static_cast<uint8_t>((AudioOnOff << 7) | (NoiseStatus << 3) | (WaveStatus << 2) | (Pulse2Status << 1) | Pulse1Status);
		}

		// all registers except audio on/off are read only
		void set(const uint8_t data)
		{
			AudioOnOff = data >> 7;
		}
	};

	// external audio for VIN left unimplemented
	struct RegisterMasterVolume
	{
		uint8_t RightVolume : 3;
		uint8_t LeftVolume : 3;

		uint8_t get() const
		{
			return static_cast<uint8_t>(RightVolume | (LeftVolume << 4));
		}

		void set(const uint8_t data)
		{
			RightVolume = data & 0x7;
			LeftVolume  = data >> 4;
		}
	};

	struct RegisterPulseFrequencySweep
	{
		uint8_t ShiftSweep : 3;
		uint8_t Negate : 1; // 1: Period decreases over time, 0: Period increases over time
		uint8_t Period : 3;

		uint8_t get() const
		{
			return static_cast<uint8_t>(ShiftSweep | (Negate << 3) | (Period << 4));
		}

		void set(const uint8_t data)
		{
			ShiftSweep = data & 0x7;
			Negate     = data >> 3;
			Period     = data >> 4;
		}
	};

	struct RegisterPulseLengthAndDutyCycle
	{
		uint8_t InitialLength : 6;
		uint8_t WaveDuty : 2;

		// length counter is write only
		uint8_t get() const
		{
			return static_cast<uint8_t>((WaveDuty << 6));
		}

		void set(const uint8_t data)
		{
			InitialLength = data;
			WaveDuty      = data >> 6;
		}
	};

	struct RegisterVolumeAndEnvelope
	{
		uint8_t Period : 3;
		uint8_t Direction : 1; // 0: Volume decreases, 1: Volume increases
		uint8_t InitialVolume : 4;

		bool isDacOn() const
		{
			return (get() & 0xF8) != 0;
		}

		uint8_t get() const
		{
			return static_cast<uint8_t>(Period | (Direction << 3) | (InitialVolume << 4));
		}

		void set(const uint8_t data)
		{
			Period        = data;
			Direction     = data >> 3;
			InitialVolume = data >> 4;
		}
	};

	// write only
	struct RegisterPulsePeriodLo
	{
		uint8_t PeriodLo8Bits;
	};

	struct RegisterPulsePeriodHighAndControl
	{
		uint8_t PeriodHi3Bits : 3;
		uint8_t LengthEnable : 1;
		uint8_t Trigger : 1;

		// All registers except LengthEnable are write only
		uint8_t get() const
		{
			return static_cast<uint8_t>(LengthEnable << 6);
		}

		void set(const uint8_t data)
		{
			PeriodHi3Bits = data;
			LengthEnable  = data >> 6;
			Trigger       = data >> 7;
		}
	};

	struct RegisterWaveDacEnable
	{
		uint8_t DacEnable : 1;

		uint8_t get() const
		{
			return static_cast<uint8_t>(DacEnable << 7);
		}

		void set(const uint8_t data)
		{
			DacEnable = data >> 7;
		}
	};

	// write only
	struct RegisterWaveLength
	{
		uint8_t LengthPeriod;
	};

	struct RegisterWaveOutputLevel
	{
		uint8_t OutputLevel : 2;

		uint8_t get() const
		{
			return static_cast<uint8_t>(OutputLevel << 5);
		}

		void set(const uint8_t data)
		{
			OutputLevel = data >> 5;
		}
	};

	struct RegisterWavePeriodLo
	{
		uint8_t PeriodLo8Bits;
	};

	struct RegisterWavePeriodHiAndControl
	{
		uint8_t Trigger : 1;
		uint8_t LengthEnable : 1;
		uint8_t PeriodHi3Bits : 3;

		uint8_t get() const
		{
			return static_cast<uint8_t>(LengthEnable << 6);
		}

		void set(const uint8_t data)
		{
			Trigger       = data >> 7;
			LengthEnable  = data >> 6;
			PeriodHi3Bits = data & 0x7;
		}
	};

	struct RegisterNoiseFreqAndRand
	{
		uint8_t ClockShift : 4;
		uint8_t LfsrWidth : 1;
		uint8_t ClockDivider : 3;

		uint8_t get() const
		{
			return static_cast<uint8_t>((ClockShift << 4) | (LfsrWidth << 3) | ClockDivider);
		}

		void set(const uint8_t data)
		{
			ClockShift   = data >> 4;
			LfsrWidth    = data >> 3;
			ClockDivider = data;
		}
	};

	struct RegisterNoiseControl
	{
		uint8_t Trigger : 1; // write only
		uint8_t LengthEnable : 1;

		uint8_t get() const
		{
			return static_cast<uint8_t>(LengthEnable << 6);
		}

		void set(const uint8_t data)
		{
			Trigger      = data >> 7;
			LengthEnable = data >> 6;
		}
	};

	// PeriodDivider, Volume, Length, and Frequncy components of pulse channels

	struct PulsePeriodDivider
	{
		uint16_t PeriodDivider;  // the channel's main period divider aka its frequency
		uint8_t  DutyCycleIndex; // duty cycle index 0 - 7
	};

	struct PulseEnvelope
	{
		uint8_t Volume;         // volume ranging $0 - $F
		uint8_t EnvelopePeriod; // volume ranging $0 - $F
	};

	struct PulseLength
	{
		uint8_t LengthPeriod; // auto duration control

		bool isLengthCounterExpired() const
		{
			return LengthPeriod == Apu::LENGTH_COUNTER_MAX;
		}
	};

	struct PulseFrequencySweep
	{
		uint16_t TempPeriodDivider; // the calculated frequency from period sweep changes
		uint8_t  SweepPeriod;       // control frequency change (PeriodDivider) over time;
	};

	// Sound channels definitions

	struct Pulse1
	{
		struct Reg
		{
			RegisterPulseLengthAndDutyCycle   LengthAndDuty;
			RegisterVolumeAndEnvelope         VolumeAndEnvelope;
			RegisterPulseFrequencySweep       FrequencySweep;
			RegisterPulsePeriodLo             PeriodLo;
			RegisterPulsePeriodHighAndControl PeriodHiAndControl;
		} Registers;

		PulsePeriodDivider PeriodAndDuty{};

		PulseEnvelope       VolumeEnvelope{};
		PulseLength         Length{};
		PulseFrequencySweep FrequencySweep{};

		bool isFreqSweepEnabled() const
		{
			return Registers.FrequencySweep.Period != 0 || Registers.FrequencySweep.ShiftSweep != 0;
		}

		// overflowing 0x7FF in addition mode silences the pulse channel
		bool isFreqSweepForcingSilence() const
		{
			return !Registers.FrequencySweep.Negate && (FrequencySweep.TempPeriodDivider >> Registers.FrequencySweep.ShiftSweep) > 0x7FF;
		}

		void clockPulseFrequencySweep();
		void computeFrequencySweep();

		// outputs audio sample in range 0x0 - 0xF
		uint8_t outputSample() const;
	};

	struct Pulse2
	{
		struct Reg
		{
			RegisterPulseLengthAndDutyCycle   LengthAndDuty;
			RegisterVolumeAndEnvelope         VolumeAndEnvelope;
			RegisterPulsePeriodLo             PeriodLo;
			RegisterPulsePeriodHighAndControl PeriodHiAndControl;
		} Registers;

		PulsePeriodDivider PeriodAndDuty{};

		PulseEnvelope VolumeEnvelope{};
		PulseLength   Length{};

		uint8_t outputSample() const;
	};

	struct Wave
	{
		struct Reg
		{
			RegisterWaveDacEnable          DacEnable;
			RegisterWaveLength             WaveLength;
			RegisterWaveOutputLevel        OutLevel;
			RegisterWavePeriodLo           PeriodLo;
			RegisterWavePeriodHiAndControl PeriodHiAndControl;
		} Registers;

		uint16_t PeriodDivider;
		uint8_t  LengthPeriod;
		uint8_t  Volume;       // here volume represents how many bits to right shift by when outputing samples from wave ram
		uint8_t  WaveRamIndex; // wave ram index for 32 samples from 16 byte wave ram. Each index accesses a 4-bit nibble from a byte (32 total nibbles)

		uint8_t SampleBuffer; // holds the sample read from wave ram and repeately outputs this sample until WaveRamIndex is incremented and a new sample is read

		bool islengthExpired() const
		{
			return LengthPeriod >= LENGTH_COUNTER_MAX;
		}

		void clockWaveLength();
		void clockWavePeriod(const std::array<uint8_t, 16> &waveRam);

		uint8_t outputSample() const;
	};

	struct Noise
	{
		struct Reg
		{
			uint8_t                   InitialLength : 6;
			RegisterVolumeAndEnvelope VolumeAndEnvelope;
			RegisterNoiseFreqAndRand  FreqAndRand;
			RegisterNoiseControl      Control;
		} Registers;

		uint32_t PeriodDivider;
		uint16_t LFSR; // linear feedback shift register

		uint8_t LengthTimer;
		uint8_t EnvelopeTimer;
		uint8_t Volume;

		bool isLengthExpired() const
		{
			return LengthTimer >= LENGTH_COUNTER_MAX;
		}

		void clockNoisePeriod();
		void clockNoiseLength();
		void clockNoiseEnvelope();

		uint8_t outputSample() const;
	};

	struct AudioCallbackData
	{
		BoxFilter *Buffer            = nullptr;
		bool       StopAudioPlayback = false;

		struct AudioThreadContext
		{
			SDL_Mutex *Mutex = nullptr;
		} AudioThreadCtx;
	};

	struct AudioChannelToggle
	{
		bool Pulse1 = true;
		bool Pulse2 = true;
		bool Wave   = true;
		bool Noise  = true;
	};

	void setAudioChannelToggle(const AudioChannelToggle &channelToggle)
	{
		m_audioChannelToggle = channelToggle;
	}

	const AudioChannelToggle &getAudioChannelToggle() const
	{
		return m_audioChannelToggle;
	}

  private:
	void mixAudio();
	void updateChannelStatus();

	static void clockPulsePeriod(PulsePeriodDivider &in, RegisterPulsePeriodLo &periodLo, RegisterPulsePeriodHighAndControl &periodHi);
	static void clockPulseVolumeEnvelope(PulseEnvelope &in, const RegisterVolumeAndEnvelope &reg);
	static void clockPulseLength(PulseLength &in);

	RegisterAudioMasterControl m_audioControl{};
	RegisterMasterVolume       m_masterVolume{};

	Pulse1 m_pulse1{};
	Pulse2 m_pulse2{};
	Wave   m_wave{};
	Noise  m_noise{};

	std::array<uint8_t, 16> m_waveRam{};

	uint16_t m_prevDivider = 0; // hold the previous divide value to detect a falling edge on bit 4
	uint8_t  m_apuDivider  = 0; // incremented on bit 4 falling edge of system divider

	SDL_AudioStream  *m_audioStream;
	AudioCallbackData m_audioCallbackData{};

	BoxFilter m_boxFilter;

	AudioLogging::AudioLogBuffers m_audioLogBuffers;

	AudioChannelToggle m_audioChannelToggle;
};
