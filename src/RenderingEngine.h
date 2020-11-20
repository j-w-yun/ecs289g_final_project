#pragma once

#include "Stat.h"
#include "Stat.cpp"
#include "Input.h"
#include "Input.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "World.h"
#include "World.cpp"

namespace RenderingEngine {

	// Font file
	const char* TTF_FILE = "./res/fonts/Roboto-Medium.ttf";

	// Default window dimensions
	int width = 1200;
	int height = 800;

	// SDL subsystems to free / destory later
	SDL_Window* gWindow;
	SDL_Renderer* gRenderer;
	TTF_Font* gFont;

	class Camera {
		public:
			Vector2f position;
			float zoom = 10;
	};

	Camera cam;

	Vector2f world_to_screen(Vector2f world_vec) {
		// Vector2f screen_vec = (world_vec - cam.position)*(cam.zoom, cam.zoom*width/height);
		Vector2f screen_vec = world_vec - cam.position;
		screen_vec.set(screen_vec.x()*cam.zoom, screen_vec.y()*cam.zoom*width/height);
		screen_vec += Vector2f(width/2, height/2);
		return screen_vec;
	}

	Vector2f screen_to_world(Vector2f screen_vec) {
		// Vector2f screen_vec = world_vec - cam.position;
		// screen_vec *= Vector2f(cam.zoom, cam.zoom*width/height);
		// screen_vec += Vector2f(width/2, height/2);
		return screen_vec;
	}

	void clear() {
		// Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(gRenderer);
	}

	void render(World& gWorld) {
		// Get current window size
		SDL_GetWindowSize(gWindow, &width, &height);
		
		// Zoom map
		if (Input::has_input()) {
			cam.zoom += (float)Input::get_scrolly() / 10.0f;
			if (cam.zoom < 0)
				cam.zoom = 0;

			// Pan map
			std::pair<int, int> mp = Input::get_mouse_pos();
			// Pan left
			int dx, dy;
			if (mp.first < 10)
				dx = -1;
			if (mp.first >= width-10)
				dx = 1;
			if (mp.second < 10)
				dy = -1;
			if (mp.second >= height-10)
				dy = 1;
			cam.position += Vector2f(dx, dy);
		}

		std::cout << "cam position: (" << cam.position.x() << ", " << cam.position.y() << ") cam zoom: " << cam.zoom << std::endl;
		
		gWorld.render(gRenderer);
	}

	void show() {
		// Render mouse dragging
		Input::render(gRenderer);

		// Render stats
		Stat::render(gRenderer, gFont);

		// Update screen
		SDL_RenderPresent(gRenderer);

		// Free stat rendering resources
		Stat::free();
	}

	bool initialize() {
		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("SDL could not initialize: %s\n", SDL_GetError());
			return false;
		}
		// Initialize SDL_ttf
		if(TTF_Init() == -1) {
			printf("SDL_ttf could not initialize: %s\n", TTF_GetError());
			return false;
		}

		// Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			printf("Warning: Linear texture filtering not enabled");

		// Create window
		gWindow = SDL_CreateWindow(
			"Title",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			SDL_WINDOW_SHOWN
		);
		if (gWindow == NULL) {
			printf("Window could not be created: %s\n", SDL_GetError());
			return false;
		}

		// Create renderer for window
		gRenderer = SDL_CreateRenderer(
			gWindow,
			-1,
			SDL_RENDERER_ACCELERATED
		);
		if (gRenderer == NULL) {
			printf("Renderer could not be created: %s\n", SDL_GetError());
			return false;
		}

		// Create font from TrueType Font file
		gFont = TTF_OpenFont(
			TTF_FILE,
			64
		);
		if (gFont == NULL) {
			printf("Failed to load font: %s\n", TTF_GetError());
			return false;
		}

		// Grab mouse
		// SDL_SetWindowGrab(gWindow, SDL_TRUE);
		
		// Set blending
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		// SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

		// Opaque blank screen
		clear();
		show();

		return true;
	}

	void destroy() {
		// Free global font
		TTF_CloseFont(gFont);
		gFont = NULL;

		// Destroy renderer
		SDL_DestroyRenderer(gRenderer);
		gRenderer = NULL;

		// Destroy window
		SDL_DestroyWindow(gWindow);
		gWindow = NULL;

		// Quit SDL subsystems
		TTF_Quit();
		SDL_Quit();
	}
};
