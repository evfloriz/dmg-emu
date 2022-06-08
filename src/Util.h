#pragma once

#include <string>
#include <unordered_map>

namespace util {
	inline uint32_t ARGB(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha = 255) {
		return (alpha << 24) | (red << 16) | (green << 8) | blue;
	}

	inline bool isNumber(const std::string& str) {
		return (str.find_first_not_of("0123456789") == std::string::npos) && !str.empty();
	}
	
	int readOptionsFile();
	
	// Global config options
	extern std::string romPath;
	extern std::string palette;
	extern int pixelScale;
	extern int displayFPS;
	extern int debugMode;
	extern std::string logPath;

	extern std::unordered_map<std::string, std::string> options;

	// TODO: Do a pass on where constants should be used vs where values should be hard coded
	
	// Global constants
	const int DMG_WIDTH = 160;
	const int DMG_HEIGHT = 144;

	const uint32_t ROM_BANK_SIZE = 16 * 1024;
	const uint32_t RAM_BANK_SIZE = 8 * 1024;

	// Debug related constants
	const int DEBUG_SCREEN_SCALE = 3;
	const int TILE_SCALE = 2;

	const int MAP_WIDTH = 256;
	const int MAP_HEIGHT = 256;

	const int TILE_DATA_WIDTH = 128;
	const int TILE_DATA_HEIGHT = 192;

	const int DEBUG_SCREEN_WIDTH = 1000;
	const int DEBUG_SCREEN_HEIGHT = 800;
}
