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

	// Helper variables
	uint8_t cycles = 0;
	uint8_t opcode = 0x00;
	uint16_t addr_abs = 0x0000;

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

private:
	// Addressing modes

private:
	// Opcodes

	// todo: original chunk, pleace rename
	uint8_t LD_SP_d16();
	uint8_t LD_HL_d16();
	uint8_t LD_BC_d16();
	uint8_t LD_DE_d16();

	uint8_t LD_a16_SP();
	uint8_t LD_HL_SP_r8();
	uint8_t LD_SP_HL();

	uint8_t LD_8();
	uint8_t LD_16();

	uint8_t LD_8_r16();
	
	uint8_t LD_HLI_A();
	uint8_t LD_HLD_A();
	uint8_t LD_A_HLI();
	uint8_t LD_A_HLD();

};