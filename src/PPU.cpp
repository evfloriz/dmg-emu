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
	delete[] objectsPriorityBuffer;
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

	bool incrementScanline = false;

	// Increment scanline every 456 real clock cycles
	// Or 114 M-cycles
	cycle++;
	if (cycle > 113) {
		cycle = 0;

		incrementScanline = true;
	}

	if (incrementScanline) {
		// Draw a line of the screen
		if (scanline < 144) {
			updateScanline();
		}

		scanline = mmu->directRead(0xFF44);
		scanline++;
		
		// Set VBLANK interrupt flag when LY is 144
		if (scanline == 144) {
			mmu->directWrite(0xFF0F, (1 << 0));
		}

		// Reset after 154 cycles
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

void PPU::setTile(
	uint32_t* buffer,
	uint16_t width,
	uint16_t start,
	uint16_t index,
	uint8_t x,
	uint8_t y,
	uint32_t* bgp) {

	// Read each pair of bytes and set each of the 8 lines of pixels.
	// The correct bytes are found using the start location of the tile data and the index,
	// multiplying the index by 16 since each tile takes up 16 bytes of data.
	// j is used to iterate through pairs of bytes at a time, as two bytes are
	// used for each line.
	for (int j = 0; j < 8; j++) {
		uint8_t lo = mmu->directRead(start + index * 16 + j * 2);
		uint8_t hi = mmu->directRead(start + index * 16 + j * 2 + 1);

		// This sets a row of 8 pixels in a tile from left to right.
		// The high and low bytes hold the palette reference information.
		for (int i = 7; i > -1; i--) {
			// Update palette based on selected object palette
			uint32_t paletteIndex = bgp[((hi >> i) << 1 & 0x02) | ((lo >> i) & 0x01)];
			buffer[(y + j) * width + x + 7 - i] = palette[paletteIndex];
		}
	}	
}

void PPU::setObject(
	uint32_t* buffer,
	uint16_t width,
	uint16_t start,
	uint16_t index,
	uint8_t x,
	uint8_t y,
	uint32_t* obp,
	bool xFlip,
	bool yFlip,
	bool priority) {

	for (int j = 0; j < 8; j++) {
		uint8_t lo = mmu->directRead(start + index * 16 + j * 2);
		uint8_t hi = mmu->directRead(start + index * 16 + j * 2 + 1);

		// For yFlip, change the y values of each line to print bottom up rather than top down
		uint8_t updatedJ = yFlip ? (7 - j) : j;

		for (int i = 7; i > -1; i--) {
			// For xFlip, change the order that the lines of the sprite are read from by shifting i vs 7 - i bits
			uint8_t updatedShift = xFlip ? (7 - i) : i;

			// Update palette based on selected object palette
			uint32_t paletteIndex = obp[((hi >> updatedShift) << 1 & 0x02) | ((lo >> updatedShift) & 0x01)];

			if (paletteIndex != 0) {
				// TODO: rework this to be more accurate to the Game Boy's actual operation.
				// Check against out of bounds updates.
				// More accurately, the ppu should search for sprites whose y collides with the current scanline,
				// so an out of bounds y position shouldn't have an impact.
				// For x, it counts toward the limit but if its simply greater than 168 it doesn't matter.
				// Instead of 256 and 256, I could use x < 168 and y < 160 since that would hid it completely,
				// I wouldn't need to render it on the object layer.
				if ((y + updatedJ < 256) && (x + 7 - i < 256)) {
					uint16_t bufferIndex = (y + updatedJ) * width + x + 7 - i;
					buffer[bufferIndex] = palette[paletteIndex];
					
					// Keep track if the pixel at the particular index is part of an object with a priority bit set to true
					objectsPriorityBuffer[bufferIndex] = priority;
				}
			}
		}
	}
}

void PPU::updateTileData() {
	uint16_t block0Start = 0x8000;
	uint16_t block1Start = 0x8800;
	uint16_t block2Start = 0x9000;

	// Update palette information
	uint32_t bgpData = mmu->directRead(0xFF47);
	bgp[0] = (bgpData & 0x03) >> 0;
	bgp[1] = (bgpData & 0x0C) >> 2;
	bgp[2] = (bgpData & 0x30) >> 4;
	bgp[3] = (bgpData & 0xC0) >> 6;

	for (int i = 0; i < 0x0080; i++) {
		uint8_t x = i % 16;
		uint8_t y = i / 16;

		setTile(tileDataBuffer, TILE_DATA_WIDTH, block0Start, i, x * 8, y * 8, bgp);
		setTile(tileDataBuffer, TILE_DATA_WIDTH, block1Start, i, x * 8, 64 + y * 8, bgp);
		setTile(tileDataBuffer, TILE_DATA_WIDTH, block2Start, i, x * 8, 128 + y * 8, bgp);
	}
}

void PPU::updateTileMaps() {
	// Early out if LCD is off
	bool lcdc7 = mmu->directRead(0xFF40) & (1 << 7);
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

	// Update palette information
	uint32_t bgpData = mmu->directRead(0xFF47);
	bgp[0] = (bgpData & 0x03) >> 0;
	bgp[1] = (bgpData & 0x0C) >> 2;
	bgp[2] = (bgpData & 0x30) >> 4;
	bgp[3] = (bgpData & 0xC0) >> 6;

	// Iterate through 32x32 tiles for the background and window
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

		setTile(backgroundBuffer, MAP_WIDTH, bs, bi, x * 8, y * 8, bgp);
		setTile(windowBuffer, MAP_WIDTH, ws, wi, x * 8, y * 8, bgp);
	}
}

void PPU::updateObjects() {
	// Early out if LCD is off
	bool lcdc7 = mmu->directRead(0xFF40) & (1 << 7);
	if (!lcdc7) {
		return;
	}

	// If lcdc2 = 1, sprites are 8x16 rather than 8x8
	bool lcdc2 = mmu->directRead(0xFF40) & (1 << 2);

	// Update palette information
	uint32_t obp0Data = mmu->directRead(0xFF48);
	uint32_t obp1Data = mmu->directRead(0xFF49);

	obp0[0] = 0;
	obp0[1] = (obp0Data & 0x0C) >> 2;
	obp0[2] = (obp0Data & 0x30) >> 4;
	obp0[3] = (obp0Data & 0xC0) >> 6;

	obp1[0] = 0;
	obp1[1] = (obp1Data & 0x0C) >> 2;
	obp1[2] = (obp1Data & 0x30) >> 4;
	obp1[3] = (obp1Data & 0xC0) >> 6;

	uint16_t oamStart = 0xFE00;
	uint16_t tileStart = 0x8000;

	// Clear objects array of old sprites
	std::fill(objectsBuffer, objectsBuffer + MAP_WIDTH * MAP_HEIGHT, 0);

	// Iterate 4 bytes at a time from 0xFE00 to 0xFE9F
	for (int i = 0; i < 40; i++) {
		uint8_t y = mmu->directRead(oamStart + i * 4);
		uint8_t x = mmu->directRead(oamStart + i * 4 + 1);
		uint8_t tileIndex = mmu->directRead(oamStart + i * 4 + 2);
		uint8_t flags = mmu->directRead(oamStart + i * 4 + 3);

		uint32_t* obp = (flags & (1 << 4)) ? obp1 : obp0;
		bool yFlip = (flags & (1 << 6));
		bool xFlip = (flags & (1 << 5));
		
		bool priority = (flags & (1 << 7));
		setObject(objectsBuffer, MAP_WIDTH, tileStart, tileIndex, x, y, obp, xFlip, yFlip, priority);

		if (lcdc2) {
			// Set the tile below x and y to the next tile index
			// TODO: Watch out for a bug related to the last bit being ignored.
			// eg if the index is 0x01, the two tiles should be 0x00 and 0x01, not 0x01 and 0x02.
			setObject(objectsBuffer, MAP_WIDTH, tileStart, tileIndex + 1, x, y + 8, obp, xFlip, yFlip, priority);
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

	// If lcdc1 = 1, objects are enabled
	bool lcdc1 = mmu->directRead(0xFF40) & (1 << 1);

	// If lcdc0 = 1, background and window are enabled
	// Otherwise they are white (palette[0])
	bool lcdc0 = mmu->directRead(0xFF40) & (1 << 0);

	uint8_t scx = mmu->directRead(0xFF43);
	uint8_t scy = mmu->directRead(0xFF42);
	uint8_t wx = mmu->directRead(0xFF4B);
	uint8_t wy = mmu->directRead(0xFF4A);

	// Iterate through every pixel on the current scanline and set to the correct value from the
	// background, window, and objects buffers.
	for (int i = 0; i < 160; i++) {
		uint8_t x = i;
		uint8_t y = scanline;

		uint16_t si = y * 160 + x;

		// Get the current position of the background tilemap to set in the screen,
		// including wrapping around if it would exceed the bounds of the tilemap.
		// TODO: figure out at what point constants are less clear than the actual number
		uint16_t bi = (((y + scy) % 256) * 256 + (x + scx) % 256);
		screenBuffer[si] = backgroundBuffer[bi];

		// Screen to window tilemap mapping - screen pixel minus wx (or wy), so long as its greater than 0
		if (lcdc5) {
			int wiy = y - wy;
			int wix = x - wx + 7;		// wx is window position + 7, see pandocs
			if (wix >= 0 && wiy >= 0) {
				int wi = wiy * 256 + wix;
				screenBuffer[si] = windowBuffer[wi];
			}
		}

		if (!lcdc0) {
			screenBuffer[si] = palette[0];
		}

		if (lcdc1) {
			// Get objects from the section of the object buffer that overlaps with the screen
			// Last in order so they have highest priority

			uint16_t oi = ((y + 16) * 256 + (x + 8)) % 65536;
			uint32_t obj = objectsBuffer[oi];
			bool priority = objectsPriorityBuffer[oi];

			// TODO: add object priority conditions on a per pixel basis
			if (obj != 0 && (!priority || screenBuffer[si] == palette[bgp[0]])) {
				screenBuffer[si] = obj;
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
