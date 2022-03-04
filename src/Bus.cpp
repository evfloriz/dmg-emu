#include "Bus.h"

Bus::Bus() {
	// Connect CPU to bus
	cpu.connectBus(this);

	// Connect PPU to bus
	ppu.connectBus(this);

	// Initialize data to 0
	vram.fill(0x00);
	externalRam.fill(0x00);
	wram.fill(0x00);
	oam.fill(0x00);
	ioRegisters.fill(0x00);
	hram.fill(0x00);
	ieRegister.fill(0x00);

	// Set joypad register to high for now
	ioRegisters[0x0000] = 0xFF;
}

Bus::~Bus() {
}

void Bus::write(uint16_t addr, uint8_t data) {
	if (addr >= 0x0000 && addr <= 0x7FFF) {
		// cartridge, invalid to write to
		return;
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		// video ram
		vram[addr - 0x8000] = data;
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		// external ram
		externalRam[addr - 0xA000] = data;
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		// wram
		wram[addr - 0xC000] = data;
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) {
		// echo ram, prohibited
		return;
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		// oam ram
		oam[addr - 0xFE00] = data;
	}
	else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
		// unusable
		return;
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		// io registers
		
		// Set joypad input to detect no button presses for now
		/*if (addr == 0xFF00) {
			data |= 0x0F;
		}*/
		
		// Set joypad input to read only for now
		if (addr == 0xFF00) {
			return;
		}

		ioRegisters[addr - 0xFF00] = data;
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		// hram
		hram[addr - 0xFF80] = data;
	}
	else if (addr == 0xFFFF) {
		// ie register
		ieRegister[addr - 0xFFFF] = data;
	}
}

uint8_t Bus::read(uint16_t addr) {
	if (addr >= 0x0000 && addr <= 0x7FFF) {
		// cartridge, fixed bank
		return cart->read(addr);
	}
	else if (addr >= 0x8000 && addr <= 0x9FFF) {
		// video ram
		return vram[addr - 0x8000];
	}
	else if (addr >= 0xA000 && addr <= 0xBFFF) {
		// external ram
		return externalRam[addr - 0xA000];
	}
	else if (addr >= 0xC000 && addr <= 0xDFFF) {
		// wram
		return wram[addr - 0xC000];
	}
	else if (addr >= 0xE000 && addr <= 0xFDFF) {
		// echo ram, prohibited
		return 0xFF;
	}
	else if (addr >= 0xFE00 && addr <= 0xFE9F) {
		// oam ram
		return oam[addr - 0xFE00];
	}
	else if (addr >= 0xFEA0 && addr <= 0xFEFF) {
		// unusable
		return 0xFF;
	}
	else if (addr >= 0xFF00 && addr <= 0xFF7F) {
		// io registers
		return ioRegisters[addr - 0xFF00];
	}
	else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		// hram
		return hram[addr - 0xFF80];
	}
	else if (addr == 0xFFFF) {
		// ie register
		return ieRegister[addr - 0xFFFF];
	}

	return 0xFF;
}

void Bus::insertCartridge(const std::shared_ptr<Cartridge>& cartridge) {
	this->cart = cartridge;
}

void Bus::clock() {
	// Execute one CPU clock cycle
	cpu.clock();

	ppu.clock();
}