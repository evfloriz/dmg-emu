#pragma once

#include "MMU.h"
#include "CPU.h"
#include "PPU.h"
#include "APU.h"
#include "Cartridge.h"

class DMG {
public:
	DMG();

	bool init();
	bool tick();
	bool tickFrame();
	bool reset();

public:
	MMU mmu;
	CPU cpu;
	PPU ppu;
	APU apu;
	std::shared_ptr<Cartridge> cart;
};
