#pragma once

#include "MBC.h"

class MBC1 : public MBC {
public:
	uint32_t mapRomAddr(uint16_t addr) override;
	uint32_t mapRamAddr(uint16_t addr) override;
	void setRegister(uint16_t addr, uint8_t data) override;

private:
	uint8_t upperRomBankNumber = 0;
};
