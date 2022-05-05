#include <iostream>

#include "MMU.h"
#include "Util.h"

#include "PPU.h"

PPU::PPU(MMU* mmu) {
	this->mmu = mmu;

	palette[0] = ARGB(155, 188, 155);
	palette[1] = ARGB(139, 172, 139);
	palette[2] = ARGB(48, 98, 48);
	palette[3] = ARGB(15, 56, 15);
}

PPU::~PPU() {
	delete[] screenBuffer;
	delete[] tileDataBuffer;
	delete[] backgroundBuffer;
	delete[] windowBuffer;
	delete[] objectsBuffer;
}

void PPU::write(uint16_t addr, uint8_t data) {
}

uint8_t PPU::read(uint16_t addr) {	
	uint8_t data = 0x00;

	// Handle data in each of the 12 registers of the ppu
	switch (addr) {
	case 0xFF40:
		break;
	case 0xFF41:
		break;
	case 0xFF42:
		break;
	case 0xFF43:
		break;
	case 0xFF44:
		break;
	case 0xFF45:
		break;
	case 0xFF46:
		break;
	case 0xFF47:
		break;
	case 0xFF48:
		break;
	case 0xFF49:
		break;
	case 0xFF4A:
		break;
	case 0xFF4B:
		break;
	}

	return data;
}

void PPU::clock() {
	// Update LY
	// This function calls the screen updating functions as well
	updateLY();
}

void PPU::updateLY() {
	// TODO: does it make sense for the PPU to read random areas of memory other than vram and oam?
	// First check if screen is off and reset everything if so
	if (!(mmu->directRead(0xFF40) & 0x80)) {
		// LCD off
		mmu->directWrite(0xFF44, 0x00);
		cycle = 0;

		return;
	}

	bool increaseScanline = false;

	// Increment every 456 real clock cycles
	// Or 114 M-cycles
	cycle++;
	if (cycle > 113) {
		cycle = 0;

		increaseScanline = true;
	}

	if (increaseScanline) {
		// Draw a line of the screen
		if (scanline < 144) {
			updateScanline();
			
		}

		scanline = mmu->directRead(0xFF44);

		// Reset after 154 cycles
		scanline++;
		
		// Set VBLANK interrupt flag when LY is 144
		if (scanline == 144) {
			mmu->directWrite(0xFF0F, (1 << 0));
		}

		if (scanline > 153) {
			scanline = 0x00;
			frameComplete = true;

			// Update the tilemaps at the end of every frame
			updateTileData();
			updateTileMaps();
			updateObjects();
		}

		mmu->directWrite(0xFF44, scanline);
	}
}

/*
* This is just for debug use, it shouldn't actually be called.
*/
void PPU::updateTileData() {
	uint16_t block0Start = 0x8000;
	uint16_t block1Start = 0x8800;
	uint16_t block2Start = 0x9000;

	auto setLine = [&](uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// Get palette index from leftmost to rightmost pixel
		tileDataBuffer[y * TILE_DATA_WIDTH + x    ] = palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)];
		tileDataBuffer[y * TILE_DATA_WIDTH + x + 1] = palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)];
		tileDataBuffer[y * TILE_DATA_WIDTH + x + 2] = palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)];
		tileDataBuffer[y * TILE_DATA_WIDTH + x + 3] = palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)];

		tileDataBuffer[y * TILE_DATA_WIDTH + x + 4] = palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)];
		tileDataBuffer[y * TILE_DATA_WIDTH + x + 5] = palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)];
		tileDataBuffer[y * TILE_DATA_WIDTH + x + 6] = palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)];
		tileDataBuffer[y * TILE_DATA_WIDTH + x + 7] = palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)];
	};

	for (int i = 0; i < 0x0080; i++) {
		uint8_t x = i % 16;
		uint8_t y = i / 16;

		for (int j = 0; j < 8; j++) {
			uint8_t lo0 = mmu->directRead(block0Start + i * 16 + j * 2);
			uint8_t hi0 = mmu->directRead(block0Start + i * 16 + j * 2 + 1);

			uint8_t lo1 = mmu->directRead(block1Start + i * 16 + j * 2);
			uint8_t hi1 = mmu->directRead(block1Start + i * 16 + j * 2 + 1);

			uint8_t lo2 = mmu->directRead(block2Start + i * 16 + j * 2);
			uint8_t hi2 = mmu->directRead(block2Start + i * 16 + j * 2 + 1);

			setLine(x * 8, j + y * 8, hi0, lo0);
			setLine(x * 8, 64 + j + y * 8, hi1, lo1);
			setLine(x * 8, 128 + j + y * 8, hi2, lo2);
		}
	}
}

void PPU::updateTileMaps() {
	bool lcdc7 = mmu->directRead(0xFF40) & (1 << 7);

	// Early out if LCD is off
	if (!lcdc7) {
		return;
	}

	// If lcdc4 = 1, 0-127 starts at 0x8000 and 128-255 starts at 0x8800
	// If lcdc4 = 0, 0-127 starts at 0x9000 and 128-255 starts at 0x8800
	bool lcdc4 = mmu->directRead(0xFF40) & (1 << 4);
	uint16_t firstHalfStart = lcdc4 ? 0x8000 : 0x9000;
	uint16_t secondHalfStart = 0x8800;

	// If lcdc3 = 1, the background map starts at 0x9C00, otherwise 0x9800
	bool lcdc3 = mmu->directRead(0xFF40) & (1 << 3);
	uint16_t backgroundStart = lcdc3 ? 0x9C00 : 0x9800;

	// If lcdc6 = 1, win map starts at 0x9C00, otherwise 0x9800
	bool lcdc6 = mmu->directRead(0xFF40) & (1 << 6);
	uint16_t windowStart = lcdc6 ? 0x9C00 : 0x9800;

	auto setLine = [&](uint32_t* buffer, uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// This sets a row of 8 pixels in a tile from left to right.
		// The high and low bytes hold the palette reference information.
		// The y position is adjusted in the loop that calls this function,
		// as each tile is set a line at a time.
		buffer[y * MAP_WIDTH + x] = palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)];
		buffer[y * MAP_WIDTH + x + 1] = palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)];
		buffer[y * MAP_WIDTH + x + 2] = palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)];
		buffer[y * MAP_WIDTH + x + 3] = palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)];

		buffer[y * MAP_WIDTH + x + 4] = palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)];
		buffer[y * MAP_WIDTH + x + 5] = palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)];
		buffer[y * MAP_WIDTH + x + 6] = palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)];
		buffer[y * MAP_WIDTH + x + 7] = palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)];
	};

	// Iterate through the 32x32 tiles for the background and window
	for (int i = 0; i < 0x0400; i++) {
		uint8_t x = i % 32;
		uint8_t y = i / 32;

		// Determine the index of the tile in the data, and determine the correct
		// starting location for block of tile data given that index
		uint8_t bi = mmu->directRead(backgroundStart + i);
		uint16_t bs = (bi > 127) ? secondHalfStart : firstHalfStart;
		bi %= 128;

		uint8_t wi = mmu->directRead(windowStart + i);
		uint16_t ws = (wi > 127) ? secondHalfStart : firstHalfStart;
		wi %= 128;

		// TODO: clean up logic
		// Read each pair of bytes and set each of the 8 lines of pixels.
		// The correct bytes are found using the start location and index found above,
		// multiplying the index by 16 since each tile takes up 16 bytes of data.
		// j is used to iterate through pairs of bytes at a time, as two bytes are
		// used for each line.
		for (int j = 0; j < 8; j++) {
			// TODO: Should these be direct reads?
			uint8_t bg_lo = mmu->directRead(bs + bi * 16 + j * 2);
			uint8_t bg_hi = mmu->directRead(bs + bi * 16 + j * 2 + 1);

			uint8_t win_lo = mmu->directRead(ws + wi * 16 + j * 2);
			uint8_t win_hi = mmu->directRead(ws + wi * 16 + j * 2 + 1);

			setLine(backgroundBuffer, x * 8, j + y * 8, bg_hi, bg_lo);
			setLine(windowBuffer, x * 8, j + y * 8, win_hi, win_lo);
		}
	}
}

void PPU::updateObjects() {
	// NOTE: there's a sprite drawn at the top left of the screen, not sure if it's supposed to be there

	bool lcdc7 = mmu->directRead(0xFF40) & (1 << 7);

	// Early out if LCD is off
	if (!lcdc7) {
		return;
	}

	uint16_t oamStart = 0xFE00;
	uint16_t tileStart = 0x8000;

	// TODO: Figure out a better way to handle these lambdas
	auto setLine = [&](uint32_t* buffer, uint8_t x, uint8_t y, uint8_t hi, uint8_t lo) {
		// This sets a row of 8 pixels in a tile from left to right.
		// The high and low bytes hold the palette reference information.
		// The y position is adjusted in the loop that calls this function,
		// as each tile is set a line at a time.
		buffer[y * MAP_WIDTH + x] = palette[((hi >> 6) & (1 << 1)) | ((lo >> 7) & 1)];
		buffer[y * MAP_WIDTH + x + 1] = palette[((hi >> 5) & (1 << 1)) | ((lo >> 6) & 1)];
		buffer[y * MAP_WIDTH + x + 2] = palette[((hi >> 4) & (1 << 1)) | ((lo >> 5) & 1)];
		buffer[y * MAP_WIDTH + x + 3] = palette[((hi >> 3) & (1 << 1)) | ((lo >> 4) & 1)];

		buffer[y * MAP_WIDTH + x + 4] = palette[((hi >> 2) & (1 << 1)) | ((lo >> 3) & 1)];
		buffer[y * MAP_WIDTH + x + 5] = palette[((hi >> 1) & (1 << 1)) | ((lo >> 2) & 1)];
		buffer[y * MAP_WIDTH + x + 6] = palette[((hi >> 0) & (1 << 1)) | ((lo >> 1) & 1)];
		buffer[y * MAP_WIDTH + x + 7] = palette[((hi << 1) & (1 << 1)) | ((lo >> 0) & 1)];
	};

	// Clear objects array of old sprites
	std::fill(objectsBuffer, objectsBuffer + 256 * 256, 0);

	// Iterate 4 bytes at a time from 0xFE00 to 0xFE9F
	for (int i = 0; i < 40; i++) {
		// TODO: Should these be direct reads?
		uint8_t y = mmu->directRead(oamStart + i * 4);
		uint8_t x = mmu->directRead(oamStart + i * 4 + 1);
		uint8_t tileIndex = mmu->directRead(oamStart + i * 4 + 2);
		uint8_t flags = mmu->directRead(oamStart + i * 4 + 3);

		for (int j = 0; j < 8; j++) {
			uint8_t lo = mmu->directRead(tileStart + tileIndex * 16 + j * 2);
			uint8_t hi = mmu->directRead(tileStart + tileIndex * 16 + j * 2 + 1);

			setLine(objectsBuffer, x, j + y, hi, lo);
		}
	}
}

void PPU::updateScreen() {
	// If lcdc5 = 1, window is enabled
	bool lcdc5 = mmu->directRead(0xFF40) & (1 << 5);

	// With two 32x32 arrays, use the scroll and window position to set the screen buffer
	
	uint8_t scx = mmu->directRead(0xFF43);
	uint8_t scy = mmu->directRead(0xFF42);
	uint8_t wx = mmu->directRead(0xFF4B);
	uint8_t wy = mmu->directRead(0xFF4A);

	uint16_t screenStart = scy * 32 + scx;
	
	// Iterate through every pixel on the screen and set to the corresponding value
	// of the background buffer
	for (int i = 0; i < 160 * 144; i++) {
		uint8_t x = i % 160;
		uint8_t y = i / 160;

		//for (int i = screenStart; i < (screenStart + 0x0168) * 64; i++) {

		// Get the current position of the tilemap to set in the screen,
		// including wrapping around if it would exceed the bounds of the tilemap.
		uint16_t bgIndex = ((y + scy) * 256 + (x + scx)) % 65536;
		screenBuffer[i] = backgroundBuffer[bgIndex];

		// screen to window tilemap mapping - screen pixel minus wx (or wy), so long as its greater than 0
		if (lcdc5) {
			int winIndexY = y - wy;
			int winIndexX = x - wx + 7;		// wx is window position + 7, see pandocs
			if (winIndexX >= 0 && winIndexY >= 0) {
				int winIndex = (winIndexY * 256 + winIndexX);
				screenBuffer[i] = windowBuffer[winIndex];
			}
		}
	}
}

void PPU::updateScanline() {
	// Early out if scanline is greater than 143, shouldn't ever hit this point
	if (scanline > 143) {
		return;
	}

	// If lcdc5 = 1, window is enabled
	bool lcdc5 = mmu->directRead(0xFF40) & (1 << 5);

	// With two 32x32 arrays, use the scroll and window position to set the screen buffer
	uint8_t scx = mmu->directRead(0xFF43);
	uint8_t scy = mmu->directRead(0xFF42);
	uint8_t wx = mmu->directRead(0xFF4B);
	uint8_t wy = mmu->directRead(0xFF4A);

	uint16_t screenStart = scy * 32 + scx;

	// Iterate through every pixel on the screen and set to the corresponding value
	// of the background buffer
	for (int i = 0; i < 160; i++) {
		uint8_t x = i;
		uint8_t y = scanline;

		uint16_t screenIndex = y * 160 + x;

		// Get the current position of the tilemap to set in the screen,
		// including wrapping around if it would exceed the bounds of the tilemap.
		uint16_t bgIndex = ((y + scy) * 256 + (x + scx)) % 65536;
		screenBuffer[screenIndex] = backgroundBuffer[bgIndex];

		// Screen to window tilemap mapping - screen pixel minus wx (or wy), so long as its greater than 0
		if (lcdc5) {
			int winIndexY = y - wy;
			int winIndexX = x - wx + 7;		// wx is window position + 7, see pandocs
			if (winIndexX >= 0 && winIndexY >= 0) {
				int winIndex = (winIndexY * 256 + winIndexX);
				screenBuffer[screenIndex] = windowBuffer[winIndex];
			}
		}
	}
}

uint32_t* PPU::getScreenBuffer() {
	return screenBuffer;
}

uint32_t* PPU::getTileDataBuffer() {
	return tileDataBuffer;
}

uint32_t* PPU::getBackgroundBuffer() {
	return backgroundBuffer;
}

uint32_t* PPU::getWindowBuffer() {
	return windowBuffer;
}

uint32_t* PPU::getObjectsBuffer() {
	return objectsBuffer;
}
