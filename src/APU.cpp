
#include <iostream>
#include "APU.h"

APU::APU() {

}

void APU::clock() {
	// Store output in a circular buffer with separate read and write positions.
	// Write a new value on every clock tick until the write position is the same as the read position.
	if (writePos == readPos) {
		return;
	}

	output[writePos] = sin(sinIndex);
		
	sinIndex += tone * 3.14f * 2 / 44100;
	if (sinIndex >= 3.14f * 2) {
		sinIndex -= 3.14f * 2;
	}

	// Wrap around to 0 if writePos exceeds the size
	writePos++;
	if (writePos >= size) {
		writePos = 0;
	}
}

void APU::fillBuffer(float* stream, int len) {
	// Set the value of the stream to the output, starting at the current read position and wrapping around
	// if the read position exceeds the size.
	for (int i = 0; i < len; i++) {
		stream[i] = output[readPos];
		readPos++;
		if (readPos >= size) {
			readPos = 0;
		}
	}
}
