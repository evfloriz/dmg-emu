#include "Bus.h"

Bus::Bus() {
	// Clear RAM
	for (auto& i : ram) {
		i = 0x00;
	}

	// Connect CPU to bus
	cpu.connectBus(this);
}

Bus::~Bus() {
}

void Bus::write(uint16_t addr, uint8_t data) {
	// 8KB is 2^13 bytes, so need 13 bits to address it
	// Max address is 16 * 16 * 16 * 2
	if (addr >= 0x0000 && addr <= 0xFFFF) {
		ram[addr] = data;
	}
}

uint8_t Bus::read(uint16_t addr) {
	if (addr >= 0x0000 && addr <= 0xFFFF) {
		return ram[addr];
	}

	return 0x00;
}
