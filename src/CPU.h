#pragma once

//#include <cstdint>

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
	
	uint16_t sp = 0x0000;	// stack pointer
	uint16_t pc = 0x0000;	// program counter

	// External event functions

	// Link CPU to bus
	void connectBus(Bus* n) { bus = n; };

	// Memory map of strings for disassembly - might not require

	// Status register flags
	enum FLAGS {
		Z = (1 << 7),	// zero flag
		N = (1 << 6),	// subtraction flag
		H = (1 << 5),	// half carry flag
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

	// Instruction structure

private:
	// Addressing modes

private:
	// Opcodes

};