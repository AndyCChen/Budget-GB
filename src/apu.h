#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "SDL3/SDL.h"
#include "emulatorConstants.h"

class Apu
{
  public:
	Apu(uint32_t sampleRate);
	~Apu();

	bool beginAudioFrame();
	void endAudioFrame();
	void tick(uint8_t divider);

	void    writeIO(uint16_t position, uint8_t data);
	uint8_t readIO(uint16_t position);

	void pauseAudio();
	void resumeAudio();

	struct AudioMasterControl
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
	struct MasterVolume
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

	struct PulseFrequencySweep
	{
		uint8_t ShiftSweep : 3;
		uint8_t Negate : 1;
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

	struct PulseLengthAndDutyCycle
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

	struct PulseVolumeAndEnvelope
	{
		uint8_t Period : 3;
		uint8_t Direction : 1; // 0: Volume decreases, 1: Volume increases
		uint8_t InitialVolume : 4;

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

	struct PulsePeriodLo
	{
		uint8_t PeriodLo8Bits;

		// Period lo is write only!
		uint8_t get() const
		{
			return 0;
		}

		void set(const uint8_t data)
		{
			PeriodLo8Bits = data;
		}
	};

	struct PulsePeriodHighAndControl
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

	struct Pulse1
	{
		struct Reg
		{
			PulseLengthAndDutyCycle   LengthAndDuty;
			PulseVolumeAndEnvelope    VolumeAndEnvelope;
			PulseFrequencySweep       FrequencySweep;
			PulsePeriodLo             PeriodLo;
			PulsePeriodHighAndControl PeriodHiAndControl;
		} Registers;

		uint16_t PeriodDivider;
		uint8_t  LengthPeriod;
		uint8_t  EnvelopePeriod;
		uint8_t  Volume;         // volume ranging $0 - $F
		uint8_t  DutyCycleIndex; // duty cycle index 0 - 7

		bool isDacOn() const
		{
			return (Registers.VolumeAndEnvelope.get() & 0xF8) != 0;
		}

		// outputs audio sample in range 0x0 - 0xF
		uint8_t outputSample() const;
		void    clockPeriodDivider();
	};

	/*struct Pulse2
	{
	    PulseLengthAndDutyCycle   LengthAndDuty;
	    PulseVolumeAndEnvelope    VolumeAndEnvelope;
	    PulsePeriodLo             PeriodLo;
	    PulsePeriodHighAndControl PeriodHiAndControl;
	};*/

	static constexpr std::array<std::array<uint8_t, 8>, 4> WAVE_DUTIES = {{
		{0, 0, 0, 0, 0, 0, 0, 1},
		{0, 1, 1, 1, 1, 1, 1, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 1},
	}};

	static constexpr uint8_t LENGTH_COUNTER_MAX = 64; // channel is silence once length counter reaches 64 (i.e. overflows 0x3F)

	class BoxFilter
	{
	  public:
		BoxFilter(uint32_t sampleRate);
		~BoxFilter();

		void pushSample(uint8_t sample);

		// samples are read into buffer and clamped into a 32 bit float sample in the range (-1.0f - 1.0f)
		uint32_t readSamples(float *buffer, uint32_t size);

		uint32_t getSamplesAvail() const
		{
			return m_samplesAvail;
		}

		bool isFull() const
		{
			return m_samplesAvail == m_buffer.size();
		}

		uint32_t getAudioFrameSize() const
		{
			return SAMPLE_RATE / 60;
		}

	  private:
		const int   SAMPLE_RATE;
		const float SAMPLES_PER_AVERAGE;

		std::vector<float> m_buffer;

		uint32_t m_runningSum       = 0;
		uint32_t m_sampleCountInBox = 0;
		uint32_t m_boxWidth         = 0;
		float    m_error            = 0;

		uint32_t m_head = 0, m_tail = 0, m_samplesAvail = 0;
	};

	class AudioThreadSampleBuffer
	{
	  public:
		void     pushSample(float sample);
		uint32_t readSamples(float *buffer, uint32_t size);

		uint32_t getSamplesAvail() const
		{
			return m_samplesAvail;
		}

	  private:
		std::array<float, BudgetGbConstants::AUDIO_SAMPLE_RATE / 60 * 16> m_buffer;

		uint32_t m_head = 0, m_tail = 0, m_samplesAvail;
	};

	struct AudioCallbackData
	{
		Apu::BoxFilter         *Buffer               = nullptr;
		uint32_t                RequestedSampleCount = 0;
		bool                    StopAudioPlayback    = false;
		AudioThreadSampleBuffer AudioBuffer;

		struct AudioThreadContext
		{
			SDL_Mutex     *Mutex      = nullptr;
			SDL_Mutex     *Mutex2     = nullptr;
			SDL_Condition *CanConsume = nullptr;
			SDL_Condition *CanProduce = nullptr;
		} AudioThreadCtx;
	};

  private:
	void mixAudio();

	AudioMasterControl m_audioControl{};
	MasterVolume       m_masterVolume{};

	Pulse1 m_pulse1{};

	uint8_t m_prevDivider = 0; // hold the previous divide value to detect a falling edge on bit 4
	uint8_t m_apuDivider  = 0; // incremented on bit 4 falling edge of system divider

	SDL_AudioStream  *m_audioStream;
	AudioCallbackData m_audioCallbackData{};

  public:
	BoxFilter m_boxFilter;
};
