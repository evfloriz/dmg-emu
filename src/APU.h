#pragma once

#include <cstdint>

class MMU;

class APU {
public:
	APU(MMU* mmu);

	void clock();
	void fillBuffer(float* stream, int len);

private:
	void updateChannel1();
	void updateChannel2();
	void updateChannel3();
	void updateChannel4();

private:
	MMU* mmu = nullptr;

	// One frame is 17556 ticks
	static const int size = 2048;
	//static const int size = 10000;
	float output[size] = {};

	// Values correspond to the value the sine wave must be greater than to convert into a square wave at each duty cycle
	// 12.5%, 25%, 50%, 75%
	float wavePatternDuty[4] = { 0.924f, 0.707f, 0.0f, -0.707f };
	
	// Channel 1 data
	uint32_t sampleIndex1 = 0;
	uint32_t tone1 = 0;
	uint8_t waveRatio1 = 0;
	uint16_t soundLengthCounter1 = 0;
	uint32_t sweepCounter1 = 0;
	uint32_t envelopeCounter1 = 0;
	uint8_t soundOn1 = 0;
	uint8_t volume1 = 0x00;

	uint32_t frequencyCounter1 = 0;
	float buffer1[1024] = {};
	uint16_t bufferIndex1 = 0;

	uint8_t waveIndex1 = 0;
	float sample1 = 0.0f;
	
	// Channel 2 data
	uint32_t sampleIndex2 = 0;
	uint32_t tone2 = 0;
	uint8_t waveRatio2 = 0;
	uint16_t soundLengthCounter2 = 0;
	uint32_t envelopeCounter2 = 0;
	uint8_t soundOn2 = 0;
	uint8_t volume2 = 0x00;

	uint32_t frequencyCounter2 = 0;
	float buffer2[1024] = {};
	uint16_t bufferIndex2 = 0;

	uint8_t waveIndex2 = 0;
	float sample2 = 0.0f;

	// Channel 3 data
	uint32_t sampleIndex3 = 0;
	uint32_t tone3 = 0;
	uint16_t soundLengthCounter3 = 0;
	uint8_t soundOn3 = 0;
	uint8_t volume3 = 0x00;
	uint32_t frequencyCounter3 = 0;
	uint8_t waveSampleIndex3 = 0;
	uint8_t sampleByte = 0;
	float sampleWave3[32] = {};

	float buffer3[1024] = {};
	uint16_t bufferIndex3 = 0;

	uint8_t waveIndex3 = 0;
	float sample3 = 0.0f;

	// Channel 4 data
	uint32_t sampleIndex4 = 0;
	uint32_t tone4 = 0;
	uint16_t soundLengthCounter4 = 0;
	uint32_t envelopeCounter4 = 0;
	uint8_t soundOn4 = 0;
	uint8_t volume4 = 0x00;
	uint32_t frequencyCounter4 = 0;
	uint8_t noiseBit = 0;
	uint16_t shiftRegister = 0xFFFF;

	float buffer4[1024] = {};
	uint16_t bufferIndex4 = 0;
	uint8_t waveIndex4 = 0;
	float sample4 = 0.0f;
	
	
	uint16_t waveSampleIndex4 = 0;
	float sampleWave4[128] = {};
	
	float bufferIndex = 0;
	float sampleCounter = 0;
	float sampleTicks = 1048576.0f / 44100.0f;
	float noiseBitSample = 0;

	float channel4 = 0.0f;
	float channel3 = 0.0f;
	float channel2 = 0.0f;
	float channel1 = 0.0f;

	
	float volume = 0.01f;

	uint16_t readPos = 0;
	uint16_t writePos = 0;

public:
	float lowPassFilter(uint16_t index);
};
