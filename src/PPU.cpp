#include <iostream>

#include "MMU.h"

#include "PPU.h"

PPU::PPU(MMU* mmu) {
	this->mmu = mmu;

	// Colours taken from gambatte libretro core

	if (util::palette == "mgb") {
		// MGB palette
		palette[0] = util::ARGB(167, 177, 154);
		palette[1] = util::ARGB(134, 146, 124);
		palette[2] = util::ARGB(83, 95, 73);
		palette[3] = util::ARGB(42, 51, 37);
		
	}
	else if (util::palette == "dmg") {
		// DMG palette
		palette[0] = util::ARGB(87, 130, 0);
		palette[1] = util::ARGB(49, 116, 0);
		palette[2] = util::ARGB(00, 81, 33);
		palette[3] = util::ARGB(00, 66, 12);
		
	}
	else {
		// Original working palette, shouldn't be used as mgb is default
		palette[0] = util::ARGB(155, 188, 155);
		palette[1] = util::ARGB(139, 172, 139);
		palette[2] = util::ARGB(48, 98, 48);
		palette[3] = util::ARGB(15, 56, 15);
	}
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
	// TODO: does it make sense for the PPU to read random areas of memory other than vram and oam?
	// First check if screen is off and reset everything if so
	if (!(lcdc & 0x80)) {
		// LCD off
		ly = 0x00;
		cycle = 0;

		return;
	}

	// TODO: Figure out timings, when should things be calculated vs incremented? Right now cycles and LY are
	// incremented in the middle of a clock cycle

	// Determine mode
	if (ly < 144) {
		if (cycle < 20) {
			// Mode 2
			stat &= 0xFC;
			stat |= 0x02;
		}
		else if (cycle < 73) {
			// Mode 3 - assuming maximum time for now
			stat &= 0xFC;
			stat |= 0x03;
		}
		else {
			// Mode 0
			stat &= 0xFC;
			stat |= 0x00;
		}
	}
	else {
		// Mode 1
		stat &= 0xFC;
		stat |= 0x01;
	}
	
	// Increment LY every 456 real clock cycles
	// Or 114 M-cycles
	bool incrementLY = false;
	cycle++;
	if (cycle > 113) {
		cycle = 0;
		incrementLY = true;
	}

	if (incrementLY) {
		// Draw a line of the screen
		if (ly < 144) {
			updateScanline();
		}

		// Handle LYC state
		uint8_t lyc = mmu->directRead(0xFF45);
		if (ly == lyc) {
			// Set bit 2 of STAT
			stat |= (1 << 2);
		}
		else {
			// Reset bit 2 of STAT
			stat &= ~(1 << 2);
		}

		ly++;
		
		// Set VBLANK interrupt flag when LY is 144
		if (ly == 144) {
			mmu->setIF(0, 1);
		}

		// Reset after 154 cycles
		if (ly > 153) {
			ly = 0x00;
			frameComplete = true;

			// Update the tilemaps and objects at the end of every frame
			updateObjects();
		}
	}

	// Handle STAT interrupt
	if ((stat & (1 << 6) && stat & (1 << 2)) ||		// LYC=LY interrupt
		(stat & (1 << 5) && (stat & 0x03) == 2) ||	// OAM interrupt (mode 2)
		(stat & (1 << 4) && (stat & 0x03) == 1) ||	// VBlank interrupt
		(stat & (1 << 3) && (stat & 0x03) == 0)) {	// HBlank interrupt
		mmu->setIF(1, 1);
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
			uint32_t objectPaletteIndex = ((hi >> updatedShift) << 1 & 0x02) | ((lo >> updatedShift) & 0x01);

			if (objectPaletteIndex != 0) {
				// TODO: rework this to be more accurate to the Game Boy's actual operation.
				// Check against out of bounds updates.
				// More accurately, the ppu should search for sprites whose y collides with the current scanline,
				// so an out of bounds y position shouldn't have an impact.
				// For x, it counts toward the limit but if its simply greater than 168 it doesn't matter.
				// Instead of 256 and 256, I could use x < 168 and y < 160 since that would hid it completely,
				// I wouldn't need to render it on the object layer.
				if ((y + updatedJ < 256) && (x + 7 - i < 256)) {
					uint16_t bufferIndex = (y + updatedJ) * width + x + 7 - i;
					buffer[bufferIndex] = palette[obp[objectPaletteIndex]];
					
					// Keep track if the pixel at the particular index is part of an object with a priority bit set to true
					objectsPriorityBuffer[bufferIndex] = priority;
				}
			}
		}
	}
}

// Useful for debugging but not needed for emulation
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

		setTile(tileDataBuffer, util::TILE_DATA_WIDTH, block0Start, i, x * 8, y * 8, bgp);
		setTile(tileDataBuffer, util::TILE_DATA_WIDTH, block1Start, i, x * 8, 64 + y * 8, bgp);
		setTile(tileDataBuffer, util::TILE_DATA_WIDTH, block2Start, i, x * 8, 128 + y * 8, bgp);
	}
}

void PPU::updateTileMaps() {
	// Early out if LCD is off
	bool lcdc7 = lcdc & (1 << 7);
	if (!lcdc7) {
		return;
	}

	// If lcdc4 = 1, 0-127 starts at 0x8000 and 128-255 starts at 0x8800
	// If lcdc4 = 0, 0-127 starts at 0x9000 and 128-255 starts at 0x8800
	bool lcdc4 = lcdc & (1 << 4);
	uint16_t firstHalfStart = lcdc4 ? 0x8000 : 0x9000;
	uint16_t secondHalfStart = 0x8800;

	// If lcdc3 = 1, the background map starts at 0x9C00, otherwise 0x9800
	bool lcdc3 = lcdc & (1 << 3);
	uint16_t backgroundStart = lcdc3 ? 0x9C00 : 0x9800;

	// If lcdc6 = 1, win map starts at 0x9C00, otherwise 0x9800
	bool lcdc6 = lcdc & (1 << 6);
	uint16_t windowStart = lcdc6 ? 0x9C00 : 0x9800;

	// Update palette information
	uint32_t bgpData = mmu->directRead(0xFF47);
	bgp[0] = (bgpData & 0x03) >> 0;
	bgp[1] = (bgpData & 0x0C) >> 2;
	bgp[2] = (bgpData & 0x30) >> 4;
	bgp[3] = (bgpData & 0xC0) >> 6;

	for (int i = 0; i < 32; i++) {
		uint8_t x = i * 8;

		for (int j = 0; j < 32; j++) {
			uint8_t y = j * 8;
			uint16_t index = j * 32 + i;

			// Determine the index of the tile in the data, and determine the correct
			// starting location for block of tile data given that index
			uint8_t bi = mmu->directRead(backgroundStart + index);
			uint16_t bs = (bi > 127) ? secondHalfStart : firstHalfStart;
			bi &= 0x7F;		// % 128

			uint8_t wi = mmu->directRead(windowStart + index);
			uint16_t ws = (wi > 127) ? secondHalfStart : firstHalfStart;
			wi &= 0x7F;		// % 128

			setTile(backgroundBuffer, util::MAP_WIDTH, bs, bi, x, y, bgp);
			setTile(windowBuffer, util::MAP_WIDTH, ws, wi, x, y, bgp);
		}
	}
}

void PPU::updateObjects() {
	// Early out if LCD is off
	bool lcdc7 = lcdc & (1 << 7);
	if (!lcdc7) {
		return;
	}

	// If lcdc2 = 1, sprites are 8x16 rather than 8x8
	bool lcdc2 = lcdc & (1 << 2);

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
	std::fill(objectsBuffer, objectsBuffer + util::MAP_WIDTH * util::MAP_HEIGHT, 0);

	// Iterate 4 bytes at a time from 0xFE00 to 0xFE9F
	// Go in reverse order so earlier oam entries overwrite later ones
	for (int i = 40; i > -1; i--) {
		uint8_t y = mmu->directRead(oamStart + i * 4);
		uint8_t x = mmu->directRead(oamStart + i * 4 + 1);
		uint8_t tileIndex = mmu->directRead(oamStart + i * 4 + 2);
		uint8_t flags = mmu->directRead(oamStart + i * 4 + 3);

		uint32_t* obp = (flags & (1 << 4)) ? obp1 : obp0;
		bool yFlip = (flags & (1 << 6));
		bool xFlip = (flags & (1 << 5));
		
		bool priority = (flags & (1 << 7));
		setObject(objectsBuffer, util::MAP_WIDTH, tileStart, tileIndex, x, y, obp, xFlip, yFlip, priority);

		if (lcdc2) {
			// Set the tile below x and y to the next tile index
			// TODO: Watch out for a bug related to the last bit being ignored.
			// eg if the index is 0x01, the two tiles should be 0x00 and 0x01, not 0x01 and 0x02.
			setObject(objectsBuffer, util::MAP_WIDTH, tileStart, tileIndex + 1, x, y + 8, obp, xFlip, yFlip, priority);
		}
	}
}

void PPU::updateBackgroundScanline(uint32_t* buffer, int* startingIndex) {
	uint8_t scx = mmu->directRead(0xFF43);
	uint8_t scy = mmu->directRead(0xFF42);
	
	// Find y coordinate of the tile in the background map
	uint8_t y = (ly + scy) % 256;
	uint16_t tileY = y / 8;
	uint16_t tileYPos = tileY * 32;

	// Starting index is the offset within the first tile to start reading
	*startingIndex = scx & 0x07;

	// Construct scanline, max 21 tiles need to be loaded
	for (int i = 0; i < 21; i++) {
		// Find x of the tile in the background map
		uint8_t x = (i * 8 + scx) % 256;
		uint16_t tileX = x / 8;

		uint16_t tileIndex = tileYPos + tileX;

		uint8_t index = mmu->directRead(backgroundStart + tileIndex);
		uint16_t start = (index > 127) ? secondHalfStart : firstHalfStart;
		index &= 0x7F;

		// Find the y of the correct line within the tile
		uint8_t lineY = y - (tileY * 8);

		uint8_t lo = mmu->directRead(start + index * 16 + lineY * 2);
		uint8_t hi = mmu->directRead(start + index * 16 + lineY * 2 + 1);

		// Add 8 consecutive horizontal pixels to the line
		for (int j = 0; j < 8; j++) {
			uint32_t paletteIndex = bgp[((hi >> (7 - j)) << 1 & 0x02) | ((lo >> (7 - j)) & 0x01)];
			buffer[i * 8 + j] = palette[paletteIndex];
		}
	}
}

void PPU::updateWindowScanline(uint32_t* buffer, int* startingIndex) {
	uint8_t wx = mmu->directRead(0xFF4B);
	uint8_t wy = mmu->directRead(0xFF4A);

	// Early out if current ly doesn't intersect with the window
	int y = ly - wy;
	if (y < 0) {
		*startingIndex = 160;
		return;
	}

	// Find y of the tile to work with
	uint16_t tileY = y / 8;
	uint16_t tileYPos = tileY * 32;

	// Starting index indicates where the first position of the window is in the scanline (-7 to 159)
	*startingIndex = wx - 7;

	// Construct scanline, max 21 tiles need to be loaded
	for (int i = 0; i < 21; i++) {
		// Find x of the tile to work with
		uint8_t x = i * 8;
		uint16_t tileX = i;

		// Find tile to load
		uint16_t tileIndex = tileYPos + tileX;

		uint8_t index = mmu->directRead(windowStart + tileIndex);
		uint16_t start = (index > 127) ? secondHalfStart : firstHalfStart;
		index &= 0x7F;

		// Find the y of the correct line within the tile
		uint8_t lineY = y - (tileY * 8);

		uint8_t lo = mmu->directRead(start + index * 16 + lineY * 2);
		uint8_t hi = mmu->directRead(start + index * 16 + lineY * 2 + 1);

		// Add 8 consecutive horizontal pixels to the line
		for (int j = 0; j < 8; j++) {
			uint32_t paletteIndex = bgp[((hi >> (7 - j)) << 1 & 0x02) | ((lo >> (7 - j)) & 0x01)];
			buffer[i * 8 + j] = palette[paletteIndex];
		}
	}
}

void PPU::updateScanline() {
	// Early out if scanline is greater than 143, shouldn't ever hit this point
	if (ly > 143) {
		return;
	}

	bool lcdc5 = lcdc & (1 << 5);				// If lcdc5 = 1, window is enabled
	bool lcdc1 = lcdc & (1 << 1);				// If lcdc1 = 1, objects are enabled
	bool lcdc0 = lcdc & (1 << 0);				// If lcdc0 = 1, background and window are enabled
												// Otherwise they are white (palette[0])
	bool lcdc4 = lcdc & (1 << 4);				// If lcdc4 = 1, 0-127 starts at 0x8000 and 128-255 starts at 0x8800
												// If lcdc4 = 0, 0-127 starts at 0x9000 and 128-255 starts at 0x8800
	bool lcdc3 = lcdc & (1 << 3);				// If lcdc3 = 1, the background map starts at 0x9C00, otherwise 0x9800
	bool lcdc6 = lcdc & (1 << 6);				// If lcdc6 = 1, win map starts at 0x9C00, otherwise 0x9800

	firstHalfStart = lcdc4 ? 0x8000 : 0x9000;	
	secondHalfStart = 0x8800;	
	backgroundStart = lcdc3 ? 0x9C00 : 0x9800;
	windowStart = lcdc6 ? 0x9C00 : 0x9800;

	// Update palette information
	uint32_t bgpData = mmu->directRead(0xFF47);
	bgp[0] = (bgpData & 0x03) >> 0;
	bgp[1] = (bgpData & 0x0C) >> 2;
	bgp[2] = (bgpData & 0x30) >> 4;
	bgp[3] = (bgpData & 0xC0) >> 6;

	// TODO: Investigate why performing these unneeded reads seems to improve performance by a couple fps on vita
	/*uint8_t scx = mmu->directRead(0xFF43);
	uint8_t scy = mmu->directRead(0xFF42);
	
	uint8_t wx = mmu->directRead(0xFF4B);
	uint8_t wy = mmu->directRead(0xFF4A);*/

	// Update the background scanline
	uint32_t bgBuffer[168];
	int bgStartingIndex = -1;
	updateBackgroundScanline(bgBuffer, &bgStartingIndex);

	// Update the window scanline
	uint32_t winBuffer[168];
	int winStartingIndex = -8;
	updateWindowScanline(winBuffer, &winStartingIndex);

	// Iterate through every pixel on the current scanline and set to the correct value from the
	// background, window, and objects buffers.
	for (int x = 0; x < 160; x++) {
		uint16_t si = ly * 160 + x;

		// Add background scanline to screen buffer
		screenBuffer[si] = bgBuffer[x + bgStartingIndex];

		// Add window scanline to screen buffer
		if (lcdc5) {
			if (x - winStartingIndex >= 0) {
				screenBuffer[si] = winBuffer[x - winStartingIndex];
			}
		}

		if (!lcdc0) {
			screenBuffer[si] = palette[0];
		}

		if (lcdc1) {
			// Get objects from the section of the object buffer that overlaps with the screen
			// Last in order so they have highest priority
			uint16_t oi = ((ly + 16) * 256 + (x + 8)) % 65536;
			uint32_t obj = objectsBuffer[oi];
			
			// Set the pixel to the object pixel if it is not 0 (transparent), and there is either no
			// priority bit in that location, or if there is a priority bit, the current value of the screen
			// (from the background and window) is equal to color 0.
			if (obj != 0 && (!objectsPriorityBuffer[oi] || screenBuffer[si] == palette[bgp[0]])) {
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
