#include "CPU.h"

#include "Bus.h"

CPU::CPU() {
}

CPU::~CPU() {
}

void CPU::write(uint16_t addr, uint8_t data) {
	bus->write(addr, data);
}

uint8_t CPU::read(uint16_t addr) {
	return bus->read(addr);
}

uint8_t CPU::LDSP() {
	// load stack pointer
	// d16 - immediate little endian 16 bit data
	// 3 machine cycles
	// no flags

	// get data value
	// todo: should i increment this before opcode is called
	pc++;

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
	return 0;
}