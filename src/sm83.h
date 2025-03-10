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
		/// Set all at once flags with a single 8bit value as input.
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
		/// Returns all flags together as a single 8bit value
		/// </returns>
		uint8_t getFlagsU8() const
		{
			return static_cast<uint8_t>((Z << 7) | (N << 6) | (H << 5) | (C << 4) | UNUSED);
		}
	};

	struct Sm83RegisterAF
	{
		uint8_t accumulator;  // accumulator
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
	void runInstruction();

private:
	Bus* m_bus;

	/// <summary>
	/// Read from memory currently pointed to by program counter
	/// and increment program counter;
	/// </summary>
	/// <returns>Unsiged byte that is read.</returns>
	uint8_t cpuFetch()
	{
		return m_bus->cpu_read(m_programCounter++);
	}

	/// <summary>
	/// Decodes and executes the input opcode.
	/// </summary>
	void decodeExecute(uint8_t opcode);

	// r8  - means 8 bit register
	// r16 - means 16 bit register (8 bit register pair)
	// n8  - means immediate 8 bit data
	// n16 - means immediate 16 bit data

	// LD Instructions

	/// <summary>
	/// Load 16-bit register with immediate 16-bit value (next 2 bytes following opcode)
	/// </summary>
	void LD_r16_n16(Sm83Register& dest);

	/// <summary>
	/// Load address pointed to by register pair dest with value from accumulator.
	/// </summary>
	void LD_indirect_r16_A(Sm83Register& dest);

	// INC Intructions

	/// <summary>
	/// Increment 8 bit registers, Set zero flag if result is zero,
	/// Set half carry flag on bit 3 overflow, Clear subtraction flag.
	/// </summary>
	void INC_r8(uint8_t& dest);

	/// <summary>
	/// Increment 16-bit register
	/// </summary>
	void INC_r16(Sm83Register& dest);

};

