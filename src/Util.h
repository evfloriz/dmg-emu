#pragma once

/*
* This file has constants that are used throughout the emulator, as well some useful inline functions.
*/

// TODO: Should these be put into a namespace?
// TODO: Do a pass on where these should be used vs. where values should be hard coded.

#include <string>
#include <unordered_map>

const int SCREEN_SCALE = 3;
const int TILE_SCALE = 2;

const int DMG_WIDTH = 160;
const int DMG_HEIGHT = 144;

// Used for background and window map
const int MAP_WIDTH = 256;
const int MAP_HEIGHT = 256;

// Used for the tile data
const int TILE_DATA_WIDTH = 128;
const int TILE_DATA_HEIGHT = 192;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;

const uint32_t ROM_BANK_SIZE = 16 * 1024;
const uint32_t RAM_BANK_SIZE = 8 * 1024;

inline uint32_t ARGB(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha = 255) {
	return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

inline bool isNumber(const std::string& str) {
	return (str.find_first_not_of("0123456789") == std::string::npos) && !str.empty();
}

namespace util {
	extern std::unordered_map<std::string, std::string> options;

	int readOptionsFile();
}
