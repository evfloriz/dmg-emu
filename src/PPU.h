#pragma once

#include <cstdint>

#include "Util.h"
#include "Cartridge.h"

class MMU;

class PPU {
public:
	PPU(MMU* mmu);
	~PPU();

public:
	void clock();
	void updateLY();

	void updateTileData();
	void updateTileMaps();
	void updateScanline();
	void updateObjects();

	void setTile(
		uint32_t* buffer,
		uint16_t width,
		uint16_t start,
		uint16_t index,
		uint8_t x,
		uint8_t y,
		uint32_t* bgp);

	void setObject(
		uint32_t* buffer,
		uint16_t width,
		uint16_t start,
		uint16_t index,
		uint8_t x,
		uint8_t y,
		uint32_t* obp,
		bool xFlip,
		bool yFlip);

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

	uint32_t* screenBuffer = new uint32_t[DMG_WIDTH * DMG_HEIGHT];
	uint32_t* tileDataBuffer = new uint32_t[TILE_DATA_WIDTH * TILE_DATA_HEIGHT];
	uint32_t* backgroundBuffer = new uint32_t[MAP_WIDTH * MAP_HEIGHT];
	uint32_t* windowBuffer = new uint32_t[MAP_WIDTH * MAP_HEIGHT];
	uint32_t* objectsBuffer = new uint32_t[MAP_WIDTH * MAP_HEIGHT];
};
