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
		// Change which set of buttons is selected to be read from
		if (~data & (1 << 5)) {
			selectedButtons = &actionButtons;
		}
		else if (~data & (1 << 4)) {
			selectedButtons = &directionButtons;
		}
		else {
			// Set selected buttons to off, always 0xFF
			// TODO: Double check that this is the correct behaviour.
			selectedButtons = &buttonsOff;
		}
		return;
	}
	else {
		memory[addr] = data;
		return;
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
		else if (addr == 0xFF00) {
			// Return the int that selected_buttons currently points to based on previous write
			return *selectedButtons;
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

void MMU::writeActionButton(uint8_t pos, uint8_t value) {
	writeButton(&actionButtons, pos, value);
}

void MMU::writeDirectionButton(uint8_t pos, uint8_t value) {
	writeButton(&directionButtons, pos, value);
}

void MMU::writeButton(uint8_t* buttons, uint8_t pos, uint8_t value) {
	// Create mask for pos, then or with value
	*buttons &= ~(1 << pos);
	*buttons |= (value << pos);
}