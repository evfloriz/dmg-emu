#pragma once

#include <cstdint>

class MMU;

class APU {
public:
	APU(MMU* mmu);

	void clock();
	void fillBuffer(float* stream, int len);

private:
	void updateChannel2();

private:
	MMU* mmu = nullptr;

	static const int size = 2048;
	
	float sinIndex = 0;
	float output[size] = {};
	float tone = 440;
	
	// Values correspond to the value the sine wave must be greater than to convert into a square wave at each duty cycle
	// 12.5%, 25%, 50%, 75%
	float wavePatternDuty[4] = { 0.924f, 0.707f, 0.0f, -0.707f };
	
	// Channel 2 data
	uint8_t waveIndex = 0;
	uint16_t soundLengthCounter = 0;
	uint32_t envelopeCounter = 0;
	uint8_t soundOn = 0;
	uint8_t volume2 = 0x00;
	
	float volume = 0.01f;

	uint16_t readPos = 0;
	uint16_t writePos = 0;
};
