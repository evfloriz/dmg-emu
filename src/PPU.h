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
	void updateObjects();

	uint32_t* getScreenBuffer();
	uint32_t* getTileDataBuffer();
	uint32_t* getBackgroundBuffer();
	uint32_t* getWindowBuffer();
	uint32_t* getObjectsBuffer();

	bool frameComplete = false;

private:
	MMU* mmu = nullptr;
	uint32_t palette[4];

	uint16_t cycle = 0;
	uint8_t scanline = 0;

	uint32_t* screenBuffer = new uint32_t[160 * 144];
	uint32_t* tileDataBuffer = new uint32_t[128 * 192];
	uint32_t* backgroundBuffer = new uint32_t[256 * 256];
	uint32_t* windowBuffer = new uint32_t[256 * 256];
	uint32_t* objectsBuffer = new uint32_t[256 * 256];
};
