
#include <iostream>
#include <SDL.h>

#include "DMG.h"

class Demo {
	
	/*
	Helpful resources:
		https://github.com/DOOMReboot/PixelPusher/blob/master/PixelPusher.cpp
		LazyFoo tutorials
	*/

public:
	Demo() {
	}

	int init() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
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
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DMG_WIDTH, DMG_HEIGHT);
		if (texture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		debugTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, DEBUG_WIDTH, DEBUG_HEIGHT);
		if (debugTexture == NULL) {
			std::cout << "Texture could not be created. SDL_Error: " << SDL_GetError() << std::endl;
			close();
			return -1;
		}

		dmg.init();

		return 0;
	}

	int execute() {
		SDL_Event e;
		
		bool quit = false;
		while (!quit) {			
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
				// TODO: Use this as an example for actual input.
				/*else if (e.type == SDL_KEYDOWN) {
					switch (e.key.keysym.sym) {
					case SDLK_UP:
						//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0x00));
						break;
					
					case SDLK_DOWN:
						//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0x00, 0x00));
						break;

					case SDLK_LEFT:
						//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0xFF, 0x00));
						break;

					case SDLK_RIGHT:
						//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0x00, 0x00, 0xFF));
						break;

					default:
						//SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
					}
				}*/
			}


			// Keep track of performance for fps display
			uint64_t start = SDL_GetPerformanceCounter();

			// Execute one full frame of the Game Boy
			dmg.tick_frame();
			
			// Render the result in the pixel buffer (and debug pixel buffer)
			render();

			// Display fps
			uint64_t end = SDL_GetPerformanceCounter();
			float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency();
			std::string fps = std::to_string((int)(1.0f / elapsed));
			std::cout << fps << "\r" << std::flush;
			//std::cout << dmg.mmu.cpu.global_cycles << "\r" << std::flush;
		}
		return 0;
	}

	int render() {
		// TODO: Figure out what exactly pitch does. Should there be multiple pitches?
		int pitch_1 = 0;
		int pitch_2 = 0;

		// Use the pixel buffer on the ppu for SDL_LockTexture
		uint32_t* pixelBuffer = dmg.ppu.getPixelBuffer();

		// Use the tile data buffer on the ppu
		uint32_t* tileDataBuffer = dmg.ppu.getTileDataBuffer();

		// Initialize rects for various rendering textures
		SDL_Rect srcRect;
		srcRect.x = 0;
		srcRect.y = 0;
		srcRect.w = DMG_WIDTH;
		srcRect.h = DMG_HEIGHT;

		SDL_Rect destRect;
		destRect.x = 0;
		destRect.y = 0;
		destRect.w = DMG_WIDTH * SCREEN_SCALE;
		destRect.h = DMG_HEIGHT * SCREEN_SCALE;

		SDL_Rect debugSrcRect;
		debugSrcRect.x = 0;
		debugSrcRect.y = 0;
		debugSrcRect.w = DEBUG_WIDTH;
		debugSrcRect.h = DEBUG_HEIGHT;

		SDL_Rect debugDestRect;
		debugDestRect.x = DMG_WIDTH * SCREEN_SCALE;
		debugDestRect.y = 0;
		debugDestRect.w = DEBUG_WIDTH * SCREEN_SCALE;
		debugDestRect.h = DEBUG_HEIGHT * SCREEN_SCALE;

		// Process screen texture
		if (SDL_LockTexture(texture, nullptr, (void**)&pixelBuffer, &pitch_1)) {
			std::cout << "Texture could not be locked. SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}

		pitch_1 /= sizeof(uint32_t);
		dmg.ppu.updateTileMap(pixelBuffer);		
		
		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, &srcRect, &destRect);

		// Process debug texture (tile data)
		if (SDL_LockTexture(debugTexture, nullptr, (void**)&tileDataBuffer, &pitch_2)) {
			std::cout << "Texture could not be locked. SDL_Error: " << SDL_GetError() << std::endl;
			return -1;
		}

		pitch_2 /= sizeof(uint32_t);
		dmg.ppu.updateTileData(tileDataBuffer);

		SDL_UnlockTexture(debugTexture);
		SDL_RenderCopy(renderer, debugTexture, &debugSrcRect, &debugDestRect);
		
		SDL_RenderPresent(renderer);

		return 0;
	}

	void close() {
		if (texture) {
			SDL_DestroyTexture(texture);
			texture = nullptr;
		}

		if (renderer) {
			SDL_DestroyRenderer(renderer);
			renderer = nullptr;
		}

		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}
		
		SDL_Quit();
	}


private:
	const int SCREEN_SCALE = 2;

	//const int DMG_WIDTH = 160;
	//const int DMG_HEIGHT = 144;
	const int DMG_WIDTH = 256;
	const int DMG_HEIGHT = 256;

	// Used for the tile data
	const int DEBUG_WIDTH = 256;
	const int DEBUG_HEIGHT = 384;

	const int SCREEN_WIDTH = 1000;
	const int SCREEN_HEIGHT = 800;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
	SDL_Texture* debugTexture = nullptr;

	DMG dmg;

};

int main(int argc, char **argv) {
	Demo demo;

	demo.init();
	demo.execute();
	demo.close();

    return 0;
}
