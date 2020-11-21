#pragma once

#include <utility>

#include "Stat.h"
#include "Stat.cpp"
#include "Input.h"
#include "Input.cpp"
#include "Map.h"
#include "Map.cpp"
#include "Stat.h"
#include "Stat.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "World.h"
#include "World.cpp"

namespace RenderingEngine {

	// Font file
	const char* TTF_FILE = "./res/fonts/Roboto-Light.ttf";

	// Default window dimensions
	int width = 1200;
	int height = 800;
	float world_width;
	float world_height;

	// SDL subsystems to free / destory later
	SDL_Window* gWindow;
	SDL_Renderer* gRenderer;
	TTF_Font* gFont;

	class Camera {
		public:
			Vector2f position = Vector2f(-1, -1);
			float zoom = 1;
			float get_zoom_factor() {
				return zoom*zoom/100.0f + 0.5f;
			}
	};

	Camera cam;

	Vector2f world_to_screen(Vector2f world_vec) {
		Vector2f screen_vec = world_vec;
		screen_vec = screen_vec.sub(cam.position);
		screen_vec = screen_vec.scale(cam.get_zoom_factor());
		screen_vec = screen_vec.add(width/2, height/2);
		return screen_vec;
	}

	Vector2f world_to_screen(float x, float y) {
		return world_to_screen(Vector2f(x, y));
	}

	Vector2f screen_to_world(Vector2f screen_vec) {
		Vector2f world_vec = screen_vec;
		world_vec = world_vec.sub(width/2, height/2);
		world_vec = world_vec.scale(1.0f/cam.get_zoom_factor());
		world_vec = world_vec.add(cam.position);
		return world_vec;
	}

	Vector2f screen_to_world(float x, float y) {
		return screen_to_world(Vector2f(x, y));
	}

	void clear() {
		// Get current window size
		SDL_GetWindowSize(gWindow, &width, &height);

		// Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(gRenderer);
	}

	const int PANNING_PAD = 10;
	const float PAN_SPEED = 0.3f;
	const float ZOOM_KEY_SPEED = 0.01f;

	Vector2f ldrag_start = Vector2f(-1, -1);
	Vector2f rdrag_start = Vector2f(-1, -1);

	void render(float delta_time, World& gWorld) {
		// Get world size
		MapLevel& level = gWorld.get_level(0);
		world_width = level.get_tile_width() * level.get_width();
		world_height = level.get_tile_height() * level.get_height();

		// Move camera to center if unset
		if (cam.position.x() < 0 && cam.position.y() < 0)
			cam.position.set(world_width/2, world_height/2);

		if (Input::has_input()) {
			// Set stats
			std::pair<int, int> mp = Input::get_mouse_pos();
			Vector2f mouse_screen = Vector2f(mp.first, mp.second);
			Vector2f mouse_world = screen_to_world(mouse_screen);
			Stat::mouse_info(mouse_screen, mouse_world);
			Stat::camera_info(cam.position, cam.zoom);

			// Zoom map
			cam.zoom += (float)Input::get_scrolly();
			if (Input::is_key_pressed(SDLK_PAGEUP))
				cam.zoom += delta_time * ZOOM_KEY_SPEED;
			else if (Input::is_key_pressed(SDLK_PAGEDOWN))
				cam.zoom -= delta_time * ZOOM_KEY_SPEED;
			if (cam.zoom < 1.0f)
				cam.zoom = 1.0f;
			else if (cam.zoom > 30.0f)
				cam.zoom = 30.0f;

			// Pan map
			Vector2f dp = Vector2f(0, 0);
			if (mp.first < PANNING_PAD || Input::is_key_pressed(SDLK_LEFT))
				dp.setx(-PAN_SPEED);
			if (mp.first > width - PANNING_PAD || Input::is_key_pressed(SDLK_RIGHT))
				dp.setx(PAN_SPEED);
			if (mp.second < PANNING_PAD || Input::is_key_pressed(SDLK_UP))
				dp.sety(-PAN_SPEED);
			if (mp.second > height - PANNING_PAD || Input::is_key_pressed(SDLK_DOWN))
				dp.sety(PAN_SPEED);
			dp = dp.scale(delta_time);

			// Clip camera position
			cam.position = cam.position.add(dp);
			if (cam.position.x() < 0)
				cam.position.setx(0);
			if (cam.position.x() > world_width)
				cam.position.setx(world_width);
			if (cam.position.y() < 0)
				cam.position.sety(0);
			if (cam.position.y() > world_height)
				cam.position.sety(world_height);

			// Send click world coordinates to World
			if (Input::is_mouse_pressed(SDL_BUTTON_LEFT) || Input::is_mouse_pressed(SDL_BUTTON_RIGHT)) {
				std::pair<int, int> pos = Input::get_mouse_pos();
				Vector2f world_pos = screen_to_world(pos.first, pos.second);
				int click_type = Input::is_mouse_pressed(SDL_BUTTON_LEFT) ? 0 : 1;
				gWorld.click(world_pos, click_type);
			}
		}

		// Render world
		gWorld.render(gRenderer);

		// Render left mouse drag
		if (Input::has_dragbox(SDL_BUTTON_LEFT)) {
			DragBox box = Input::get_dragbox(SDL_BUTTON_LEFT);
			// Allow panning drag
			if (ldrag_start.x() < 0 && ldrag_start.y() < 0)
				ldrag_start.set(screen_to_world(box.x1, box.y1));
			Vector2f start = world_to_screen(ldrag_start);
			SDL_Rect dragbox = {
				(int)(start.x()),
				(int)(start.y()),
				(int)(box.x2-start.x()),
				(int)(box.y2-start.y())
			};
			// Send drag world coordinates to World
			Vector2f ldrag_end = screen_to_world(box.x2, box.y2);
			gWorld.select(ldrag_start, ldrag_end, 0);
			// Render filled quad
			SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0x55);
			SDL_RenderFillRect(gRenderer, &dragbox);
			// Render outline quad
			SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0xFF);
			SDL_RenderDrawRect(gRenderer, &dragbox);
		}
		else {
			ldrag_start = Vector2f(-1, -1);
		}

		// Render right mouse drag
		if (Input::has_dragbox(SDL_BUTTON_RIGHT)) {
			DragBox box = Input::get_dragbox(SDL_BUTTON_RIGHT);
			// Allow panning drag
			if (rdrag_start.x() < 0 && rdrag_start.y() < 0)
				rdrag_start.set(screen_to_world(box.x1, box.y1));
			Vector2f start = world_to_screen(rdrag_start);
			SDL_Rect dragbox = {
				(int)(start.x()),
				(int)(start.y()),
				(int)(box.x2-start.x()),
				(int)(box.y2-start.y())
			};
			// Send drag world coordinates to World
			Vector2f rdrag_end = screen_to_world(box.x2, box.y2);
			gWorld.select(rdrag_start, rdrag_end, 1);
			// Render filled quad
			SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0x55);
			SDL_RenderFillRect(gRenderer, &dragbox);
			// Render outline quad
			SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &dragbox);
		}
		else {
			rdrag_start = Vector2f(-1, -1);
		}
	}

	void show() {
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
			16
		);
		if (gFont == NULL) {
			printf("Failed to load font: %s\n", TTF_GetError());
			return false;
		}

		// Grab mouse
		// SDL_SetWindowGrab(gWindow, SDL_TRUE);
		
		// Set blending
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

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
