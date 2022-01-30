#include "CPU.h"

#include "Bus.h"
#include <iostream>

CPU::CPU() {
	lookup = {		
		// 16-bit loads
		{0x01, {&CPU::LD_r16,		3,	&b, &c}},
		{0x11, {&CPU::LD_r16,		3,	&d, &e}},
		{0x21, {&CPU::LD_r16,		3,	&h, &l}},
		{0x31, {&CPU::LD_SP,		3}},
		{0x08, {&CPU::LD_a16_SP,	5}},
		{0xF8, {&CPU::LD_HL_SP_o8,	3}},
		{0xF9, {&CPU::LD_SP,		2, &h, &l}},

		{0xC1, {&CPU::POP_r16,		3,	&b,	&c}},		{0xC5, {&CPU::PUSH_r16,		3,	&b, &c}},
		{0xD1, {&CPU::POP_r16,		3,	&d, &e}},		{0xD5, {&CPU::PUSH_r16,		3,	&d, &e}},
		{0xE1, {&CPU::POP_r16,		3,	&h, &l}},		{0xE5, {&CPU::PUSH_r16,		3,	&h, &l}},
		{0xF1, {&CPU::POP_r16,		3,	&a, &f}},		{0xF5, {&CPU::PUSH_r16,		3,	&a, &f}},

		// 8-bit loads
		{0x06, {&CPU::LD_r8_r8,		2,	&b}},			{0x0E, {&CPU::LD_r8_r8,		2,	&c}},
		{0x40, {&CPU::LD_r8_r8,		1,	&b, &b}},		{0x48, {&CPU::LD_r8_r8,		1,	&c, &b}},
		{0x41, {&CPU::LD_r8_r8,		1,	&b, &c}},		{0x49, {&CPU::LD_r8_r8,		1,	&c, &c}},
		{0x42, {&CPU::LD_r8_r8,		1,	&b, &d}},		{0x4A, {&CPU::LD_r8_r8,		1,	&c, &d}},
		{0x43, {&CPU::LD_r8_r8,		1,	&b, &e}},		{0x4B, {&CPU::LD_r8_r8,		1,	&c, &e}},
		{0x44, {&CPU::LD_r8_r8,		1,	&b,	&h}},		{0x4C, {&CPU::LD_r8_r8,		1,	&c,	&h}},
		{0x45, {&CPU::LD_r8_r8,		1,	&b, &l}},		{0x4D, {&CPU::LD_r8_r8,		1,	&c, &l}},
		{0x46, {&CPU::LD_r8_p16,	2,	&b, &h, &l}},	{0x4E, {&CPU::LD_r8_p16,	2,	&c, &h, &l}},
		{0x47, {&CPU::LD_r8_r8,		1,	&b, &a}},		{0x4F, {&CPU::LD_r8_r8,		1,	&c, &a}},

		{0x16, {&CPU::LD_r8_r8,		2,	&d}},			{0x1E, {&CPU::LD_r8_r8,		2,	&e}},
		{0x50, {&CPU::LD_r8_r8,		1,	&d, &b}},		{0x58, {&CPU::LD_r8_r8,		1,	&e, &b}},
		{0x51, {&CPU::LD_r8_r8,		1,	&d, &c}},		{0x59, {&CPU::LD_r8_r8,		1,	&e, &c}},
		{0x52, {&CPU::LD_r8_r8,		1,	&d, &d}},		{0x5A, {&CPU::LD_r8_r8,		1,	&e, &d}},
		{0x53, {&CPU::LD_r8_r8,		1,	&d, &e}},		{0x5B, {&CPU::LD_r8_r8,		1,	&e, &e}},
		{0x54, {&CPU::LD_r8_r8,		1,	&d,	&h}},		{0x5C, {&CPU::LD_r8_r8,		1,	&e,	&h}},
		{0x55, {&CPU::LD_r8_r8,		1,	&d, &l}},		{0x5D, {&CPU::LD_r8_r8,		1,	&e, &l}},
		{0x56, {&CPU::LD_r8_p16,	2,	&d, &h, &l}},	{0x5E, {&CPU::LD_r8_p16,	2,	&e, &h, &l}},
		{0x57, {&CPU::LD_r8_r8,		1,	&d, &a}},		{0x5F, {&CPU::LD_r8_r8,		1,	&e, &a}},

		{0x26, {&CPU::LD_r8_r8,		2,	&h}},			{0x2E, {&CPU::LD_r8_r8,		2,	&l}},
		{0x60, {&CPU::LD_r8_r8,		1,	&h, &b}},		{0x68, {&CPU::LD_r8_r8,		1,	&l, &b}},
		{0x61, {&CPU::LD_r8_r8,		1,	&h, &c}},		{0x69, {&CPU::LD_r8_r8,		1,	&l, &c}},
		{0x62, {&CPU::LD_r8_r8,		1,	&h, &d}},		{0x6A, {&CPU::LD_r8_r8,		1,	&l, &d}},
		{0x63, {&CPU::LD_r8_r8,		1,	&h, &e}},		{0x6B, {&CPU::LD_r8_r8,		1,	&l, &e}},
		{0x64, {&CPU::LD_r8_r8,		1,	&h,	&h}},		{0x6C, {&CPU::LD_r8_r8,		1,	&l,	&h}},
		{0x65, {&CPU::LD_r8_r8,		1,	&h, &l}},		{0x6D, {&CPU::LD_r8_r8,		1,	&l, &l}},
		{0x66, {&CPU::LD_r8_p16,	2,	&h, &h, &l}},	{0x6E, {&CPU::LD_r8_p16,	2,	&l, &h, &l}},
		{0x67, {&CPU::LD_r8_r8,		1,	&h, &a}},		{0x6F, {&CPU::LD_r8_r8,		1,	&l, &a}},

		{0x02, {&CPU::LD_p16_r8,	2,	&b, &c, &a}},	{0x0A, {&CPU::LD_r8_p16,	2,	&a, &b, &c}},
		{0x12, {&CPU::LD_p16_r8,	2,	&d, &e, &a}},	{0x1A, {&CPU::LD_r8_p16,	2,	&a, &d, &e}},
		{0x22, {&CPU::LD_HLI_A,	2,	&h, &l, &a}},		{0x2A, {&CPU::LD_A_HLI,	2,	&a, &h, &l}},	// these don't actually need operands
		{0x32, {&CPU::LD_HLD_A,	2,	&h, &l, &a}},		{0x3A, {&CPU::LD_A_HLD,	2,	&a, &h, &l}},
		
		{0x36, {&CPU::LD_p16_r8,	3,	&h, &l}},		{0x3E, {&CPU::LD_r8_r8,		2,	&a}},
		{0x70, {&CPU::LD_p16_r8,	2,	&h, &l, &b}},	{0x78, {&CPU::LD_r8_r8,		1,	&a, &b}},
		{0x71, {&CPU::LD_p16_r8,	2,	&h, &l, &c}},	{0x79, {&CPU::LD_r8_r8,		1,	&a, &c}},
		{0x72, {&CPU::LD_p16_r8,	2,	&h, &l, &d}},	{0x7A, {&CPU::LD_r8_r8,		1,	&a, &d}},
		{0x73, {&CPU::LD_p16_r8,	2,	&h, &l, &e}},	{0x7B, {&CPU::LD_r8_r8,		1,	&a, &e}},
		{0x74, {&CPU::LD_p16_r8,	2,	&h, &l, &h}},	{0x7C, {&CPU::LD_r8_r8,		1,	&a, &h}},
		{0x75, {&CPU::LD_p16_r8,	2,	&h, &l, &l}},	{0x7D, {&CPU::LD_r8_r8,		1,	&a, &l}},
		/*{0x76, {&CPU::HALT,		1}},*/				{0x7E, {&CPU::LD_r8_p16,	2,	&a, &h, &l}},
		{0x77, {&CPU::LD_p16_r8,	2,	&h, &l, &a}},	{0x7F, {&CPU::LD_r8_r8,		1,	&a, &a}},

		{0xE0, {&CPU::LDH_p8_A,		3}},				{0xE2, {&CPU::LDH_p8_A,		2,	&c}},
		{0xF0, {&CPU::LDH_A_p8,		3}},				{0xE2, {&CPU::LDH_A_p8,		2,	&c}},
		{0xEA, {&CPU::LD_a16_A,		4}},
		{0xFA, {&CPU::LD_A_a16,		4}},

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

bool CPU::halfCarryPredicate(uint16_t val1, uint16_t val2) {
	// half carry
	// if the bottom 4 bits of each added together sets an upper 4 bit
	return (val1 & 0xF + val2 & 0xF) & 0x10;
}

bool CPU::carryPredicate(uint16_t val1, uint16_t val2) {
	// carry
	// if the bottom 8 bits of each added together sets an upper 8 bit
	return (val1 & 0xFF + val2 & 0xFF) & 0x100;
}

uint8_t CPU::LD_r16() {
	// load 16-bit register
	// d16 - immediate little endian 16 bit data
	// eg LD BC, d16

	// todo: should I check for valid input?

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	*(lookup[opcode].op1) = hi;
	*(lookup[opcode].op2) = lo;

	// apparently b, d, and h hold the more significant values
	// todo: double check this, im not sure why endianness would change
	// https://stackoverflow.com/questions/21639597/z80-register-endianness

	return 0;
}

uint8_t CPU::LD_SP() {
	// load stack pointer with (op2, op3) if provided or immediate 16-bit data if not
	// eg LD SP, HL

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	if (op1 && op2) {
		// load hl into sp
		sp = (*op1 << 8) | *op2;
	}
	else {
		// get immediate
		uint8_t lo = bus->read(pc);
		pc++;
		uint8_t hi = bus->read(pc);
		pc++;

		// combine and store
		sp = (hi << 8) | lo;
	}

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

uint8_t CPU::LD_HL_SP_o8() {
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
	setFlag(H, halfCarryPredicate(sp, offset));
	setFlag(C, carryPredicate(sp, offset));
	
	return 0;
}

// B 8-bit loads

uint8_t CPU::LD_r8_r8() {
	// load an 8 bit register or data into an 8 bit register (op2 = null means data)
	// eg LD B, B

	uint8_t *op1 = lookup[opcode].op1;
	uint8_t *op2 = lookup[opcode].op2;

	// todo: might need to check that op1 isn't null

	if (op2) {
		// todo: might need to guard against self assignment
		// todo: double check I can do this with pointers
		*op1 = *op2;
	}
	else {
		*op1 = bus->read(pc);
		pc++;
	}

	return 0;
}

uint8_t CPU::LD_r8_p16() {
	// load the value at the address pointed to by a 16-bit register into an 8-bit register
	// eg LD B, (HL)

	// set value of (hl) to register
	uint8_t hi = *(lookup[opcode].op2);
	uint8_t lo = *(lookup[opcode].op3);

	uint16_t addr = (hi << 8) | lo;
	*(lookup[opcode].op1) = bus->read(addr);

	return 0;
}

uint8_t CPU::LD_p16_r8() {
	// load the value of an 8-bit register into the address pointed to by a 16-bit register
	// eg LD (HL), B
	
	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;
	
	uint8_t hi = *(lookup[opcode].op1);
	uint8_t lo = *(lookup[opcode].op2);

	uint16_t addr = (hi << 8) | lo;

	// if op3 isn't null, use it as the data, otherwise read next value from pc
	uint8_t data;
	if (op3) {
		data = *op3;
	}
	else {
		data = bus->read(pc);
		pc++;
	}

	bus->write(addr, data);

	return 0;
}

uint8_t CPU::LD_HLI_A() {
	// eg LD (HL+), A

	// todo: can this overflow?

	uint16_t addr = (h << 8) | l;
	bus->write(addr, a);
	addr++;

	// convert back to h and l
	// idea: function to convert to and from hl and address
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LD_HLD_A() {
	// eg LD (HL-), A

	// todo: can this underflow?
	
	uint16_t addr = (h << 8) | l;
	bus->write(addr, a);
	addr--;

	// convert back to h and l
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LD_A_HLI() {
	// eg LD A, (HL+)

	// note: these are guaranteed to be a, h, and l
	// todo: can this overflow?
	uint16_t addr = (h << 8) | l;
	a = bus->read(addr);
	addr++;

	// convert back to h and l
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LD_A_HLD() {
	// eg LD A, (HL+)

	// todo: can this underflow?

	uint16_t addr = (h << 8) | l;
	a = bus->read(addr);
	addr--;

	// convert back to h and l
	l = addr & 0xFF;
	h = (addr >> 8) & 0xFF;

	return 0;
}

uint8_t CPU::LDH_A_p8() {
	// load A with 0xFF00 page byte from register or address
	// eg LD A, [C]
	// op1 = &c for [C] or null for a8

	uint8_t* op1 = lookup[opcode].op1;

	uint8_t offset;

	if (op1) {
		// load from register
		offset = *op1;
	}
	else {
		// load from immediate address
		offset = bus->read(pc);
		pc++;
	}

	uint16_t addr = 0xFF00 + offset;
	
	a = bus->read(addr);

	return 0;
}

uint8_t CPU::LDH_p8_A() {
	// load 0xFF00 page byte from register or address with A
	// eg LD [C], A
	// op1 = &c for [C] or null for a8

	uint8_t* op1 = lookup[opcode].op1;

	uint8_t offset;

	if (op1) {
		// load from register
		offset = *op1;
	}
	else {
		// load from immediate address
		offset = bus->read(pc);
		pc++;
	}

	uint16_t addr = 0xFF00 + offset;

	// write A to address
	bus->write(addr, a);

	return 0;
}

uint8_t CPU::LD_a16_A() {
	// load A into a16 location
	// a16 - little endian 16 bit address
	// 3 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	// combine
	uint16_t addr = (hi << 8) | lo;

	bus->write(addr, a);

	return 0;
}

uint8_t CPU::LD_A_a16() {
	// load data at a16 location into A
	// a16 - little endian 16 bit address
	// 3 machine cycles
	// no flags

	uint8_t lo = bus->read(pc);
	pc++;
	uint8_t hi = bus->read(pc);
	pc++;

	// combine
	uint16_t addr = (hi << 8) | lo;

	a = bus->read(addr);

	return 0;
}

uint8_t CPU::PUSH_r16() {
	// push register onto the stack
	// eg PUSH BC

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	sp--;
	bus->write(sp, *op1);
	sp--;
	bus->write(sp, *op2);

	return 0;
}

uint8_t CPU::POP_r16() {
	// load the value of an address onto the stack into the address pointed to by a 16-bit register
	// eg LD (HL), B

	uint8_t* op1 = lookup[opcode].op1;
	uint8_t* op2 = lookup[opcode].op2;

	*op2 = bus->read(sp);
	sp++;
	*op1 = bus->read(sp);
	sp++;

	return 0;

}

uint8_t CPU::ADD() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;
	
	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op1 << 8) | *op2;
		val2 = bus->read(addr);			
	}
	else {
		// add from register
		val2 = *op2;
	}

	// add and load into a (8-bit)
	*op1 = (val1 + val2) & 0xFF;
	
	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(val1, val2));
	setFlag(C, carryPredicate(val1, val2));

	return 0;
}

uint8_t CPU::ADC() {
	uint8_t* op1 = lookup[opcode].op1;		// should always be a
	uint8_t* op2 = lookup[opcode].op2;
	uint8_t* op3 = lookup[opcode].op3;

	// upcast to 16-bit for easier addition
	uint16_t val1 = *op1;
	uint16_t val2 = 0x0000;
	
	// add the carry bit
	uint16_t carry = getFlag(FLAGS::C) ? 0x0001 : 0x0000;

	if (op3) {
		// add from address
		uint16_t addr = (*op1 << 8) | *op2;
		val2 = bus->read(addr);
	}
	else {
		// add from register
		val2 = *op2;
	}

	// add with carry and load into a (8-bit)
	*op1 = (val1 + val2 + carry) & 0xFF;

	// set flags
	setFlag(Z, (*op1 == 0));
	setFlag(N, 0);
	setFlag(H, halfCarryPredicate(val1, val2));
	setFlag(C, carryPredicate(val1, val2));

	return 0;
}