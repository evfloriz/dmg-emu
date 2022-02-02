#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

class Bus;

class CPU {
public:
	CPU();
	~CPU();

public:
	// CPU core registers
	uint8_t a = 0x00;	// accumulator
	uint8_t f = 0x00;	// flags
	uint8_t b = 0x00;
	uint8_t c = 0x00;	// bc
	uint8_t d = 0x00;
	uint8_t e = 0x00;	// de
	uint8_t h = 0x00;
	uint8_t l = 0x00;	// hl

	// interrupt flag
	uint8_t IME = 0x00;
	
	// interrupt registers
	uint8_t IE = 0x00;
	uint8_t IF = 0x00;

	// value at memory pointed to by hl can be used in place of any register in any instruction
	
	uint16_t sp = 0x0000;	// stack pointer
	uint16_t pc = 0x0000;	// program counter

	// External event functions
	// todo: double check that these are the same on the dmg cpu
	//void reset();
	//void irq();
	//void nmi();
	void clock();

	// interrupt model
	// just to different locations preset for interrupts at the beginning of ram (irq)
	// software interrupts as well, 0x00 is reset

	// Signal that instruction is complete
	bool complete();

	// Link CPU to bus
	void connectBus(Bus* n) { bus = n; };

	// Memory map of strings for disassembly
	// todo: figure out if I should do this

	// Status register flags
	enum FLAGS {
		Z = (1 << 7),	// zero flag
		N = (1 << 6),	// subtraction flag (likely not needed)
		H = (1 << 5),	// half carry flag (likely not needed)
		C = (1 << 4)	// carry flag
	};

private:
	// Helper functions to access status register
	uint8_t getFlag(FLAGS flag);
	void setFlag(FLAGS flag, bool value);
	
	bool halfCarryPredicate(uint16_t val1, uint16_t val2);
	bool carryPredicate(uint16_t val1, uint16_t val2);
	bool halfBorrowPredicate(uint16_t val1, uint16_t val2);
	bool borrowPredicate(uint16_t val1, uint16_t val2);

	bool halfCarryPredicate16(uint16_t val1, uint16_t val2);
	bool carryPredicate16(uint16_t val1, uint16_t val2);
	
	bool checkCondition(uint8_t cc);

	// Helper variables
	uint8_t cycles = 0;
	uint8_t opcode = 0x00;
	uint8_t cb_opcode = 0x00;
	uint16_t addr_abs = 0x0000;
	bool set_ime = false;
	bool pending_ime = false;
	
	// Enum class for condition codes
	// using an enum class for scope (to prevent interference with FLAGS) since I don't
	// need bitwise operations with them
	enum CONDITION {
		c_N,		// none
		c_Z,		// zero flag set
		c_NZ,	// zero flag not set
		c_C,		// carry flag set
		c_NC		// carry flag not set
	};

	// Facilitate link to bus
	Bus* bus = nullptr;
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	// Fetch data
	//uint8_t fetch();

	struct INSTRUCTION {
		uint8_t(CPU::* operate)(void) = nullptr;
		//uint8_t (CPU::* addrmode)(void) = nullptr;
		uint8_t cycles = 0;
		uint8_t* op1 = nullptr;
		uint8_t* op2 = nullptr;
		uint8_t* op3 = nullptr;
	};

	// lookup tables for opcodes
	std::map<uint8_t, INSTRUCTION> lookup;
	std::map<uint8_t, INSTRUCTION> cb_lookup;

private:
	// Addressing modes

private:
	// Opcodes

	// r8 - 8-bit register, or data if no operand
	// r16 - 16-bit register
	// d16 - 16 bits of data
	// p16 - address pointed to by a 16-bit register
	// p8 - address pointed to by 8-bit register, OR immediate 8-bit address (for LDH)
	// a16 - immediate 16-bit address
	// o8 - 8-bit signed offset

	// todo: simplify 
	
	uint8_t LD_r16();
	uint8_t LD_SP();

	uint8_t LD_a16_SP();
	uint8_t LD_HL_SP_o8();
	
	uint8_t LD_r8_r8();
	uint8_t LD_r8_p16();
	uint8_t LD_p16_r8();
	
	uint8_t LD_HLI_A();
	uint8_t LD_HLD_A();
	uint8_t LD_A_HLI();
	uint8_t LD_A_HLD();

	uint8_t LDH_A_p8();
	uint8_t LDH_p8_A();
	uint8_t LD_a16_A();
	uint8_t LD_A_a16();

	uint8_t PUSH_r16();
	uint8_t POP_r16();

	uint8_t ADD();
	uint8_t ADC();
	uint8_t SUB();
	uint8_t SBC();

	uint8_t ADD_r16();
	uint8_t ADD_SP();
	uint8_t ADD_SP_o8();
	
	uint8_t INC_r16();
	uint8_t INC_SP();
	uint8_t DEC_r16();
	uint8_t DEC_SP();

	uint8_t AND();
	uint8_t XOR();
	uint8_t OR();
	uint8_t CP();

	uint8_t INC();
	uint8_t DEC();

	uint8_t DAA();
	uint8_t SCF();
	uint8_t CPL();
	uint8_t CCF();

	uint8_t JP();
	uint8_t JR();
	uint8_t CALL();
	uint8_t RET();
	uint8_t RETI();
	uint8_t RST();

	uint8_t NOP();
	uint8_t STOP();
	uint8_t HALT();
	
	uint8_t EI();
	uint8_t DI();

	uint8_t CB();

	uint8_t BIT();
	uint8_t RES();
	uint8_t SET();
	
	uint8_t RLCA();
	uint8_t RLA();
	uint8_t RRCA();
	uint8_t RRA();

	uint8_t RLC();
	uint8_t RL();
	uint8_t RRC();
	uint8_t RR();

	uint8_t SLA();
	uint8_t SRA();
	uint8_t SWAP();
	uint8_t SRL();

	void print_test();
};