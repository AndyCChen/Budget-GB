#pragma once

#include <cstdint>

#include "bus.h"

class Sm83
{
private:
	struct Sm83FlagsRegister
	{
		unsigned int Z : 1; // bit 7, zero flag
		unsigned int N : 1; // bit 6, subtraction flag
		unsigned int H : 1; // bit 5, half carry flag
		unsigned int C : 1; // bit 4, carry flag
		unsigned int UNUSED : 4; // unused

		/// <summary>
		/// Set flags with a single 8bit value as input.
		/// </summary>
		/// <param name="in">Flags as a 8bit bitfield</param>
		void setFlagsU8(uint8_t in)
		{
			Z = in >> 7;
			N = in >> 6;
			H = in >> 5;
			C = in >> 4;
			UNUSED = in & 0xF;
		}

		/// <summary>
		/// </summary>
		/// <returns>
		/// Returns flags as a single 8bit value
		/// </returns>
		uint8_t getFlagsU8()
		{
			return static_cast<uint8_t>((Z << 7) | (N << 6) | (H << 5) | (C << 4) | UNUSED);
		}
	};

	struct Sm83RegisterAF
	{
		unsigned int accumulator : 8;  // accumulator
		Sm83FlagsRegister flags;       // cpu flags
	};

	// general purpose registers
	struct Sm83Register
	{
		uint8_t hi;
		uint8_t lo;

		
	};

public:
	uint16_t       m_programCounter;
	uint16_t       m_stackPointer;
	Sm83RegisterAF m_registerAF;
	Sm83Register   m_registerBC;
	Sm83Register   m_registerDE;
	Sm83Register   m_registerHL;

	Sm83(Bus* bus);

	/// <summary>
	/// Emulate cpu for a single instruction.
	/// </summary>
	void run_instruction();

private:
	Bus* m_bus;

	uint8_t cpu_fetch();
};

