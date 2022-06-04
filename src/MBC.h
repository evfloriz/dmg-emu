#pragma once

#include <cstdint>
#include "Util.h"

class MBC {
public:
	MBC(uint8_t romBanks = 0, uint8_t ramBanks = 0);

	virtual uint32_t mapRomAddr(uint16_t addr) = 0;
	virtual uint32_t mapRamAddr(uint16_t addr) = 0;
	virtual void setRegister(uint16_t addr, uint8_t data) = 0;

protected:
	uint8_t romBanks = 0;
	uint8_t ramBanks = 0;
	
	// State of the MBC chip
	uint8_t ramg = 0;
	uint8_t bank1 = 1;		// This defaults to 1, since 0 is treated as 1 for MBC1
	uint8_t bank2 = 0;
	uint8_t mode = 0;
};
