#pragma once

#include "MMU.h"
#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"

class DMG {
public:
	DMG();

	bool init();
	bool tick();
	bool tick_frame();

public:
	MMU mmu;
	CPU cpu;
	PPU ppu;
	std::shared_ptr<Cartridge> cart;
};
