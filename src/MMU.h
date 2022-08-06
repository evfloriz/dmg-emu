#pragma once

#include <cstdint>
#include <array>
#include <memory>

#include "Cartridge.h"

class CPU;
class APU;

class MMU {
public:
	MMU(CPU* cpu, APU* apu);
	~MMU();

public:
	std::shared_ptr<Cartridge> cart;
	CPU* cpu = nullptr;
	APU* apu = nullptr;

	// Main memory, bottom half is unused but simplifies addressing
	uint8_t* memory = new uint8_t[64 * 1024];

public:
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	// Lower overhead direct MMU read and write
	void directWrite(uint16_t addr, uint8_t data);
	uint8_t directRead(uint16_t addr);

	void setBit(uint16_t addr, uint8_t pos, uint8_t value);

	void insertCartridge(const std::shared_ptr<Cartridge>& cartridge);

	// Set the values of the input
	void writeActionButton(uint8_t pos, uint8_t value);
	void writeDirectionButton(uint8_t pos, uint8_t value);
	void writeButton(uint8_t* buttons, uint8_t pos, uint8_t value);

private:
	uint8_t actionButtons = 0xFF;
	uint8_t directionButtons = 0xFF;
	uint8_t buttonsOff = 0xFF;
	uint8_t* selectedButtons = &buttonsOff;
};
