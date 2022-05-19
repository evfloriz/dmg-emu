#pragma once

#include <cstdint>

class APU {
public:
	APU();

	void clock();
	void fillBuffer(float* stream, int len);

private:
	static const int size = 2000;
	
	float sinIndex = 0;
	float output[size] = {};
	float tone = 440;

	uint16_t readPos = 0;
	uint16_t writePos = 0;
};
