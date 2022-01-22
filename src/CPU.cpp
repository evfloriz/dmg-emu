#include "CPU.h"

#include "Bus.h"
#include <iostream>

CPU::CPU() {
	/*lookup = {
		{"LDSP", &CPU::LDSP, 3}
	};*/

	lookup = {
		{0x31, {&CPU::LDSP, 3}}
	};
}

CPU::~CPU() {
}

void CPU::write(uint16_t addr, uint8_t data) {
	bus->write(addr, data);
}

uint8_t CPU::read(uint16_t addr) {
	return bus->read(addr);
}

void CPU::clock() {
	// If cycles remaining for an instruction is 0, read next byte
	if (cycles == 0) {
		opcode = read(pc);
		pc++;

		if (lookup.count(opcode)) {
			// Set cycles to number of cycles
			cycles = lookup[opcode].cycles;

			// Perform operation
			(this->*lookup[opcode].operate)();
		}
		else {
			printf("Unexpected opcode 0x%02x at 0x%04x\n", opcode, pc);
		}
	}
	cycles--;
}

bool CPU::complete() {
	return (cycles == 0);
}

uint8_t CPU::LDSP() {
	// load stack pointer
	// d16 - immediate little endian 16 bit data
	// 3 machine cycles
	// no flags

	// get data value
	// todo: should i increment this before opcode is called
	// looks like I probably should increment it in clock() since I always will have to
	//pc++;

	// get low and high values
	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	// combine
	uint16_t imm = (hi << 8) | lo;

	// store imm in stack pointer
	sp = bus->read(imm);

	// can this have additional cycles
	// I think it's only branch instructions that can
	return 0;
}