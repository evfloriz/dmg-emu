#include <iostream>

#include "DMG.h"

DMG::DMG()
	: mmu(&cpu)
	, cpu(&mmu)
	, ppu(&mmu)
	, apu(&mmu) {}

bool DMG::init() {
	// Test roms
	size_t testRomNum = 24;
	std::string test_roms[] = {
		"cpu_instrs.gb",		// 0
		"instr_timing.gb",		// 1
		"interrupt_time.gb",	// 2
		"mem_timing.gb",		// 3
		"dmg_sound.gb",			// 4
		"mbc1/bits_bank1.gb",	// 5
		"mbc1/bits_bank2.gb",	// 6
		"mbc1/bits_mode.gb",	// 7
		"mbc1/bits_ramg.gb",	// 8
		"mbc1/ram_64kb.gb",		// 9
		"mbc1/ram_256kb.gb",	// 10
		"mbc1/rom_1Mb.gb",		// 11
		"mbc1/rom_2Mb.gb",		// 12
		"mbc1/rom_4Mb.gb",		// 13
		"mbc1/rom_8Mb.gb",		// 14
		"mbc1/rom_16Mb.gb",		// 15
		"mbc1/rom_512kb.gb",	// 16
		"mts/acceptance/interrupts/ie_push.gb",	// 17
		"mts/acceptance/instr/daa.gb",	// 18
		"mts/acceptance/timer/div_write.gb",	// 19
		"mts/acceptance/timer/tim00.gb",	// 20
		"mts/acceptance/timer/tim00_div_trigger.gb",	// 21
		"mts/acceptance/halt_ime0_ei.gb",	// 22
		"mts/acceptance/halt_ime0_nointr_timing.gb",	// 23
		"mts/acceptance/halt_ime1_timing.gb",	// 24
		"mts/acceptance/halt_ime1_timing2-GS.gb",	// 25
	};

	// Games
	size_t romNum = 2;
	std::string roms[] = {
		"tetris.gb",
		"tennis.gb",
		"super-mario-land.gb",
		"pokemon-red.gb",
		"loz-la.gb",
		"metroid.gb"
	};
	
	// TODO: Add config file to set these parameters
	//std::string romName = "test-roms/" + test_roms[testRomNum];
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
	cpu.file = fopen(cpu.log_file.c_str(), "w");

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
