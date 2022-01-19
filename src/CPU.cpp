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