#include "MMU.h"

MMU::MMU() {
	// Set joypad register to high for now
	memory[0xFF00] = 0xFF;
}

MMU::~MMU() {
	delete[] memory;
}

void MMU::write(uint16_t addr, uint8_t data) {
	if (addr <= 0x7FFF) {
		// cartridge, invalid to write to
		return;
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) {
		// echo ram, prohibited
		return;
	}
	else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
		// unusable
		return;
	}
	else if (addr == 0xFF00) {
		// Set joypad input to read only for now
		return;
	}
	else {
		memory[addr] = data;
	}
}

uint8_t MMU::read(uint16_t addr) {	
	if (addr <= 0x7FFF) {
		// cartridge, fixed bank
		return cart->read(addr);
	}
	else {
		
		if (addr >= 0xE000 && addr <= 0xFDFF) {
			// echo ram, prohibited
			return 0xFF;
		}
		else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
			// unusable
			return 0xFF;
		}
		else {
			return memory[addr];
		}
	}
}

void MMU::directWrite(uint16_t addr, uint8_t data) {
	memory[addr] = data;
}

uint8_t MMU::directRead(uint16_t addr) {
	return memory[addr];
}

void MMU::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
	this->cart = cartridge;
}
