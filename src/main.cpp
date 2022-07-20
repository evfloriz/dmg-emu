#include <iostream>

#include "Util.h"
#include "DMG.h"

#ifdef VITA
#include <psp2/kernel/processmgr.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "debugScreen.h"

#define printf psvDebugScreenPrintf
#else
#include <SDL.h>
#include <SDL_ttf.h>
#endif

class Demo {
public:
	Demo() {
		using namespace util;

		windowName = "dmg-emu";
		int x = (SCREEN_WIDTH - DMG_WIDTH * pixelScale) / 2;
		int y = (SCREEN_HEIGHT - DMG_HEIGHT * pixelScale) / 2;

		srcScreenRect = { 0, 0, DMG_WIDTH, DMG_HEIGHT };
		destScreenRect = { x, y, DMG_WIDTH * pixelScale, DMG_HEIGHT * pixelScale };

		messageRect = { 0, 0, 64, 32 };
		fontColor = { 255, 255, 255 };
		fontSize = 24;
	}

	~Demo() {
		close();
	}

	int init() {
		using namespace util;

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
			std::cout << "SDL could not initialize. SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}
		
		window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			std::cout << "Window could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		// Should be good to use hardware acceleration exclusively for now
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == NULL) {
			std::cout << "Renderer could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		// TODO: What's the advantage of different pixel formats?
		screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DMG_WIDTH, DMG_HEIGHT);
		if (screenTexture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		audioSpec.freq = 44100;
		audioSpec.format = AUDIO_F32SYS;
		audioSpec.channels = 2;
		audioSpec.samples = 2048;
		audioSpec.callback = fillAudioBuffer;
		audioSpec.userdata = &dmg;
		
		// TODO: Investigate audio device flags
		audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);
		
		if (audioDevice == 0) {
			std::cout << "Audio device error. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		SDL_PauseAudioDevice(audioDevice, 0);

#ifdef VITA
		gameController = SDL_GameControllerOpen(0);
		if (gameController == NULL) {
			std::cout << "Game controller error. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}
#endif

		// Init TTF
		if (TTF_Init() == -1) {
			std::cout << "SDL_ttf could not initialize. TTF Error: " << TTF_GetError() << std::endl;
			return -1;
		}

		// Open font
		font = TTF_OpenFont("fonts/consola.ttf", fontSize);
		if (font == NULL) {
			printf("Failed to open font. TTF Error: %s\n", TTF_GetError());
			return -1;
		}

		// Start dmg
		dmg.init();

		return 0;
	}

	int execute() {
		SDL_Event e;

#ifndef VITA
		const uint8_t* keyboardState = SDL_GetKeyboardState(NULL);
#endif

		int fpsRenderCounter = 0;
		double totalElapsed = 0.0;
		
		bool quit = false;
		while (!quit) {
			// Individual inputs
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
			}

#ifdef VITA
			dmg.mmu.writeDirectionButton(0, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
			dmg.mmu.writeDirectionButton(1, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
			dmg.mmu.writeDirectionButton(2, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_UP));
			dmg.mmu.writeDirectionButton(3, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN));

			dmg.mmu.writeActionButton(0, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_B));		// A
			dmg.mmu.writeActionButton(1, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_A));		// B
			dmg.mmu.writeActionButton(2, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_BACK));		// Select
			dmg.mmu.writeActionButton(3, !SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_START));	// Start
#else
			// 0 is pressed, 1 is unpressed
			dmg.mmu.writeDirectionButton(0, !keyboardState[SDL_SCANCODE_RIGHT]);
			dmg.mmu.writeDirectionButton(1, !keyboardState[SDL_SCANCODE_LEFT]);
			dmg.mmu.writeDirectionButton(2, !keyboardState[SDL_SCANCODE_UP]);
			dmg.mmu.writeDirectionButton(3, !keyboardState[SDL_SCANCODE_DOWN]);

			dmg.mmu.writeActionButton(0, !keyboardState[SDL_SCANCODE_Z]);		// A
			dmg.mmu.writeActionButton(1, !keyboardState[SDL_SCANCODE_X]);		// B
			dmg.mmu.writeActionButton(2, !keyboardState[SDL_SCANCODE_S]);		// Select
			dmg.mmu.writeActionButton(3, !keyboardState[SDL_SCANCODE_A]);		// Start
#endif

			// TODO: Clean up main loop

			// Keep track of start time for each frame and every 60 frames
			uint64_t start = SDL_GetPerformanceCounter();

			// Execute one full frame of the Game Boy
			dmg.tickFrame();
			
			// Render the result in the pixel buffer
			render();

			// Calculate elapsed time
			uint64_t end = SDL_GetPerformanceCounter();
			double elapsed = (end - start) / (double)SDL_GetPerformanceFrequency();
			totalElapsed += elapsed;

			// Delay until 16.666 ms have passed
			while (elapsed < 0.016666) {
				end = SDL_GetPerformanceCounter();
				elapsed = (end - start) / (double)SDL_GetPerformanceFrequency();
			}			
			
			// Render fps at specified rate using the average elapsed time over the last quantity of frames
			if (fpsRenderCounter == 0) {
				fpsRenderCounter = fpsRenderTicks;
				if (util::displayFPS) {
					std::string uncappedFPS = std::to_string((int)(1.0 / (totalElapsed / fpsRenderTicks)));
					updateText(uncappedFPS);
				}
				totalElapsed = 0.0;
			}
			fpsRenderCounter--;
		}
		return 0;
	}

	int updateText(std::string text) {
		// Free surface and destroy texture if they exist
		if (textSurface) {
			SDL_FreeSurface(textSurface);
		}

		if (textTexture) {
			SDL_DestroyTexture(textTexture);
		}

		// Solid is fastest but looks the worst
		textSurface = TTF_RenderText_Solid(font, text.c_str(), fontColor);
		textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

		messageRect.w = textSurface->w;
		messageRect.h = textSurface->h;

		return 0;
	}
	
	int render() {	
		SDL_RenderClear(renderer);

		// Render fps counter
		if (util::displayFPS) {
			SDL_RenderCopy(renderer, textTexture, NULL, &messageRect);
		}
		
		// Process screen texture
		renderTexture(
			dmg.ppu.getScreenBuffer(),
			screenTexture,
			srcScreenRect,
			destScreenRect,
			util::DMG_WIDTH * util::DMG_HEIGHT);

		SDL_RenderPresent(renderer);

		return 0;
	}

	int renderTexture(uint32_t* buffer, SDL_Texture* texture, SDL_Rect srcRect, SDL_Rect destRect, uint32_t size) {
		// TODO: Figure out what exactly pitch does. Should there be multiple pitches?
		uint32_t* pixels = nullptr;

		int pitch = 0;

		if (SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch)) {
			std::cout << "Texture could not be locked. SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}

		pitch /= sizeof(uint32_t);

		std::copy(buffer, buffer + size, pixels);

		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, &srcRect, &destRect);

		return 0;
	}

	static void fillAudioBuffer(void* userdata, uint8_t* stream, int len) {
		DMG* dmg = (DMG*)userdata;
		float* buffer = (float*)stream;
		len /= 4;			// since the data is in floats

		dmg->apu.fillBuffer(buffer, len);
	}

	void close() {
		if (screenTexture) {
			SDL_DestroyTexture(screenTexture);
			screenTexture = nullptr;
		}

		if (renderer) {
			SDL_DestroyRenderer(renderer);
			renderer = nullptr;
		}

		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}

		if (audioDevice) {
			SDL_CloseAudioDevice(audioDevice);
		}

#ifdef VITA
		if (gameController) {
			SDL_GameControllerClose(gameController);
		}
#endif

		if (textSurface) {
			SDL_FreeSurface(textSurface);
			textSurface = nullptr;
		}

		if (textTexture) {
			SDL_DestroyTexture(textTexture);
			textTexture = nullptr;
		}

		TTF_CloseFont(font);

		TTF_Quit();
		
		SDL_Quit();
	}

private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* screenTexture = nullptr;
	
	SDL_Rect srcScreenRect;
	SDL_Rect destScreenRect;

	SDL_Rect messageRect;

	SDL_AudioDeviceID audioDevice;
	SDL_AudioSpec audioSpec;

#ifdef VITA
	SDL_GameController* gameController;
#endif

	TTF_Font* font;
	SDL_Color fontColor;
	int fontSize;

	int screenWidth;
	int screenHeight;

	std::string windowName;

	DMG dmg;

	bool renderFPS = false;
	int fpsRenderTicks = 30;

	SDL_Surface* textSurface = NULL;
	SDL_Texture* textTexture = NULL;
};

int main(int argc, char **argv) {
#ifdef VITA
	psvDebugScreenInit();
#endif

	if (util::readOptionsFile() == -1) {
		printf("Exiting...\n");
#ifdef VITA
		sceKernelDelayThread(3 * 1000000);
		sceKernelExitProcess(0);
#endif
		return 1;
	}

	Demo demo;

	demo.init();
	demo.execute();
	demo.close();

#ifdef VITA
	sceKernelExitProcess(0);
#endif
    return 0;
}
