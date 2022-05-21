
#include <iostream>
#include <SDL.h>

#include "Util.h"

#include "DMG.h"

class Demo {
	
	/*
	Helpful resources:
		https://github.com/DOOMReboot/PixelPusher/blob/master/PixelPusher.cpp
		LazyFoo tutorials
	*/

public:
	Demo() {
		// Initialize rects for various rendering textures
		srcScreenRect = { 0, 0, DMG_WIDTH, DMG_HEIGHT };
		destScreenRect = { 0, 0, DMG_WIDTH * SCREEN_SCALE, DMG_HEIGHT * SCREEN_SCALE };

		srcTileDataRect = { 0, 0, TILE_DATA_WIDTH, TILE_DATA_HEIGHT };
		destTileDataRect = { SCREEN_WIDTH - (TILE_DATA_WIDTH * TILE_SCALE), 0, TILE_DATA_WIDTH * TILE_SCALE, TILE_DATA_HEIGHT * TILE_SCALE };

		srcBackgroundRect = { 0, 0, MAP_WIDTH, MAP_HEIGHT };
		destBackgroundRect = { 0, SCREEN_HEIGHT - MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT };

		srcWindowRect = { 0, 0, MAP_WIDTH, MAP_HEIGHT };
		destWindowRect = { MAP_WIDTH, SCREEN_HEIGHT - MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT };

		srcObjectsRect = { 0, 0, MAP_WIDTH, MAP_HEIGHT };
		destObjectsRect = { MAP_WIDTH * 2, SCREEN_HEIGHT - MAP_HEIGHT, MAP_WIDTH, MAP_HEIGHT };
	}

	int init() {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
			std::cout << "SDL could not initialize. SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}
		
		window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

		tileDataTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, TILE_DATA_WIDTH, TILE_DATA_HEIGHT);
		if (tileDataTexture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		backgroundTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, MAP_WIDTH, MAP_HEIGHT);
		if (backgroundTexture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		windowTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, MAP_WIDTH, MAP_HEIGHT);
		if (windowTexture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		objectsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, MAP_WIDTH, MAP_HEIGHT);
		if (objectsTexture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		audioSpec.freq = 44100;
		audioSpec.format = AUDIO_F32SYS;
		audioSpec.channels = 1;
		audioSpec.samples = 1024;
		audioSpec.callback = fillAudioBuffer;
		audioSpec.userdata = &dmg;
		//audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
		audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);
		//audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, SDL_AUDIO_ALLOW_SAMPLES_CHANGE); // 16384 -> 1764
		//audioDevice = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, SDL_AUDIO_ALLOW_CHANNELS_CHANGE); // 16384 -> 32768
		//SDL_AUDIO_ALLOW_FORMAT_CHANGE - unchange
		//SDL_AUDIO_ALLOW_FREQUENCY_CHANGE - unchanged
		if (audioDevice == NULL) {
			std::cout << "Audio device error. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		SDL_PauseAudioDevice(audioDevice, 0);

		// Start dmg
		dmg.init();

		return 0;
	}

	int execute() {
		SDL_Event e;
		const uint8_t* keyboardState = SDL_GetKeyboardState(NULL);
		
		bool quit = false;
		while (!quit) {
			// Individual inputs
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
			}

			// TODO: Is this way of handling input overengineered?
			// 0 is pressed, 1 is unpressed
			dmg.mmu.writeDirectionButton(0, !keyboardState[SDL_SCANCODE_RIGHT]);
			dmg.mmu.writeDirectionButton(1, !keyboardState[SDL_SCANCODE_LEFT]);
			dmg.mmu.writeDirectionButton(2, !keyboardState[SDL_SCANCODE_UP]);
			dmg.mmu.writeDirectionButton(3, !keyboardState[SDL_SCANCODE_DOWN]);

			dmg.mmu.writeActionButton(0, !keyboardState[SDL_SCANCODE_Z]);		// A
			dmg.mmu.writeActionButton(1, !keyboardState[SDL_SCANCODE_X]);		// B
			dmg.mmu.writeActionButton(2, !keyboardState[SDL_SCANCODE_S]);		// Select
			dmg.mmu.writeActionButton(3, !keyboardState[SDL_SCANCODE_A]);		// Start

			// Keep track of performance for fps display
			uint64_t start = SDL_GetPerformanceCounter();

			// Execute one full frame of the Game Boy
			dmg.tickFrame();
			
			// Render the result in the pixel buffer (and debug pixel buffer)
			render();

			// Calculate fps
			uint64_t end = SDL_GetPerformanceCounter();
			float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();
			
			// Delay until 16.666f ms have past (for 60 fps)
			float elapsedMS = elapsed * 1000.0f;
			if (elapsedMS < 16.666f) {
				SDL_Delay(static_cast<uint32_t>(16.666f - elapsedMS));
			}

			uint64_t capped_end = SDL_GetPerformanceCounter();
			float capped_elapsed = (capped_end - start) / (float)SDL_GetPerformanceFrequency();
			
			// Display both the capped and uncapped fps
			std::string fps = std::to_string((int)(1.0f / elapsed));
			std::string capped_fps = std::to_string((int)(1.0f / capped_elapsed));
			std::cout << capped_fps << " | " << fps << "\r" << std::flush;

		}
		return 0;
	}

	int render() {
		auto renderTexture = [&](uint32_t* buffer, SDL_Texture* texture, SDL_Rect srcRect, SDL_Rect destRect, uint32_t size) {
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
		};
		
		// Process screen texture
		renderTexture(
			dmg.ppu.getScreenBuffer(),
			screenTexture,
			srcScreenRect,
			destScreenRect,
			DMG_WIDTH * DMG_HEIGHT);
		
		// Process tile data texture
		renderTexture(
			dmg.ppu.getTileDataBuffer(),
			tileDataTexture,
			srcTileDataRect,
			destTileDataRect,
			TILE_DATA_WIDTH * TILE_DATA_HEIGHT);

		// Process background texture
		renderTexture(
			dmg.ppu.getBackgroundBuffer(),
			backgroundTexture,
			srcBackgroundRect,
			destBackgroundRect,
			MAP_WIDTH * MAP_HEIGHT);

		// Process window texture
		renderTexture(
			dmg.ppu.getWindowBuffer(),
			windowTexture,
			srcWindowRect,
			destWindowRect,
			MAP_WIDTH * MAP_HEIGHT);

		// Process objects texture
		renderTexture(
			dmg.ppu.getObjectsBuffer(),
			objectsTexture,
			srcObjectsRect,
			destObjectsRect,
			MAP_WIDTH * MAP_HEIGHT);
		
		SDL_RenderPresent(renderer);

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
		
		if (tileDataTexture) {
			SDL_DestroyTexture(tileDataTexture);
			tileDataTexture = nullptr;
		}
		
		if (backgroundTexture) {
			SDL_DestroyTexture(backgroundTexture);
			backgroundTexture = nullptr;
		}
		
		if (windowTexture) {
			SDL_DestroyTexture(windowTexture);
			windowTexture = nullptr;
		}

		if (objectsTexture) {
			SDL_DestroyTexture(objectsTexture);
			objectsTexture = nullptr;
		}

		if (renderer) {
			SDL_DestroyRenderer(renderer);
			renderer = nullptr;
		}

		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}

		SDL_CloseAudioDevice(audioDevice);
		
		SDL_Quit();
	}


private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* screenTexture = nullptr;
	SDL_Texture* tileDataTexture = nullptr;
	SDL_Texture* backgroundTexture = nullptr;
	SDL_Texture* windowTexture = nullptr;
	SDL_Texture* objectsTexture = nullptr;

	SDL_Rect srcScreenRect;
	SDL_Rect destScreenRect;
	SDL_Rect srcTileDataRect;
	SDL_Rect destTileDataRect;

	SDL_Rect srcBackgroundRect;
	SDL_Rect destBackgroundRect;
	SDL_Rect srcWindowRect;
	SDL_Rect destWindowRect;
	SDL_Rect srcObjectsRect;
	SDL_Rect destObjectsRect;

	SDL_AudioDeviceID audioDevice;
	SDL_AudioSpec audioSpec;

	DMG dmg;

};

int main(int argc, char **argv) {
	Demo demo;

	demo.init();
	demo.execute();
	demo.close();

    return 0;
}
