#include <fstream>

#include "Cartridge.h"


Cartridge::Cartridge(const std::string& fileName) {	
	// Initialize with 0s
	rom.fill(0x00);

	// Load file into rom array
	std::ifstream ifs;
	ifs.open(fileName, std::ifstream::binary);
	if (ifs.is_open()) {
		ifs.read((char*)rom.data(), rom.size());
		ifs.close();
	}
}

Cartridge::~Cartridge() {

}

uint8_t Cartridge::read(uint16_t addr) {
	return rom[addr];
}