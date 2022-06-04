#pragma once

#include <cstdint>

class MMU;

class APU {
public:
	APU(MMU* mmu);

	void clock();
	void fillBuffer(float* stream, int len);
	void toggleSound(uint8_t data);
	
	int getPosDifference();

private:
	void updateChannel1();
	void updateChannel2();
	void updateChannel3();
	void updateChannel4();

	void updateControl();

private:
	MMU* mmu = nullptr;

	static const int size = 4096;
	float outputSO2[size] = {};
	float outputSO1[size] = {};

	float sampleCounter = 0;
	float sampleTicks = 1048576.0f / 44100.0f;
	float fasterTicks = 1048576.0f / (44100.0f * 1.005f);
	float slowerTicks = 1048576.0f / (44100.0f * 0.995f);

	float volume = 0.001f;
	uint8_t soundOn = 1;

	// Left and right sound output data
	uint8_t volumeSO2 = 0;
	uint8_t volumeSO1 = 0;
	uint8_t selectionSO2[4] = {};
	uint8_t selectionSO1[4] = {};

	uint16_t readPos = 0;
	uint16_t writePos = 0;
	
	// Channel 1 data
	float buffer1[1024] = {};
	uint8_t soundOn1 = 0;
	uint8_t volume1 = 0x00;
	float sample1 = 0.0f;

	uint16_t soundLengthCounter1 = 0;
	uint32_t sweepCounter1 = 0;
	uint32_t envelopeCounter1 = 0;
	uint32_t frequencyCounter1 = 0;
	uint16_t bufferIndex1 = 0;
	uint8_t waveIndex1 = 0;
	
	// Channel 2 data
	float buffer2[1024] = {};
	uint8_t soundOn2 = 0;
	uint8_t volume2 = 0x00;
	float sample2 = 0.0f;
	
	uint16_t soundLengthCounter2 = 0;
	uint32_t envelopeCounter2 = 0;
	uint32_t frequencyCounter2 = 0;
	uint16_t bufferIndex2 = 0;
	uint8_t waveIndex2 = 0;

	// Channel 3 data
	float buffer3[1024] = {};
	uint8_t soundOn3 = 0;
	uint8_t volume3 = 0x00;
	float sample3 = 0.0f;
	
	uint16_t soundLengthCounter3 = 0;
	uint32_t frequencyCounter3 = 0;
	uint16_t bufferIndex3 = 0;
	uint8_t waveIndex3 = 0;

	// Channel 4 data
	float buffer4[1024] = {};
	uint8_t soundOn4 = 0;
	uint8_t volume4 = 0x00;
	float sample4 = 0.0f;
	uint16_t shiftRegister = 0xFFFF;
	
	uint16_t soundLengthCounter4 = 0;
	uint32_t envelopeCounter4 = 0;
	uint32_t frequencyCounter4 = 0;
	uint16_t bufferIndex4 = 0;

public:
	// Debug related information
	int readCounter = 0;
	int writeCounter = 0;
	int writesDropped = 0;
	int readsDropped = 0;

};
