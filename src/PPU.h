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

	// TODO: Do a pass on function access, I think a lot of these should be private. I think this goes alongside
	// an update to frameComplete in the aim of keeping the PPU state internal to the PPU.

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
		bool yFlip,
		bool priority);

	uint32_t* getScreenBuffer();
	uint32_t* getTileDataBuffer();
	uint32_t* getBackgroundBuffer();
	uint32_t* getWindowBuffer();
	uint32_t* getObjectsBuffer();

	uint32_t readPixel(uint16_t index);
	void updateBackgroundScanline(uint32_t* buffer, int* startingIndex);
	void updateWindowScanline(uint32_t* buffer, int* startingIndex);

	void updateBackgroundTileMap();
	void updateWindowTileMap();

	bool frameComplete = false;

private:
	MMU* mmu = nullptr;
	uint32_t palette[4] = {};
	uint32_t bgp[4] = {};
	uint32_t obp0[4] = {};
	uint32_t obp1[4] = {};

	uint16_t cycle = 0;
	uint8_t ly = 0;

	uint32_t* screenBuffer = new uint32_t[util::DMG_WIDTH * util::DMG_HEIGHT];
	uint32_t* tileDataBuffer = new uint32_t[util::TILE_DATA_WIDTH * util::TILE_DATA_HEIGHT];
	uint32_t* backgroundBuffer = new uint32_t[util::MAP_WIDTH * util::MAP_HEIGHT];
	uint32_t* windowBuffer = new uint32_t[util::MAP_WIDTH * util::MAP_HEIGHT];
	uint32_t* objectsBuffer = new uint32_t[util::MAP_WIDTH * util::MAP_HEIGHT];
	bool* objectsPriorityBuffer = new bool[util::MAP_WIDTH * util::MAP_HEIGHT];

	uint16_t firstHalfStart = 0x0000;
	uint16_t secondHalfStart = 0x0000;
	uint16_t backgroundStart = 0x0000;
	uint16_t windowStart = 0x0000;
};
