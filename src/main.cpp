/*
	License (OLC-3)
	~~~~~~~~~~~~~~~

	Copyright 2018-2019 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.

	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#define OLC_PGE_APPLICATION

#include <iostream>

#include "Bus.h"
#include "CPU.h"
#include "olcPixelGameEngine.h"

class Demo : public olc::PixelGameEngine {
public:
	Demo() {
		sAppName = "dmg cpu demonstration";
	}

	Bus dmg;

	// don't worry about disassembly
	//std::map<uint16_t, std::string> mapAsm;

	std::string hex(uint32_t n, uint8_t d) {
		std::string s(d, '0');
		for (int i = d - 1; i >= 0; i--, n >>= 4) {
			s[i] = "0123456789ABCDEF"[n & 0xF];
		}
		return s;
	}

	void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns) {
		int nRamX = x;
		int nRamY = y;

		for (int row = 0; row < nRows; row++) {
			std::string sOffset = "$" + hex(nAddr, 4) + ":";
			for (int col = 0; col < nColumns; col++) {
				sOffset += " " + hex(dmg.read(nAddr), 2);
				nAddr++;
			}
			DrawString(nRamX, nRamY, sOffset);
			nRamY += 10;
		}
	}

	void DrawCPU(int x, int y) {
		std::string status = "STATUS: ";
		DrawString(x, y, "STATUS:", olc::WHITE);

		DrawString(x + 64, y, "Z", dmg.cpu.f & CPU::Z ? olc::GREEN : olc::RED);
		DrawString(x + 80, y, "N", dmg.cpu.f & CPU::N ? olc::GREEN : olc::RED);
		DrawString(x + 96, y, "H", dmg.cpu.f & CPU::H ? olc::GREEN : olc::RED);
		DrawString(x + 112, y, "C", dmg.cpu.f & CPU::C ? olc::GREEN : olc::RED);

		DrawString(x, y + 10, "PC: $" + hex(dmg.cpu.pc, 4));
		DrawString(x, y + 20, "A: $" + hex(dmg.cpu.a, 2) + " [" + std::to_string(dmg.cpu.a) + "]");
		DrawString(x, y + 30, "F: $" + hex(dmg.cpu.f, 2) + " [" + std::to_string(dmg.cpu.f) + "]");
		DrawString(x, y + 40, "B: $" + hex(dmg.cpu.b, 2) + " [" + std::to_string(dmg.cpu.b) + "]");
		DrawString(x, y + 50, "C: $" + hex(dmg.cpu.c, 2) + " [" + std::to_string(dmg.cpu.c) + "]");
		DrawString(x, y + 60, "D: $" + hex(dmg.cpu.d, 2) + " [" + std::to_string(dmg.cpu.d) + "]");
		DrawString(x, y + 70, "E: $" + hex(dmg.cpu.e, 2) + " [" + std::to_string(dmg.cpu.e) + "]");
		DrawString(x, y + 80, "H: $" + hex(dmg.cpu.h, 2) + " [" + std::to_string(dmg.cpu.h) + "]");
		DrawString(x, y + 90, "L: $" + hex(dmg.cpu.l, 2) + " [" + std::to_string(dmg.cpu.l) + "]");
		DrawString(x, y + 100, "SP: $" + hex(dmg.cpu.sp, 4));
	}

	bool OnUserCreate() {
		// anything to do? right now I'm reading the file just in main
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) {
		Clear(olc::DARK_BLUE);

		/*if (GetKey(olc::Key::SPACE).bPressed) {
			do {
				dmg.cpu.clock();
			} while (!dmg.cpu.complete());
		}*/

		// what exactly should I draw for the bootrom?
		DrawRam(2, 2, 0x0000, 16, 16);
		DrawRam(2, 182, 0x8000, 16, 16);
		DrawCPU(448, 2);

		return true;
	}

};

int main() {
    // load boot rom

	unsigned char memory[0x100];
	FILE* file = fopen("DMG_ROM.bin", "rb");
	int pos = 0;
	while (fread(&memory[pos], 1, 1, file)) {
		pos++;
	}
	fclose(file);

	/*for (auto m : memory) {
		printf("%x\n", m);
	}*/

	Demo demo;
	demo.Construct(680, 480, 2, 2);
	demo.Start();

    return 0;
}