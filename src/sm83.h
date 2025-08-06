#pragma once

#include <cstdint>
#include <string>

#include "bus.h"
#include "fmt/base.h"
#include "opcodeLogger.h"
#include "dmgBootrom.h"
#include "joypad.h"

class Sm83
{
	friend class Sm83JsonTest;

  public:
	struct Sm83FlagsRegister
	{
		unsigned char Z : 1; // bit 7, zero flag
		unsigned char N : 1; // bit 6, subtraction flag
		unsigned char H : 1; // bit 5, half carry flag
		unsigned char C : 1; // bit 4, carry flag

		/**
		 * @brief Set all at once flags with a single 8bit value as input.
		 */
		void setFlagsU8(uint8_t in)
		{
			Z = in >> 7;
			N = in >> 6;
			H = in >> 5;
			C = in >> 4;
		}

		/**
		 * @brief Returns all flags together as a single 8bit value
		 */
		uint8_t getFlagsU8() const
		{
			return static_cast<uint8_t>((Z << 7) | (N << 6) | (H << 5) | (C << 4));
		}
	};

	struct Sm83RegisterAF
	{
		uint8_t           accumulator; // accumulator
		Sm83FlagsRegister flags;       // cpu flags

		// return the AF register pair as a 16-bit unsigned int
		uint16_t get_u16() const
		{
			return static_cast<uint16_t>((accumulator << 8) | flags.getFlagsU8());
		}
	};

	// general purpose registers
	struct Sm83Register
	{
		uint8_t hi;
		uint8_t lo;

		void set_u16(uint16_t value)
		{
			lo = value & 0xFF;
			hi = (value & 0xFF00) >> 8;
		}

		// return general register pair as a 16-bit unsigned int
		uint16_t get_u16() const
		{
			return static_cast<uint16_t>((hi << 8) | lo);
		}
	};

	enum class InterruptVector
	{
		NONE   = 0,
		VBLANK = 0x40,
		STAT   = 0x48,
		TIMER  = 0x50,
		SERIAL = 0x58,
		JOYPAD = 0x60,
	};

	enum InterruptFlags
	{
		InterruptFlags_VBLANK = 1 << 0,
		InterruptFlags_LCD    = 1 << 1,
		InterruptFlags_TIMER  = 1 << 2,
		InterruptFlags_SERIAL = 1 << 3,
		InterruptFlags_JOYPAD = 1 << 4,
	};

	struct Sm83InterruptRegisters
	{
		bool    m_interruptMasterEnable        = false; // interrupt master enable
		bool    m_eiPending                    = false;
		uint8_t m_eiPendingElapsedInstructions = 0;

		uint8_t m_interruptEnable = 0; // control which interrupts are allowed to fire
		uint8_t m_interruptFlags  = 0; // controls which interrupt handlers are being requested

		bool interruptPending() const
		{
			return m_interruptEnable & m_interruptFlags;
		}

		void handle_ie_requests();
		void reset();
	};

	// https://gbdev.io/pandocs/Timer_and_Divider_Registers.html
	struct Sm83Timer
	{
	  private:
		uint16_t m_divider      = 0; // divider is used by the timer and apu
		uint8_t  m_timerCounter = 0;

	  public:
		// Bit 0-1: Clock select
		// Bit 2: timer counter increment enable
		uint8_t m_timerControl = 0;
		uint8_t m_timerModulo  = 0; // reload value for timerCounter

		void tick(uint8_t &interruptFlags);

		void setTimerCounter(uint8_t in)
		{
			m_timerCounter          = in;
			m_timerCounterIsWritten = true;
		}

		uint8_t getTimerCounter() const
		{
			return m_timerCounter;
		}

		// writing to the divider resets it to 0
		void setDivider()
		{
			m_divider = 0;
		}

		uint8_t getDivider() const
		{
			return static_cast<uint8_t>((m_divider & 0x3FC0) >> 6);
		}

		// retrive the full divider value which increments at 1048576 hz
		uint16_t getDividerFull() const
		{
			return m_divider;
		}

		void reset();

	  private:
		bool m_timerCounterIsWritten = false; // true when timerCounter(TIMA) register is written to
		bool m_reloadScheduled       = false; // true when timerCounter overflows from increment, meaning a reload and interrupt request is scheduled on the next m-cycle (next 4 t-cycles)

		bool m_prevStateDivider = false; // hold the prev state of the bit selected by timer control
		bool m_prevStateCounter = false; // hold the prev state

		enum TimerControlFlags
		{
			TAC_CLOCK_SELECT_0 = 0, // increment every 256 m-cycles
			TAC_CLOCK_SELECT_1 = 1, // increment every 4 m-cycles
			TAC_CLOCK_SELECT_2 = 2, // increment every 16 m-cycles
			TAC_CLOCK_SELECT_3 = 3, // increment every 64 m-cycles
			TAC_ENABLE         = 4, // timer counter increment enable
		};
	};

	DmgBootRom m_bootrom;
	uint8_t    m_bootRomDisable = 0;

	Joypad m_joypad;

	uint16_t       m_programCounter;
	uint16_t       m_stackPointer;
	Sm83RegisterAF m_registerAF;
	Sm83Register   m_registerBC;
	Sm83Register   m_registerDE;
	Sm83Register   m_registerHL;

	Sm83InterruptRegisters m_interrupts;
	Sm83Timer              m_timer;

	bool m_logEnable = false;

	OpcodeLogger m_opcodeLogger;

	Sm83(Bus &bus);

	void init(bool useBootrom);

	/**
	 * @brief Emulate cpu for a single instruction.
	 */
	void instructionStep();

  private:
	Bus &m_bus;

	uint16_t opcodeOperand       = 0; // saves the 1 or 2 byte long operand of cpu instructions for logging purposes
	uint16_t computedJumpAddress = 0; // saves computed jump address for logging purposes

	bool m_isHalted = false;

	void formatToOpcodeString(const std::string &format, uint16_t arg);
	void formatToOpcodeString(const std::string &format);

	void initWithBootrom();
	void initWithoutBootrom();

	/**
	 * @brief Service any pending interrupt if the ime flag is set and the corresponding
	 * interrupt enable flag is set.
	 */
	void handleInterrupt();

	/**
	 * @brief Read single byte from memory currently pointed to by program counter
	 * and increment program counter
	 * @return
	 */
	uint8_t cpuFetch_u8();

	/**
	 * @brief Read two bytes from memory pointed to by program counter, takes into account little endian storage.
	 * @return
	 */
	uint16_t cpuFetch_u16();

	/**
	 * @brief Decodes and executes the input opcode.
	 */
	void decodeExecute(uint8_t opcode);

	/**
	 * @brief Decode execute 0xCB prefixed instructions.
	 *
	 * @param opcode
	 */
	void decodeExecutePrefixedMode(uint8_t opcode);

	// -------------------------------------------- SM83 CPU Intructions ------------------------------------------

	// r8  - means 8 bit register
	// r16 - means 16 bit register (8 bit register pair)
	// n8  - means immediate 8 bit data
	// n16 - means immediate 16 bit data
	// indirect - means reading from a memory address where address is a n16 immediate value or r16 register value
	// i8  - means signed 8 bit data
	// CC  - means some booleon condition, NZ: zero flag not set, Z: set flag set, NC: carry flag not set, C: carry flag
	// set

	// LDH Instructions -------------------------------------------------------

	/**
	 * @brief Load accumulator into address 0xFF00 + immediate 8-bit value.
	 */
	void LDH_indirect_n8_A();

	/**
	 * @brief Load value at address 0xFF00 + 8-bit immediate value into
	 * accumulator.
	 */
	void LDH_A_indirect_n8();

	/**
	 * @brief Load accumulator into address 0xFF00 + 8-bit C register.
	 */
	void LDH_indirect_C_A();

	/**
	 * @brief Load value at address 0xFF00 + 8-bit C register into accumulator.
	 */
	void LDH_A_indirect_C();

	// LD Instructions --------------------------------------------------------

	/**
	 * @brief Load HL register into stack pointer.
	 */
	void LD_SP_HL();

	/**
	 * @brief Add signed immediate 8-bit value to stack pointer, store result into
	 * HL register. Clear zero & subtraction flag. Set half carry on bit 3 overflow,
	 * Set carry on bit 7 overflow.
	 */
	void LD_HL_SP_i8();

	/**
	 * @brief Load 8-bit register with immediate 8-bit value
	 */
	void LD_r8_n8(uint8_t &dest);

	/**
	 * @brief Load 8-bit registers with value in another 8-bit register.
	 *
	 * @param dest
	 */
	void LD_r8_r8(uint8_t &dest, uint8_t &src);

	/**
	 * @brief Load 16-bit register with immediate 16-bit value (next 2 bytes following opcode)
	 * @param dest
	 */
	void LD_r16_n16(Sm83Register &dest);

	/**
	 * @brief Load stack pointer with immediate 16-bit value
	 */
	void LD_SP_n16();

	/**
	 * @brief Load address pointed to by register pair dest with value from 8bit register.
	 * @param dest
	 */
	void LD_indirect_r16_r8(Sm83Register &dest, uint8_t &src);

	/**
	 * @brief Load accumulator into memory pointed to by immediate 16-bit value.
	 */
	void LD_indirect_n16_A();

	/**
	 * @brief Load value pointed to in memory by immediate 16-bit address.
	 */
	void LD_A_indirect_n16();

	/**
	 * @brief Load 8 bit register with value at memory address location in 16 bit register.
	 * @param src
	 */
	void LD_A_indirect_r16(Sm83Register &src);

	/**
	 * @brief Load address pointed to by HL register with 8-bit immediate value.
	 */
	void LD_indirect_HL_n8();

	/**
	 * @brief Load 8bit register with value pointed to by HL register.
	 */
	void LD_r8_indirect_HL(uint8_t &dest);

	/**
	 * @brief Load address pointed to by register HL with value in accumulator.
	 * Increment HL register after load.
	 */
	void LD_indirect_HLI_A();

	/**
	 * @brief Load address pointed to by register HL with value in accumulator.
	 * Decrement HL register after load.
	 */
	void LD_indirect_HLD_A();

	/**
	 * @brief Load accumualtor with value at memory address in HL register
	 * Increments HL register.
	 */
	void LD_A_indirect_HLI();

	/**
	 * @brief Load accumulator with value at memory address in HL register.
	 * Decrements HL register.
	 */
	void LD_A_indirect_HLD();

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

	/**
	 * @brief Increment stack pointer
	 */
	void INC_SP();

	/**
	 * @brief Increment value pointed by HL register
	 * Set half carry flag on bit 3 overflow, Clear subtraction flag.
	 */
	void INC_indirect_HL();

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

	/**
	 * @brief Decrement value pointed to by HL register.
	 */
	void DEC_indirect_HL();

	/**
	 * @brief Decrement stack pointer.
	 */
	void DEC_SP();

	// Rotate (Bit-shift) Instructions -------------------------------------------

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

	/**
	 * @brief Rotate 8-bit register left. Set zero on 0 result. Clear subtraction,
	 * half carry flag. Set carry to rotated out bit.
	 *
	 * @param dest
	 */
	void RLC_r8(uint8_t &dest);

	/**
	 * @brief Left rotate value pointed to by HL register. Flags affected same way
	 * as RLC_r8.
	 */
	void RLC_indirect_HL();

	/**
	 * @brief Rotate  8-bit register right. Set zero flag on 0 result. Clear subtraction
	 * and half carry flags. Set carry flag to rotated out bit.
	 *
	 * @param dest
	 */
	void RRC_r8(uint8_t &dest);

	/**
	 * @brief Same as RRC_r8, but rotate value in memory pointed to by HL register.
	 */
	void RRC_indirect_HL();

	/**
	 * @brief Rotate 8-bit register left through carry. Set zero on 0 result,
	 * clear subtraction and half carry. Carry set to rotated out bit.
	 *
	 * @param dest
	 */
	void RL_r8(uint8_t &dest);

	/**
	 * @brief Same as RL_r8, except it operates on value in memory pointed to
	 * by HL registers.
	 */
	void RL_indirect_HL();

	/**
	 * @brief Rotate  8-bit register right through carry. Set zero on 0 result.
	 * Clear subtraction & half carry. Carry set to rotated out bit.
	 *
	 * @param dest
	 */
	void RR_r8(uint8_t &dest);

	/**
	 * @brief Same as RR_r8, except operates on value from memory pointed to by
	 * HL register.
	 */
	void RR_indirect_HL();

	// SLA, SRA, SRL Instructions -------------------------------------------------

	/**
	 * @brief Arithmetic left shift 8-bit register.
	 * Set zero flag on 0 result, Clear subtraction, half carry. Set carry to shifted
	 * out bit.
	 *
	 * @param dest
	 */
	void SLA_r8(uint8_t &dest);

	/**
	 * @brief Same as SLA_r8, operate on byte in memory pointed to by HL register.
	 */
	void SLA_indirect_HL();

	/**
	 * @brief Arithmetic right shift 8-bit register. Bit 7 of register
	 * is not shifted. Set zero flag on 0 result. Clear
	 * subtraction & half carry. Set carry to shifted out bit.
	 *
	 * @param dest
	 */
	void SRA_r8(uint8_t &dest);

	/**
	 * @brief Same as SRA_r8, operate on value from memory pointed to by HL register.
	 */
	void SRA_indirect_HL();

	/**
	 * @brief Logical right shift on 8-bit register. Set zero flag if result 0.
	 * Clear half carry and subtraction flags. Set carry to shifted out bit.
	 *
	 * @param dest
	 */
	void SRL_r8(uint8_t &dest);

	void SRL_indirect_HL();

	// SWAP Instructions

	/**
	 * @brief Swap upper 4 bits with lower 4 bits of 8-bit register.
	 * Set zero flag on 0 result. Clear subtraction, half carry and carry.
	 *
	 * @param dest
	 */
	void SWAP_r8(uint8_t &dest);

	void SWAP_indirect_HL();

	// ADD Instructions -----------------------------------------------------------

	/**
	 * @brief Add signed 8-bit immediate value into stack pointer.
	 * Clear zero, subtraction flag. Set half carry on bit 3 overflow.
	 * Set carry on bit 7 overflow.
	 */
	void ADD_SP_i8();

	/**
	 * @brief Add 16-bit register into register HL. Set half carry on bit 11 overflow.
	 * Set Carry on bit 15 overflow. Clear subtraction flag.
	 * @param operand
	 */
	void ADD_HL_r16(Sm83Register &operand);

	/**
	 * @brief Add 16-bit register into register HL. Set half carry on bit 11 overflow.
	 * Set Carry on bit 15 overflow. Clear subtraction flag.
	 */
	void ADD_HL_SP();

	/**
	 * @brief Add 8-bit register to accumulator.Set zero flag if result is 0.
	 * Clear subtraction flag. Set half carry on bit 3 overflow, Set carry on
	 * bit 7 overflow.
	 */
	void ADD_A_r8(uint8_t &operand);

	/**
	 * @brief Add 8-bit immediate value into accumulator. Set flags same way
	 * as ADD_A_r8.
	 */
	void ADD_A_n8();

	/**
	 * @brief Add byte pointed to by HL registers into accumulator.
	 * Set zero flag if result is zero. Clear subtraction flag. Set half
	 * carry on bit 3 overflow, Set carry flag on bit 7 overflow.
	 */
	void ADD_A_indirect_HL();

	// ADC Instructions ----------------------------------------------------------

	/**
	 * @brief Add 8-bit register plus carry into accumulator. Set zero flag
	 * if result is zero. Clear subtraction flag. Set half carry on bit 3 overflow,
	 * Set carry on bit 7 overflow.
	 *
	 * @param operand
	 */
	void ADC_A_r8(uint8_t &operand);

	/**
	 * @brief Add value pointed to by HL register into accumulator. Flags are affected
	 * the same way as ADC_A_r8 instruction.
	 */
	void ADC_A_indirect_HL();

	/**
	 * @brief ADC with immediate 8-bit value
	 */
	void ADC_A_n8();

	// SUB Instructions -------------------------------------------------------

	/**
	 * @brief Subtract 8-bit register value from accumulator. Set zero flag if
	 * result is zero. Set subtraction flag. Set half carry on bit 4 borrow,
	 * set Carry on borrow.
	 *
	 * @param operand
	 */
	void SUB_A_r8(uint8_t &operand);

	/**
	 * @brief SUB with immediate 8-bit value
	 */
	void SUB_A_n8();

	/**
	 * @brief Subtract value pointed to by HL register from accumulators.
	 * Flags are affected the same way as SUB_A_r8 instruction.
	 */
	void SUB_A_indirect_HL();

	// SBC Instructions --------------------------------------------------------

	/**
	 * @brief Subtract 8-bit register and carry from accumulator.
	 * Set zero if result is zero. Set subtraction flag. Set half
	 * carry flag on bit-4 borrow. Set carry flag if subtraction
	 * results in borrow.
	 *
	 * @param operand
	 */
	void SBC_A_r8(uint8_t &operand);

	/**
	 * @brief SBC with 8-bit immediate value
	 */
	void SBC_A_n8();

	/**
	 * @brief Same as SBC_A_r8, except adding valuing pointed to by HL register
	 * into accumulator.
	 */
	void SBC_A_indirect_HL();

	// JR Instructions -----------------------------------------------------------

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

	// JP Instructions ------------------------------------------------------------

	/**
	 * @brief Jump to absolute address if boolean condition is met.
	 * 4 m-cycles on branch taken, else 3 m-cycles.
	 *
	 * @param condition
	 */
	void JP_CC_n16(bool condition);

	/**
	 * @brief Jump to absolute address.
	 */
	void JP_n16();

	void JP_HL();

	// DAA Instructions ------------------------------------------------

	/**
	 * @brief Adjusts the result in accumulator to be in valid binary decimal format.
	 * Indended to be used after arithmetic instructions.
	 * https://rgbds.gbdev.io/docs/v0.9.1/gbz80.7#DAA
	 */
	void DAA();

	// CPL Instructions -------------------------------------------------

	/**
	 * @brief Perform bitwise not on accumulator. Set subtraction and half carry flag.
	 */
	void CPL();

	// Flag Instructions -------------------------------------------------

	/**
	 * @brief Set carry flag. Clear subtraction and half carry flag.
	 */
	void SCF();

	/**
	 * @brief Bitwise not on carry flag. Clear subtraction and half carry flags.
	 */
	void CCF();

	// AND Instructions ------------------------------------------------------------

	/**
	 * @brief Bitwise and accumulator with 8-bit register. Set zero flag on 0 result.
	 * Clear subtraction and carry flag. Set half carry flag.
	 *
	 * @param operand
	 */
	void AND_A_r8(uint8_t &operand);

	/**
	 * @brief AND with immediate 8-bit register.
	 */
	void AND_A_n8();

	/**
	 * @brief Bitwise and accumulator with value pointed to by HL register.
	 * Flags affected the same way as AND_A_r8.
	 */
	void AND_A_indirect_HL();

	// XOR Instructions ----------------------------------------------------------------

	/**
	 * @brief Bitwise exclusive or accumulator and 8-bit register. Clears subtraction,
	 * half carry, and carry flags. Set zero flag on 0 result.
	 *
	 * @param operand
	 */
	void XOR_A_r8(uint8_t &operand);

	/**
	 * @brief XOR with immediate 8-bit value.
	 */
	void XOR_A_n8();

	/**
	 * @brief Bitwise exclusive or accumulator and value pointed to by HL registers.
	 * Flags affected the same way as XOR_A_r8
	 */
	void XOR_A_indirect_HL();

	// OR Instructions ------------------------------------------------------------------

	/**
	 * @brief Bitwise or accumulator with 8-bit register. Clear subtraction, half carry,
	 * and carry flags. Set zero flag on 0 result.
	 *
	 * @param operand
	 */
	void OR_A_r8(uint8_t &operand);

	/**
	 * @brief OR with immediate 8-bit value
	 */
	void OR_A_n8();

	/**
	 * @brief Bitwise or accumulator with value pointed to HL registers. Flags affected
	 * the same way as OR_A_r8.
	 */
	void OR_A_indirect_HL();

	// CP Instructions -----------------------------------------------------------------

	/**
	 * @brief Compare accumulator with 8-bit register by subtracting register value
	 * from accumulator, does not store result.
	 * Sets zero flag if result is zero. Set subtraction flag. Set half carry on bit 4 borrow,
	 * Set carry if borrow.
	 *
	 * @param operand
	 */
	void CP_A_r8(uint8_t &operand);

	/**
	 * @brief Compare accumulator with 8-bit immediate value. Flags affected same way as
	 * CP_A_r8
	 */
	void CP_A_n8();

	/**
	 * @brief Same as CP_A_r8 except it compares with value pointed to by HL register.
	 */
	void CP_A_indirect_HL();

	// RET, RETI Instructions -------------------------------------------------------------------

	/**
	 * @brief Return from subroutine by popping return address from the stack into the program
	 * counter.
	 */
	void RET();

	/**
	 * @brief Return from subroutine if boolean condition is met.
	 *
	 * @param condition
	 */
	void RET_CC(bool condition);

	/**
	 * @brief Return from subroutine and set flag to enable interupts.
	 */
	void RETI();

	// POP Instructions ---------------------------------------------------------------------

	/**
	 * @brief Pop 2 bytes from stack and store into register pair.
	 *
	 * @param dest
	 */
	void POP_r16(Sm83Register &dest);

	/**
	 * @brief Pop 16-bit AF registers from stack. Flags are set to the
	 * flags byte that is popped. This is the low byte of AF register.
	 */
	void POP_AF();

	// PUSH Instructions ---------------------------------------------------------------------

	/**
	 * @brief Push 16-bit register to stack, hi byte followed by lo byte.
	 *
	 * @param dest
	 */
	void PUSH_r16(Sm83Register &dest);

	/**
	 * @brief Push the AF register onto the stack.
	 */
	void PUSH_AF();

	// CALL Instructions ---------------------------------------------------------------------

	/**
	 * @brief Pushes address of next instruction after CALL onto the stack.
	 */
	void CALL_n16();

	/**
	 * @brief Call subroutine if boolean condition is met. 6 m-cycles if branch taken,
	 * Else 3 m-cycles.
	 *
	 * @param condition
	 */
	void CALL_CC_n16(bool condition);

	// RST Instructions

	/**
	 * @brief Fixed jump vectors for RST instruction.
	 */
	enum class RstVector
	{
		H00 = 0x0000,
		H08 = 0x0008,
		H10 = 0x0010,
		H18 = 0x0018,
		H20 = 0x0020,
		H28 = 0x0028,
		H30 = 0x0030,
		H38 = 0x0038,
	};

	/**
	 * @brief Subroutine call a fixed jump vector.
	 *
	 * @param vec
	 */
	void RST(RstVector vec);

	// BIT, RES Instructions ----------------------------------------------------------------------

	/**
	 * @brief Select 1 of 8 bits in a byte to operate on for bit flag instructions.
	 */
	enum class BitSelect
	{
		B0 = 0x01,
		B1 = 0x02,
		B2 = 0x04,
		B3 = 0x08,
		B4 = 0x10,
		B5 = 0x20,
		B6 = 0x40,
		B7 = 0x80,
	};

	/**
	 * @brief Test a bit in 8-bit register. Set half carry, clear subtraction flag.
	 * Set zero if tested bit is 0.
	 *
	 * @param b
	 * @param dest
	 */
	void BIT_r8(BitSelect b, uint8_t dest);

	void BIT_indirect_HL(BitSelect b);

	/**
	 * @brief Clear a bit in 8-bit register.
	 *
	 * @param b
	 * @param dest
	 */
	void RES_r8(BitSelect b, uint8_t &dest);

	/**
	 * @brief Clear a bit in value from memory pointed to by HL register.
	 *
	 * @param b
	 */
	void RES_indirect_HL(BitSelect b);

	/**
	 * @brief Set a bit in 8-bit register.
	 *
	 * @param b
	 * @param dest
	 */
	void SET_r8(BitSelect b, uint8_t &dest);

	/**
	 * @brief Set a bit in value from memory pointed to by register HL.
	 *
	 * @param b
	 */
	void SET_indirect_HL(BitSelect b);

	// EI, DI Instructions

	/**
	 * @brief Disables interrupts by clearing ime flag.
	 */
	void DI();

	/**
	 * @brief Enables interrupts by setting ime flag AFTER the intruction following
	 * EI.
	 */
	void EI();

	void HALT();
};
