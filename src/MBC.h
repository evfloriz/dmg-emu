#pragma once

#include <cstdint>

class MBC {
public:
	virtual uint32_t mapAddr(uint16_t addr) = 0;
	virtual void setRegister(uint16_t addr, uint8_t data) = 0;

protected:
	uint8_t romBanks = 0;
	uint8_t ramBanks = 0;
	
	// State of the MBC chip
	bool ramEnable = 0;
	uint8_t romBankNumber = 1;		// This defaults to 1, since 0 is treated as 1 for MBC1
	uint8_t ramBankNumber = 0;
	uint8_t upperRomBankNumber = 0;
	uint8_t modeSelect = 0;

	const uint32_t ROM_BANK_SIZE = 16 * 1024;
	const uint32_t RAM_BANK_SIZE = 8 * 1024;
};
