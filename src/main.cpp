#ifdef _MSC_VER
#include <SDL.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif

#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <utility>

#include "Stat.h"
#include "Stat.cpp"
#include "Input.h"
#include "Input.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "GameObject.h"
#include "GameObject.cpp"
#include "Map.h"
#include "Map.cpp"
#include "World.h"
#include "World.cpp"
#include "Noise.cpp"
#include "AStar.hpp"
#include "AStar.cpp"

#include "rts_unit.h"
#include "level.h"
#include "algorithms.h"

// Screen dimension constants
const unsigned int SCREEN_WIDTH = 1200;
const unsigned int SCREEN_HEIGHT = 800;

// Update time slice
const unsigned int MS_PER_UPDATE = 10;

// Font file
const char* FONT = "./res/fonts/Roboto-Medium.ttf";

// Starts up SDL and creates window
bool init();

// Frees media and shuts down SDL
void close();

// The window we'll be rendering to
SDL_Window* gWindow;

// The window renderer
SDL_Renderer* gRenderer;

// Globally used font
TTF_Font* gFont;

// World game objects
World gWorld;

// Update if true
bool is_running = false;

// Pathfinding test variables
// auto origin = std::make_pair(0, 0);
// auto target = std::make_pair(X_TILES-1, Y_TILES-1);
const int X_TILES = 50;
const int Y_TILES = 50;
const int BASE_PADDING = 5;
std::vector<std::pair<int, int>> bases;
std::vector<std::vector<AStar::Vec2i>> paths;
void run_test() {
	// Generate bases
	srand(time(NULL));
	
	// int xh = X_TILES / 2;
	// int yh = Y_TILES / 2;
	// bases = {
	// 	{(float)rand()/RAND_MAX * (xh+1), (float)rand()/RAND_MAX * (yh+1)},
	// 	{(float)rand()/RAND_MAX * (X_TILES+1)+xh, (float)rand()/RAND_MAX * (yh+1)},
	// 	{(float)rand()/RAND_MAX * (xh+1), (float)rand()/RAND_MAX * (Y_TILES+1)+yh},
	// 	{(float)rand()/RAND_MAX * (X_TILES+1)+xh, (float)rand()/RAND_MAX * (Y_TILES+1)+yh},
	// };
	
	bases = {
		{BASE_PADDING, BASE_PADDING},
		{BASE_PADDING, Y_TILES-(BASE_PADDING+1)},
		{X_TILES-(BASE_PADDING+1), BASE_PADDING},
		{X_TILES-(BASE_PADDING+1), Y_TILES-(BASE_PADDING+1)},
	};

	// Create level
	std::shared_ptr<MapLevel> map_level_ptr = std::make_shared<MapLevel>(X_TILES, Y_TILES, SCREEN_WIDTH/X_TILES, SCREEN_HEIGHT/Y_TILES);
	MapLevel& map_level = *map_level_ptr;
	auto obstructions = map_level.generate_obstructions(bases, BASE_PADDING);
	map_level.set_obstructions(obstructions);
	gWorld.add(map_level_ptr);

	// // Test ball
	// std::shared_ptr<GameObject> ball_ptr = std::make_shared<GameObject>(Vector2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2), Vector2f(0, 0), 10, SCREEN_WIDTH, SCREEN_HEIGHT, X_TILES, Y_TILES);
	// GameObject& ball = *ball_ptr;
	// // Draw circle
	// auto render_callback = [ball_ptr](SDL_Renderer* renderer) {
	// 	GameObject& ball = *ball_ptr;
	// 	float x = ball.p().x();
	// 	float y = ball.p().y();
	// 	int r = (int)ball.r();
	// 	// SDL_SetRenderDrawColor(renderer, 0xFF, 0x77, 0x55, 0xFF);
	// 	for (int w = 0; w < r * 2; w++) {
	// 		for (int h = 0; h < r * 2; h++) {
	// 			float dx = (float)r - w;
	// 			float dy = (float)r - h;
	// 			float distance = dx*dx + dy*dy;
	// 			float max_length = r*r;
	// 			if (distance <= max_length) {
	// 				// Make edges more transluscent
	// 				float transparency = distance / max_length;
	// 				float opacity = 1.0f - (transparency * 0.8f);
	// 				SDL_SetRenderDrawColor(renderer, 0xFF, 0x22, 0x22, (int)(255*opacity));
	// 				SDL_RenderDrawPoint(renderer, (int)(x+dx), (int)(y+dy));
	// 			}
	// 		}
	// 	}
	// };
	// // Enable user control and bouncing
	// auto update_callback = [ball_ptr](float t) {
	// 	GameObject& ball = *ball_ptr;
	// 	Vector2f v = ball.v();
	// 	Vector2f p = ball.p();
	// 	int r = (int)ball.r();
	// 	// Input changes velocity
	// 	const float dv = 0.01;
	// 	if (Input::is_key_pressed(SDLK_UP))
	// 		v.set(v.x(), v.y()-dv);
	// 	if (Input::is_key_pressed(SDLK_DOWN))
	// 		v.set(v.x(), v.y()+dv);
	// 	if (Input::is_key_pressed(SDLK_LEFT))
	// 		v.set(v.x()-dv, v.y());
	// 	if (Input::is_key_pressed(SDLK_RIGHT))
	// 		v.set(v.x()+dv, v.y());
	// 	// Bounce off walls
	// 	if (p.x() > SCREEN_WIDTH-r) {
	// 		p.set(SCREEN_WIDTH-r, p.y());
	// 		v.set(v.x() * -1, v.y());
	// 	}
	// 	if (p.y() > SCREEN_HEIGHT-r) {
	// 		p.set(p.x(), SCREEN_HEIGHT-r);
	// 		v.set(v.x(), v.y() * -1);
	// 	}
	// 	if (p.x() < r) {
	// 		p.set(r, p.y());
	// 		v.set(v.x() * -1, v.y());
	// 	}
	// 	if (p.y() < r) {
	// 		p.set(p.x(), r);
	// 		v.set(v.x(), v.y() * -1);
	// 	}
	// 	// Discount velocity
	// 	v = v.scale(0.99f);
	// 	ball.set_p(p);
	// 	ball.set_v(v);
	// };
	// // Set callbacks
	// ball.set_render_callback(render_callback);
	// ball.set_update_callback(update_callback);
	// map_level.add(ball_ptr);

	// Test pathfinding
	for (int j = 0; j < (int)bases.size(); j++) {
		for (int k = j+1; k < (int)bases.size(); k++) {
			auto path = find_path(map_level, bases.at(j), bases.at(k));
			paths.push_back(path);
			// for (auto& p : path)
			// 	std::cout << "(" << p.first << ", " << p.second << ")" << std::endl;
		}
	}

	// FIXME enable
	auto rts_ptr = std::make_shared<rts_unit>(
		Vector2f(bases.at(0).first * SCREEN_WIDTH/X_TILES, bases.at(0).second * SCREEN_HEIGHT/Y_TILES),
		Vector2f(0, 0),
		11,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		X_TILES,
		Y_TILES,
		.1,
		2,
		map_level
	);
	rts_ptr->selected = 1;
	map_level.add(rts_ptr);
}

bool init() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize: %s\n", SDL_GetError());
		return false;
	}

	// Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		printf("Warning: Linear texture filtering not enabled");

	// Create window
	gWindow = SDL_CreateWindow("Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL) {
		printf("Window could not be created: %s\n", SDL_GetError());
		return false;
	}

	// Create renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL) {
		printf("Renderer could not be created: %s\n", SDL_GetError());
		return false;
	}

	// Initialize fonts
	if(TTF_Init() == -1) {
		printf("SDL_ttf could not initialize: %s\n", TTF_GetError());
		return false;
	}
	gFont = TTF_OpenFont(FONT, 64);
	if (gFont == NULL) {
		printf("Failed to load font: %s\n", TTF_GetError());
		return false;
	}

	// Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

	// Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x11, 0xFF);
	SDL_RenderClear(gRenderer);
	SDL_RenderPresent(gRenderer);

	// Grab mouse
	// SDL_SetWindowGrab(gWindow, SDL_TRUE);

	// Initialize world
	gWorld = World();

	run_test();

	return true;
}

void close() {
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

void update(float delta_time) {
	Stat::update_tick();
	if (!is_running)
		return;

	gWorld.update(delta_time);
}

void render() {
	Stat::frame_tick();

	// Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x11, 0xFF);
	SDL_RenderClear(gRenderer);

	// Draw world
	gWorld.render(gRenderer);

	// Path
	int tile_width = SCREEN_WIDTH / X_TILES;
	int tile_height = SCREEN_HEIGHT / Y_TILES;
	for (auto& path : paths) {
		for (auto& p : path) {
			// SDL_Rect box = {tile_width*p.first, tile_height*p.second, tile_width, tile_height};
			SDL_Rect box = {tile_width*p.x, tile_height*p.y, tile_width, tile_height};
			SDL_SetRenderDrawColor(gRenderer, 0x22, 0xFF, 0x22, 0x55);
			SDL_RenderFillRect(gRenderer, &box);
		}
	}
	// // Origin
	// SDL_Rect origin_box = {tile_width*origin.first, tile_height*origin.second, tile_width, tile_height};
	// SDL_SetRenderDrawColor(gRenderer, 0x22, 0x22, 0x77, 0xFF);
	// SDL_RenderFillRect(gRenderer, &origin_box);
	// // Target
	// SDL_Rect target_box = {tile_width*target.first, tile_height*target.second, tile_width, tile_height};
	// SDL_SetRenderDrawColor(gRenderer, 0x77, 0x22, 0x22, 0xFF);
	// SDL_RenderFillRect(gRenderer, &target_box);

	// Draw bases
	for (int i = 0; i < (int)bases.size(); i++) {
		auto base = bases.at(i);
		SDL_Rect base_tile = {tile_width*base.first, tile_height*base.second, tile_width, tile_height};
		SDL_SetRenderDrawColor(gRenderer, 0x77, 0x22, 0x22, 0xFF);
		SDL_RenderFillRect(gRenderer, &base_tile);
	}

	// Left mouse drag
	if (Input::has_dragbox(SDL_BUTTON_LEFT)) {
		DragBox box = Input::get_dragbox(SDL_BUTTON_LEFT);
		SDL_Rect dragbox = {box.x1, box.y1, box.x2-box.x1, box.y2-box.y1};
		// Render filled quad
		SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0x11);
		SDL_RenderFillRect(gRenderer, &dragbox);
		// Render outline quad
		SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0xFF);
		SDL_RenderDrawRect(gRenderer, &dragbox);
	}

	// Right mouse drag
	if (Input::has_dragbox(SDL_BUTTON_RIGHT)) {
		DragBox box = Input::get_dragbox(SDL_BUTTON_RIGHT);
		SDL_Rect dragbox = {box.x1, box.y1, box.x2-box.x1, box.y2-box.y1};
		// Render filled quad
		SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0x22);
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

#ifdef _MSC_VER
#undef main
#endif

int main(int argc, char* args[]) {
	// Start up SDL and create window
	if (!init()) {
		printf("Failed to initialize!\n");
		return 1;
	}

	// Game loop
	unsigned int last_time = SDL_GetTicks();
	unsigned int current_time;
	unsigned int elapsed_time;
	unsigned int unprocessed_time = 0;
	while (Input::process_inputs()) {
		current_time = SDL_GetTicks();
		elapsed_time = current_time - last_time;
		last_time = current_time;
		unprocessed_time += elapsed_time;
		while (unprocessed_time >= MS_PER_UPDATE) {
			update(MS_PER_UPDATE);
			unprocessed_time -= MS_PER_UPDATE;
		}
		render();

		if (Input::has_input())
			is_running = true;
	}

	// Free resources and close SDL
	close();

	return 0;
}
