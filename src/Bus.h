#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "CPU.h"
#include "PPU.h"
#include "Cartridge.h"

class Bus {
public:
	Bus(CPU* cpu, PPU* ppu);
	~Bus();

public:
	// Devices on bus
	CPU *cpu;
	PPU *ppu;
	std::shared_ptr<Cartridge> cart;

	// Sections of memory (excluding rom which is on cartridge)
	/*std::array<uint8_t, 8 * 1024> vram;
	std::array<uint8_t, 8 * 1024> externalRam;
	std::array<uint8_t, 8 * 1024> wram;
	std::array<uint8_t, 160> oam;
	std::array<uint8_t, 128> ioRegisters;
	std::array<uint8_t, 128> hram;*/

	/*uint8_t vram[8 * 1024];
	uint8_t externalRam[8 * 1024];
	uint8_t wram[8 * 1024];
	uint8_t oam[160];
	uint8_t ioRegisters[128];
	uint8_t hram[128];*/
	uint8_t* memory = new uint8_t[32 * 1024];

	/*uint8_t *vram = new uint8_t[8 * 1024];
	uint8_t *externalRam = new uint8_t[8 * 1024];
	uint8_t *wram = new uint8_t[8 * 1024];
	uint8_t *oam = new uint8_t[160];
	uint8_t *ioRegisters = new uint8_t[128];
	uint8_t *hram = new uint8_t[128];*/
	
	// Implement frequently used registers as ints for quicker access
	uint8_t ieRegister;
	uint8_t ifRegister;
	uint8_t timerControlRegister;



public:
	// Bus read and write
	// Should be 16 bit addresses with 8 bit data, read one register at a time I think
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);
	void clock();
};
