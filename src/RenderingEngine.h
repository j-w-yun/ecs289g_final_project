#pragma once

#include <utility>

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
			Vector2f position = Vector2f(0, 0);
			float zoom = 10;
	};

	Camera cam;

	Vector2f world_to_screen(Vector2f world_vec) {
		Vector2f screen_vec = world_vec.sub(cam.position);
		screen_vec = screen_vec.mul(Vector2f(cam.zoom, cam.zoom*width/height));
		screen_vec = screen_vec.add(Vector2f(width/2, height/2));
		return screen_vec;
	}

	Vector2f screen_to_world(Vector2f screen_vec) {
		Vector2f world_vec = screen_vec.sub(Vector2f(width/2, height/2));
		world_vec = world_vec.div(Vector2f(cam.zoom, cam.zoom*width/height));
		world_vec = world_vec.add(cam.position);
		return world_vec;
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
			cam.zoom += (float)Input::get_scrolly() / 2.0f;
			if (cam.zoom < 0)
				cam.zoom = 0;
			else if (cam.zoom > 100)
				cam.zoom = 100;

			// Pan map
			std::pair<int, int> mp = Input::get_mouse_pos();
			// Pan left
			int dx, dy;
			if (mp.first < 4)
				dx = -1;
			else if (mp.first >= width-4)
				dx = 1;
			if (mp.second < 4)
				dy = -1;
			else if (mp.second >= height-4)
				dy = 1;
			cam.position = cam.position.add(Vector2f(dx, dy));

			// std::cout << "cam position: (" << cam.position.x() << ", " << cam.position.y() << ") cam zoom: " << cam.zoom << std::endl;
			std::cout << cam.position.x() << ", " << cam.position.y() << std::endl;
			// Vector2f world = screen_to_world(Vector2f(mp.first, mp.second));
			// std::cout << "mouse world position: (" << world.x() << ", " << world.y() << ")" << std::endl;
		}

		gWorld.render(gRenderer);
	}

	void show() {
		// Render left mouse drag
		if (Input::has_dragbox(SDL_BUTTON_LEFT)) {
			DragBox box = Input::get_dragbox(SDL_BUTTON_LEFT);
			SDL_Rect dragbox = {box.x1, box.y1, box.x2-box.x1, box.y2-box.y1};
			// Render filled quad
			SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0x33);
			SDL_RenderFillRect(gRenderer, &dragbox);
			// Render outline quad
			SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0xFF);
			SDL_RenderDrawRect(gRenderer, &dragbox);
		}

		// Render right mouse drag
		if (Input::has_dragbox(SDL_BUTTON_RIGHT)) {
			DragBox box = Input::get_dragbox(SDL_BUTTON_RIGHT);
			SDL_Rect dragbox = {box.x1, box.y1, box.x2-box.x1, box.y2-box.y1};
			// Render filled quad
			SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0x44);
			SDL_RenderFillRect(gRenderer, &dragbox);
			// Render outline quad
			SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &dragbox);
		}

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
