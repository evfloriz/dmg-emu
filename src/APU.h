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

	void triggerChannel1();
	void triggerChannel2();
	void triggerChannel3();
	void triggerChannel4();

private:
	void updateChannel1();
	void updateChannel2();
	void updateChannel3();
	void updateChannel4();

	void updateFrameSequencer();

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

	uint16_t readPos = 0;
	uint16_t writePos = 0;

	// TODO: Consider switching channel operation to be more object oriented
	struct Channel {
		float buffer[1024] = {};
		uint8_t soundOn = 0;
		uint8_t volume = 0;
		uint8_t sample = 0;		// (0 to 16 for -1.0 to 1.0)

		uint32_t lengthCounter = 0;
		uint32_t sweepCounter = 0;
		uint32_t envelopeCounter = 0;
		uint32_t frequencyCounter = 0;
		uint16_t bufferIndex = 0;
		uint8_t waveIndex = 0;

		uint8_t shiftAmount = 0;
		uint8_t divisor = 0;
		uint8_t widthMode = 0;

		uint32_t frequencyPeriod = 0;
		uint8_t dacPower = 0;
		uint8_t bufferSample = 0;

		uint8_t so2 = 0;
		uint8_t so1 = 0;
	};

	uint16_t shiftRegister = 0xFFFF;

	// Frame sequencer state information
	uint32_t fsCounter = 0;
	uint8_t fsStep = 0;
	bool lengthCtrClock = false;
	bool volEnvClock = false;
	bool sweepClock = false;

	// Various arrays to be indexed
	uint8_t squareWaveRatio[4] = { 1, 2, 4, 6 };
	uint8_t waveVolume[4] = { 0, 0x0F, 0x07, 0x03 };		// 0%, 100%, 50%, 25%
	//uint8_t noiseDivisor[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

public:
	Channel channel1;
	Channel channel2;
	Channel channel3;
	Channel channel4;

	// Left and right sound output data
	uint8_t volumeSO2 = 0;
	uint8_t volumeSO1 = 0;
	//uint8_t selectionSO2[4] = {};
	//uint8_t selectionSO1[4] = {};

	uint8_t noiseDivisor[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

public:
	// Debug related information
	int readCounter = 0;
	int writeCounter = 0;
	int writesDropped = 0;
	int readsDropped = 0;

};
