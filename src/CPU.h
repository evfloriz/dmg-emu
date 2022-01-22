#pragma once

#include <cstdint>
#include <string>
#include <vector>

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
	void reset();
	void irq();
	void nmi();
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

	// Helper variables

	// Facilitate link to bus
	Bus* bus = nullptr;
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	// Fetch data
	uint8_t fetch();

	// todo:double check if this is the best structure
	// Instruction structure
	struct INSTRUCTION {
		std::string name;
		uint8_t (CPU::* operate)(void) = nullptr;
		uint8_t (CPU::* addrmode)(void) = nullptr;
		uint8_t cycles = 0;
	};

	// lookup tables for opcodes
	std::vector<INSTRUCTION> lookup;
	std::vector<INSTRUCTION> lookupCB;

private:
	// Addressing modes
	
	// Don't need I don't think. Unlike the 6502 which has 56 opcodes augmented by 13 addressing
	// modes, it looks like the dmg cpu just has a ton of opcodes instead.
	
	// Actually I think it still does, I think addressing modes are the general way of describing
	// if the data is immediate or what register it's loaded from etc. I'll have to think about a
	// solution tomorrow.

private:
	// Opcodes

};