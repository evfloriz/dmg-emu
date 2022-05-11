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
	}
	else if (addr == 0xFF04) {
		// If the divider is written to, set it to 0
		memory[addr] = 0x00;
	}
	else if (addr == 0xFF41) {
		// LCD STAT
		// Bottom 3 bits are read only
		uint8_t readOnlyBits = memory[addr] & 0x07;
		data &= 0xF8;

		memory[addr] = data | readOnlyBits;
	}
	else if (addr == 0xFF46) {
		// Early out if data is out of range
		if (data > 0xDF) {
			return;
		}

		// DMA transfer
		// Write data from 0xXX00-0xXX9F to 0xFE00-0xFE9F
		uint16_t dataStart = data * 0x100;
		uint16_t oamStart = 0xFE00;
		for (int i = 0; i < 0xA0; i++) {
			directWrite(oamStart + i, directRead(dataStart + i));
		}
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
		else if (addr == 0xFF00) {
			// Return the int that selectedButtons currently points to based on the previous write
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
	// Create mask for the bit at pos, then or with the value at pos
	uint8_t oldButtons = *buttons;
	*buttons &= ~(1 << pos);
	*buttons |= (value << pos);

	// If a bit went from high to low, request a joypad interrupt
	if (oldButtons > *buttons) {
		setBit(0xFF0F, 4, 1);
	}
}

void MMU::setBit(uint16_t addr, uint8_t pos, uint8_t value) {
	uint8_t data = memory[addr];
	data &= ~(1 << pos);
	data |= (value << pos);
	memory[addr] = data;
}
