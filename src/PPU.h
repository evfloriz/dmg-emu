#pragma once

#include <cstdint>

#include "Cartridge.h"

class MMU;

class PPU {
public:
	PPU(MMU* mmu);
	~PPU();

public:
	void write(uint16_t addr, uint8_t data);
	uint8_t read(uint16_t addr);

	void clock();
	void updateLY();

	void updateTileData();
	void updateTileMaps();
	void updateScreen();
	void updateScanline();

	uint32_t* getScreenBuffer();
	uint32_t* getTileDataBuffer();
	uint32_t* getBackgroundBuffer();
	uint32_t* getWindowBuffer();

	bool frameComplete = false;

private:
	MMU* mmu = nullptr;
	uint32_t palette[4];

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	uint32_t screenBuffer[160 * 144];
	uint32_t tileDataBuffer[128 * 192];
	uint32_t backgroundBuffer[256 * 256];
	uint32_t windowBuffer[256 * 256];
};
