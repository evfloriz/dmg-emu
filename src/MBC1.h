#pragma once

#include "MBC.h"

class MBC1 : public MBC {
public:
	MBC1(uint8_t romBanks, uint8_t ramBanks);

	uint32_t mapRomAddr(uint16_t addr) override;
	uint32_t mapRamAddr(uint16_t addr) override;
	void setRegister(uint16_t addr, uint8_t data) override;
};
