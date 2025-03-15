#pragma once

#include <cstdint>

#include "bus.h"

class Sm83
{
  private:
	struct Sm83FlagsRegister
	{
		unsigned int Z : 1;		 // bit 7, zero flag
		unsigned int N : 1;		 // bit 6, subtraction flag
		unsigned int H : 1;		 // bit 5, half carry flag
		unsigned int C : 1;		 // bit 4, carry flag
		unsigned int UNUSED : 4; // unused

		/**
		 * @brief Set all at once flags with a single 8bit value as input.
		 */
		void setFlagsU8(uint8_t in)
		{
			Z = in >> 7;
			N = in >> 6;
			H = in >> 5;
			C = in >> 4;
			UNUSED = in & 0xF;
		}

		/**
		 * @brief Returns all flags together as a single 8bit value
		 */
		uint8_t getFlagsU8() const
		{
			return static_cast<uint8_t>((Z << 7) | (N << 6) | (H << 5) | (C << 4) | UNUSED);
		}
	};

	struct Sm83RegisterAF
	{
		uint8_t accumulator;	 // accumulator
		Sm83FlagsRegister flags; // cpu flags
	};

	// general purpose registers
	struct Sm83Register
	{
		uint8_t hi;
		uint8_t lo;
	};

  public:
	uint16_t m_programCounter;
	uint16_t m_stackPointer;
	Sm83RegisterAF m_registerAF;
	Sm83Register m_registerBC;
	Sm83Register m_registerDE;
	Sm83Register m_registerHL;

	Sm83(Bus *bus);

	/**
	 * @brief Emulate cpu for a single instruction.
	 */
	void runInstruction();

  private:
	Bus *m_bus;

	/**
	 * @brief Read from memory currently pointed to by program counter
	 * and increment program counter
	 * @return 8bit data that is read.
	 */
	uint8_t cpuFetch()
	{
		return m_bus->cpu_read(m_programCounter++);
	}

	/**
	 * @brief Decodes and executes the input opcode.
	 */
	void decodeExecute(uint8_t opcode);

	// -------------------------------------------- SM83 CPU Intructions ------------------------------------------

	// r8  - means 8 bit register
	// r16 - means 16 bit register (8 bit register pair)
	// n8  - means immediate 8 bit data
	// n16 - means immediate 16 bit data
	// indirect - means reading from a memory address where address is a n16 immediate value or r16 register value
	// i8  - means signed 8 bit data
	// CC  - means some booleon condition, NZ: zero flag not set, Z: set flag set, NC: carry flag not set, C: carry flag
	// set

	// LD Instructions ---------------------------------------------

	/**
	 * @brief Load 8-bit register with immediate 8-bit value
	 */
	void LD_r8_n8(uint8_t &dest);

	/**
	 * @brief Load 16-bit register with immediate 16-bit value (next 2 bytes following opcode)
	 * @param dest
	 */
	void LD_r16_n16(Sm83Register &dest);

	/**
	 * @brief Load address pointed to by register pair dest with value from 8 bit register.
	 * @param dest
	 * @param src
	 */
	void LD_indirect_r16_r8(Sm83Register &dest, uint8_t &src);

	/**
	 * @brief Load 8 bit register with value at memory address location in 16 bit register.
	 * @param src
	 */
	void LD_A_indirect_r16(Sm83Register &src);

	/**
	 * @brief Load value of stack pointer into the n16 immediate value memory address. Lo
	 * byte of stack pointer to loaded first into address, followed by the hi byte at the next memory address.
	 */
	void LD_indirect_n16_SP();

	// INC Intructions ---------------------------------------------

	/**
	 * @brief Increment 8 bit register, Set zero flag if result is zero,
	 * Set half carry flag on bit 3 overflow, Clear subtraction flag.
	 * @param dest
	 */
	void INC_r8(uint8_t &dest);

	/**
	 * @brief Increment 16-bit register
	 * @param dest
	 */
	void INC_r16(Sm83Register &dest);

	// DEC Intructions ---------------------------------------------

	/**
	 * @brief Decrement 8 bit register, Set zero flag if result is zero,
	 * Set half carry flag on bit 4 borrow, Set subtraction flag.
	 * @param dest
	 */
	void DEC_r8(uint8_t &dest);

	/**
	 * @brief Decrement 16 bit register.
	 * @param dest
	 */
	void DEC_r16(Sm83Register &dest);

	// RLCA, RRCA, RLA, RRA Instructions

	/**
	 * @brief Rotate accumulator left by 1. Clear zero, subtraction, and half carry flags.
	 * Carry flag set to bit that is rotated out.
	 */
	void RLCA();

	/**
	 * @brief Rotate accumulator right by 1. Clear zero, subtraction, and half cary flags.
	 * Carry flag set to bit that is rotated out.
	 */
	void RRCA();

	/**
	 * @brief Rotate accumulator left through carry flag.
	 * Clear zero, subtraction, and half carry flag. Carry flag set to
	 * shifted out bit.
	 */
	void RLA();

	/**
	 * @brief Rotate accumulator right through carry flag.
	 * Clear zero, subtraction, and half carry flag. Carry flag set to
	 * shifted out bit.
	 */
	void RRA();

	// ADD Instructions --------------------------------------------

	/**
	 * @brief Add 16-bit register into register HL. Set half carry on bit 11 overflow.
	 * Set Carry on bit 15 overflow. Clear subtraction flag.
	 * @param operand
	 */
	void ADD_HL_r16(Sm83Register &operand);

	// JR Instructions

	/**
	 * @brief Relative jump based on offset. The byte following the opcode
	 * is a signed 8-bit offset that determines where to jump to from the current
	 * program counter address
	 */
	void JR_i8();

	/**
	 * @brief Conditional relative jump based on offset. The byte following the opcode
	 * is a signed 8-bit offset that determines where to jump. Takes 2 m-cycles
	 * when jump is not taken, else 3 m-cycles.
	 * @param condition
	 */
	void JR_CC_i8(bool condition);
};
