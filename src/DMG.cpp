
#include <iostream>

#include "DMG.h"

DMG::DMG()
	: mmu()
	, cpu(&mmu)
	, ppu(&mmu) {}

bool DMG::init() {
	// Passing tests 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, instr_timing
	size_t test_num = 12;

	// Get rom name
	std::string test_roms[] = {
		"cpu_instrs.gb",
		"01-special.gb",
		"02-interrupts.gb",
		"03-op sp,hl.gb",
		"04-op r,imm.gb",
		"05-op rp.gb",
		"06-ld r,r.gb",
		"07-jr,jp,call,ret,rst.gb",
		"08-misc instrs.gb",
		"09-op r,r.gb",
		"10-bit ops.gb",
		"11-op a,(hl).gb",
		"instr_timing.gb"
	};
	std::string romName = "test-roms/" + test_roms[test_num];

	//romName = "roms/tetris.gb";
	romName = "roms/tennis.gb";

	// Create cartridge
	cart = std::make_shared<Cartridge>(romName);
	mmu.insertCartridge(cart);

	std::cout << "Beginning execution of " << romName << std::endl;

	cpu.print_toggle = false;
	cpu.log_toggle = false;
	cpu.log_file = "log/l" + std::to_string(test_num) + ".txt";

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
