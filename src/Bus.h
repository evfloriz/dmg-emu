#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"

class Bus {
public:
	Bus();
	~Bus();

public:
	// Devices on bus
	CPU cpu;
	PPU ppu;
	std::shared_ptr<Cartridge> cart;

	// Sections of memory (excluding rom which is on cartridge)
	std::array<uint8_t, 8 * 1024> vram;
	std::array<uint8_t, 8 * 1024> externalRam;
	std::array<uint8_t, 8 * 1024> wram;
	std::array<uint8_t, 160> oam;
	std::array<uint8_t, 128> ioRegisters;
	std::array<uint8_t, 128> hram;
	std::array<uint8_t, 1> ieRegister;

public:
	// Bus read and write
	// Should be 16 bit addresses with 8 bit data, read one register at a time I think
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void clock();
};
