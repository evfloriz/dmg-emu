#pragma once

#include "MBC.h"

class MBC0 : public MBC {
public:
	uint32_t mapAddr(uint16_t addr) override;
	void setRegister(uint16_t addr, uint8_t data) override;
};
