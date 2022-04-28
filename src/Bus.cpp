#include "Bus.h"

Bus::Bus() {
	
	// Initialize data to 0
	ieRegister = 0x00;
	ifRegister = 0x00;
	timerControlRegister = 0x00;

	// Set joypad register to high for now
	//ioRegisters[0x0000] = 0xFF;
	memory[0xFF00 - 0x7FFF] = 0xFF;
}

Bus::~Bus() {
	delete[] memory;
}

void Bus::write(uint16_t addr, uint8_t data) {
	if (addr >= 0x0000 && addr <= 0x7FFF) {
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
	// Handle frequently used registers for timer and interrupts
	else if (addr == 0xFF07) {
		timerControlRegister = data;
	}
	else if (addr == 0xFF0F) {
		ifRegister = data;
	}	
	else if (addr == 0xFFFF) {
		// ie register
		ieRegister = data;
	}
	else {
		// hram
		memory[addr - 0x7FFF] = data;
	}
}

uint8_t Bus::read(uint16_t addr) {	
	uint8_t data = 0xFF;
	
	if (addr >= 0x0000 && addr <= 0x7FFF) {
		// cartridge, fixed bank
		data = cart->read(addr);
	}
	else {
		
		if (addr >= 0xE000 && addr <= 0xFDFF) {
			// echo ram, prohibited
			data = 0xFF;
		}
		else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
			// unusable
			data = 0xFF;
		}
		else if (addr == 0xFF07) {
			// Handle frequently used registers for timer and interrupts
			data = timerControlRegister;
		}
		else if (addr == 0xFF0F) {	
			data = ifRegister;
		}
		
		else if (addr == 0xFFFF) {
			// ie register
			data = ieRegister;
		}
		else {
			data = memory[addr - 0x7FFF];
		}
	}
	
	return data;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
	this->cart = cartridge;
}
