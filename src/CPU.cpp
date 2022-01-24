#include "CPU.h"

#include "Bus.h"
#include <iostream>

CPU::CPU() {
	lookup = {		
		// 16-bit loads
		{0x01, {&CPU::LD_BC_d16, 3}},
		{0x11, {&CPU::LD_DE_d16, 3}},
		{0x21, {&CPU::LD_HL_d16, 3}},
		{0x31, {&CPU::LD_SP_d16, 3}},
		{0x08, {&CPU::LD_a16_SP, 5}},
		{0xF8, {&CPU::LD_HL_SP_r8, 3}},
		{0xF9, {&CPU::LD_SP_HL, 2}},

		// 8-bit loads
		{0x06, {&CPU::LD_B_d8, 2}},
		{0x40, {&CPU::LD_B_B, 1}},
		{0x41, {&CPU::LD_B_C, 1}},
		{0x42, {&CPU::LD_B_D, 1}},
		{0x43, {&CPU::LD_B_E, 1}},
		{0x44, {&CPU::LD_B_H, 1}},
		{0x45, {&CPU::LD_B_L, 1}},
		{0x46, {&CPU::LD_B_HL, 2}},
		{0x47, {&CPU::LD_B_A, 1}},

		{0x0E, {&CPU::LD_C_d8, 2}},
		{0x48, {&CPU::LD_C_B, 1}},
		{0x49, {&CPU::LD_C_C, 1}},
		{0x4A, {&CPU::LD_C_D, 1}},
		{0x4B, {&CPU::LD_C_E, 1}},
		{0x4C, {&CPU::LD_C_H, 1}},
		{0x4D, {&CPU::LD_C_L, 1}},
		{0x4E, {&CPU::LD_C_HL, 2}},
		{0x4F, {&CPU::LD_C_A, 1}},
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

uint8_t CPU::getFlag(FLAGS flag) {
	// return true if f has a 1 in the position of flag
	return ((f & flag) > 0) ? 1 : 0;
}
void CPU::setFlag(FLAGS flag, bool value) {
	// set the position of the flag in f to 1 or 0
	if (value) {
		f |= flag;
	}
	else {
		f &= ~flag;
	}
}

uint8_t CPU::LD_BC_d16() {
	// load BC register
	// d16 - immediate little endian 16 bit data
	// 3 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	b = hi;
	c = lo;

	return 0;
}

uint8_t CPU::LD_DE_d16() {
	// load DE register
	// d16 - immediate little endian 16 bit data
	// 3 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	d = hi;
	e = lo;

	return 0;
}

uint8_t CPU::LD_HL_d16() {
	// load HL register
	// d16 - immediate little endian 16 bit data
	// 3 machine cycles
	// no flags

	// get low and high values
	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	// store lo in h and hi in l
	// apparently b, d, and h hold the more significant values
	// todo: double check this, im not sure why endianness would change
	// https://stackoverflow.com/questions/21639597/z80-register-endianness
	h = hi;
	l = lo;

	// can this have additional cycles
	// I think it's only branch instructions that can
	return 0;
}

uint8_t CPU::LD_SP_d16() {
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
	sp = imm;

	// can this have additional cycles
	// I think it's only branch instructions that can
	return 0;
}

uint8_t CPU::LD_a16_SP() {
	// load SP into a16 location
	// a16 - little endian 16 bit address
	// 5 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;
	
	// combine
	uint16_t addr = (hi << 8) | lo;

	// store stack pointer low in addr and high in addr+1
	uint8_t sp_lo = sp & 0xFF;
	uint8_t sp_hi = (sp >> 8) & 0xFF;
	
	bus->write(addr, sp_lo);
	bus->write(addr + 1, sp_hi);

	return 0;
}

uint8_t CPU::LD_HL_SP_r8() {
	// load sp with a signed offset into hl
	// 3 machine cycles
	// 0 0 H C
	
	// Get signed offset
	uint16_t offset = bus->read(pc);
	pc++;

	// If highest bit is negative sign, extend to higher 8 bits
	if (offset & 0x80) {
		offset |= 0xFF00;
	}

	// I think just add, doesn't matter if it extends the page at least for this load
	// since its not actually loading a value from memory other than the offset (not like a branch)
	uint16_t value = sp + offset;

	l = value & 0xFF;
	h = (value >> 8) & 0xFF;

	setFlag(Z, 0);
	setFlag(N, 0);
	
	// half carry
	// if the bottom 4 bits of each added together sets an upper 4 bit
	if ((sp & 0xF + offset & 0xF) & 0x10) {
		setFlag(H, 1);
	}
	else {
		setFlag(H, 0);
	}

	// carry
	// if the bottom 8 bits of each added together sets an upper 8 bit
	if ((sp & 0xFF + offset & 0xFF) & 0x100) {
		setFlag(C, 1);
	}
	else {
		setFlag(C, 0);
	}

	return 0;
}

uint8_t CPU::LD_SP_HL() {
	// load hl into sp
	// 2 machine cycles
	// no flags

	// simple as?
	sp = (h << 8) | l;

	return 0;
}

// B 8-bit loads

uint8_t CPU::LD_B_d8() {
	b = bus->read(pc);
	pc++;

	return 0;
}

uint8_t CPU::LD_B_B() {
	// todo: check if this is allowed, might need to guard against self assignment
	b = b;
	return 0;
}

uint8_t CPU::LD_B_C() {
	b = c;
	return 0;
}

uint8_t CPU::LD_B_D() {
	b = d;
	return 0;
}

uint8_t CPU::LD_B_E() {
	b = e;
	return 0;
}

uint8_t CPU::LD_B_H() {
	b = h;
	return 0;
}

uint8_t CPU::LD_B_L() {
	b = l;
	return 0;
}

uint8_t CPU::LD_B_HL() {
	// set value of (hl) to b
	uint16_t addr = (h << 8) | l;
	b = bus->read(addr);

	return 0;
}

uint8_t CPU::LD_B_A() {
	b = a;
	return 0;
}

// C 8-bit loads
// todo: combine these

uint8_t CPU::LD_C_d8() {
	c = bus->read(pc);
	pc++;

	return 0;
}

uint8_t CPU::LD_C_B() {
	// todo: check if this is allowed, might need to guard against self assignment
	c = b;
	return 0;
}

uint8_t CPU::LD_C_C() {
	c = c;
	return 0;
}

uint8_t CPU::LD_C_D() {
	c = d;
	return 0;
}

uint8_t CPU::LD_C_E() {
	c = e;
	return 0;
}

uint8_t CPU::LD_C_H() {
	c = h;
	return 0;
}

uint8_t CPU::LD_C_L() {
	c = l;
	return 0;
}

uint8_t CPU::LD_C_HL() {
	// set value of (hl) to b
	uint16_t addr = (h << 8) | l;
	c = bus->read(addr);

	return 0;
}

uint8_t CPU::LD_C_A() {
	c = a;
	return 0;
}