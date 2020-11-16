#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <utility>

#include "Stat.h"
#include "Stat.cpp"
#include "Input.h"
#include "Input.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "GameObject.h"
#include "GameObject.cpp"

#include "rts_unit.h"
#include "level.h"
#include "algorithms.h"

// Screen dimension constants
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

const unsigned int MS_PER_UPDATE = 10;
// const char* FONT = "./res/fonts/Bebas-Regular.ttf";
// const char* FONT = "./res/fonts/veteran_typewriter.ttf";
// const char* FONT = "./res/fonts/underwood_champion.ttf";
const char* FONT = "./res/fonts/Roboto-Medium.ttf";

// Starts up SDL and creates window
bool init();

// Frees media and shuts down SDL
void close();

// The window we'll be rendering to
SDL_Window* gWindow = NULL;

// The window renderer
SDL_Renderer* gRenderer = NULL;

// Globally used font
TTF_Font* gFont = NULL;

// Test ball
bool run_test = false;
GameObject* ball = new GameObject(
	new Vector2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2),
	new Vector2f(0, 0),
	10
);

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

	// Grab mouse
	SDL_SetWindowGrab(gWindow, SDL_TRUE);

	std::cout << "Hello" << std::endl;
	level lev(5, 5, {std::make_pair(1, 2), std::make_pair(2, 2), std::make_pair(2, 3), std::make_pair(1, 2), std::make_pair(3, 3), std::make_pair(3, 4)});
	auto path = astar(lev, {0, 1}, {4, 4});
	for (auto& p : path) {
		std::cout << "(" << p.first << ", " << p.second << "), ";
	}
	std::cout << std::endl;
	std::cout << "Goodbye" << std::endl;

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

	if (!run_test)
		return;

	// Update position
	ball->update(delta_time);
	// Discount velocity
	ball->set_v(ball->v()->mul(0.99f));
	// Input changes velocity
	const float dv = 0.01;
	if (Input::is_key_pressed(SDLK_UP))
		ball->v()->set(ball->v()->x(), ball->v()->y()-dv);
	if (Input::is_key_pressed(SDLK_DOWN))
		ball->v()->set(ball->v()->x(), ball->v()->y()+dv);
	if (Input::is_key_pressed(SDLK_LEFT))
		ball->v()->set(ball->v()->x()-dv, ball->v()->y());
	if (Input::is_key_pressed(SDLK_RIGHT))
		ball->v()->set(ball->v()->x()+dv, ball->v()->y());
	// Bounce off walls
	if (ball->p()->x() > SCREEN_WIDTH) {
		ball->p()->set(SCREEN_WIDTH, ball->p()->y());
		ball->v()->set(ball->v()->x() * -1, ball->v()->y());
	}
	if (ball->p()->y() > SCREEN_HEIGHT) {
		ball->p()->set(ball->p()->x(), SCREEN_HEIGHT);
		ball->v()->set(ball->v()->x(), ball->v()->y() * -1);
	}
	if (ball->p()->x() < 0) {
		ball->p()->set(0, ball->p()->y());
		ball->v()->set(ball->v()->x() * -1, ball->v()->y());
	}
	if (ball->p()->y() < 0) {
		ball->p()->set(ball->p()->x(), 0);
		ball->v()->set(ball->v()->x(), ball->v()->y() * -1);
	}
}

void render() {
	Stat::frame_tick();

	// Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x11, 0xFF);
	SDL_RenderClear(gRenderer);

	// Draw test ball
	ball->render(gRenderer);

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

int main(int argc, char* args[]) {
	// Start up SDL and create window
	if (!init()) {
		printf("Failed to initialize!\n");
		return 1;
	}

	// Game loop
	unsigned int last_time;
	unsigned int current_time;
	unsigned int elapsed_time;
	unsigned int unprocessed_time;
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

		if (Input::is_key_pressed(SDLK_UP) ||
			Input::is_key_pressed(SDLK_DOWN) ||
			Input::is_key_pressed(SDLK_LEFT) ||
			Input::is_key_pressed(SDLK_RIGHT))
			run_test = true;
	}

	// Free resources and close SDL
	close();

	return 0;
}