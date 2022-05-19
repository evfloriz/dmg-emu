
#include <iostream>

#include "DMG.h"

DMG::DMG()
	: mmu()
	, cpu(&mmu)
	, ppu(&mmu)
	, apu() {}

bool DMG::init() {
	// Test roms
	size_t testRomNum = 0;
	std::string test_roms[] = {
		"cpu_instrs.gb",
		"instr_timing.gb"
	};
	std::string testRomName = "test-roms/" + test_roms[testRomNum];

	// Games
	size_t romNum = 2;
	std::string roms[] = {
		"tetris.gb",
		"tennis.gb",
		"super-mario-land.gb",
		"pokemon-red.gb",
		"loz-la.gb"
	};
	std::string romName = "roms/" + roms[romNum];

	// Create cartridge
	cart = std::make_shared<Cartridge>(romName);
	mmu.insertCartridge(cart);

	std::cout << "Beginning execution of " << romName << std::endl;

	// TODO: Clean up printing and logging
	cpu.print_toggle = false;
	cpu.log_toggle = false;
	cpu.log_file = "log/l" + std::to_string(testRomNum) + ".txt";

	// Initialize output file
	if (cpu.log_toggle) {
		cpu.file = fopen(cpu.log_file.c_str(), "w");
	}

	// Reset LY
	mmu.directWrite(0xFF44, 0x00);

	// Reset divider and timer registers
	mmu.directWrite(0xFF04, 0xAB);
	mmu.directWrite(0xFF05, 0x00);
	mmu.directWrite(0xFF06, 0x00);
	mmu.directWrite(0xFF07, 0xF8);

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
