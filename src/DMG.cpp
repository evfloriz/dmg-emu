#include <iostream>

#include "DMG.h"

DMG::DMG()
	: mmu(&cpu)
	, cpu(&mmu)
	, ppu(&mmu)
	, apu(&mmu) {}

bool DMG::init() {
	// Create cartridge
	cart = std::make_shared<Cartridge>(util::romPath);
	mmu.insertCartridge(cart);

	std::cout << "Beginning execution of " << util::romPath << std::endl;

	cpu.log_capture = false;
	cpu.log_file = util::logPath;

	// Initialize output file
	cpu.file = fopen(cpu.log_file.c_str(), "w");

	reset();

	return true;
}

bool DMG::reset() {
	// Reset LY
	mmu.directWrite(0xFF44, 0x00);

	// Reset divider and timer registers
	mmu.directWrite(0xFF04, 0xAB);
	mmu.directWrite(0xFF05, 0x00);
	mmu.directWrite(0xFF06, 0x00);
	mmu.directWrite(0xFF07, 0xF8);

	// Reset audio registers
	// Channel 1
	mmu.directWrite(0xFF10, 0x80);
	mmu.directWrite(0xFF11, 0xBF);
	//mmu.directWrite(0xFF12, 0xF3);	// This starts channel 1 with a volume of 15
	mmu.directWrite(0xFF12, 0x00);
	mmu.directWrite(0xFF13, 0xFF);
	mmu.directWrite(0xFF14, 0xBF);

	// Channel 2
	mmu.directWrite(0xFF16, 0x3F);
	mmu.directWrite(0xFF17, 0x00);
	mmu.directWrite(0xFF18, 0xFF);
	mmu.directWrite(0xFF19, 0xBF);

	// Channel 3
	mmu.directWrite(0xFF1A, 0x7F);
	mmu.directWrite(0xFF1B, 0xFF);
	mmu.directWrite(0xFF1C, 0x9F);
	mmu.directWrite(0xFF1D, 0xFF);
	mmu.directWrite(0xFF1E, 0xBF);

	// Channel 4
	mmu.directWrite(0xFF20, 0xFF);
	mmu.directWrite(0xFF21, 0x00);
	mmu.directWrite(0xFF22, 0x00);
	mmu.directWrite(0xFF23, 0xBF);

	return true;
}

bool DMG::tick() {
	do {
		cpu.clock();
		ppu.clock();
		apu.clock();
	} while (!cpu.complete());

	return true;
}

bool DMG::tickFrame() {
	do {
		tick();
	} while (!ppu.frameComplete);

	ppu.frameComplete = false;

	return true;
}
