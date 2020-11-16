#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <utility> 

#include "Input.h"
#include "Input.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "rts_unit.h"
#include "level.h"
#include "algorithms.h"
// #include "GameObject.h"
// #include "GameObject.cpp"

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

// Stat
float fps;
float ups;

// Test ball
Vector2f* ball_pos = new Vector2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
Vector2f* ball_vel = new Vector2f(0, 0);
int ball_r = 10;
bool test_start = false;

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

	level lev(5, 5, {std::make_pair(1, 2), std::make_pair(2, 2), std::make_pair(2, 3), std::make_pair(1, 2)});
	auto path = astar(lev, {0, 1}, {4, 4});

	for(auto& p : path){
		std::cout << "(" << p.first << ", " << p.second << "), ";
	}
	std::cout << std::endl;

	std::cout << "Goodbye" << std::endl;

	// Capture mouse
	// int result = SDL_CaptureMouse(SDL_TRUE);
	// if (result == -1)
	// 	printf("Warning: Mouse could not be captured");

	// SDL_SetWindowOpacity(gWindow, 0.9f);

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
	if (!test_start)
		return;

	// Update pos
	Vector2f* delta_pos = ball_vel->mul(delta_time);
	
	// Discount vel
	ball_vel = ball_vel->mul(0.99f);
	
	// Input changes velocity
	const float dv = 0.01;
	if (Input::is_key_pressed(SDLK_UP))
		ball_vel->set(ball_vel->x(), ball_vel->y()-dv);
	if (Input::is_key_pressed(SDLK_DOWN))
		ball_vel->set(ball_vel->x(), ball_vel->y()+dv);
	if (Input::is_key_pressed(SDLK_LEFT))
		ball_vel->set(ball_vel->x()-dv, ball_vel->y());
	if (Input::is_key_pressed(SDLK_RIGHT))
		ball_vel->set(ball_vel->x()+dv, ball_vel->y());

	// Bounce off walls
	ball_pos = ball_pos->add(delta_pos);
	if (ball_pos->x() > SCREEN_WIDTH) {
		ball_pos->set(SCREEN_WIDTH, ball_pos->y());
		ball_vel->set(ball_vel->x() * -1, ball_vel->y());
	}
	if (ball_pos->y() > SCREEN_HEIGHT) {
		ball_pos->set(ball_pos->x(), SCREEN_HEIGHT);
		ball_vel->set(ball_vel->x(), ball_vel->y() * -1);
	}
	if (ball_pos->x() < 0) {
		ball_pos->set(0, ball_pos->y());
		ball_vel->set(ball_vel->x() * -1, ball_vel->y());
	}
	if (ball_pos->y() < 0) {
		ball_pos->set(ball_pos->x(), 0);
		ball_vel->set(ball_vel->x(), ball_vel->y() * -1);
	}
}

void render() {
	// Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0x11, 0x11, 0x11, 0xFF);
	SDL_RenderClear(gRenderer);

	// Draw test ball
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	int x = (int)ball_pos->x();
	int y = (int)ball_pos->y();
	SDL_Rect ballbox = {x-ball_r/2, y-ball_r/2, ball_r, ball_r};
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xAA, 0x77, 0xFF);		
	SDL_RenderFillRect(gRenderer, &ballbox);

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
	SDL_Color color = {50, 255, 50};
	SDL_Rect text_rect;
	text_rect.h = 20;
	text_rect.x = 8;
	text_rect.y = 4;
	char text_str[64];
	// Display FPS
	sprintf(text_str, "FPS: %.0f", fps);
	SDL_Surface* fps_surface = TTF_RenderText_Solid(gFont, text_str, color);
	SDL_Texture* fps_texture = SDL_CreateTextureFromSurface(gRenderer, fps_surface);
	text_rect.w = strlen(text_str) * 10;
	SDL_RenderCopy(gRenderer, fps_texture, NULL, &text_rect);
	// Display UPS
	sprintf(text_str, "UPS: %.0f", ups);
	SDL_Surface* ups_surface = TTF_RenderText_Solid(gFont, text_str, color);
	SDL_Texture* ups_texture = SDL_CreateTextureFromSurface(gRenderer, ups_surface);
	text_rect.w = strlen(text_str) * 10;
	text_rect.y += text_rect.h;
	SDL_RenderCopy(gRenderer, ups_texture, NULL, &text_rect);

	// Update screen
	SDL_RenderPresent(gRenderer);

	// Free stat rendering resources
	SDL_FreeSurface(fps_surface);
	SDL_DestroyTexture(fps_texture);
	SDL_FreeSurface(ups_surface);
	SDL_DestroyTexture(ups_texture);
}

int main(int argc, char* args[]) {
	// Start up SDL and create window
	if (!init()) {
		printf("Failed to initialize!\n");
		return 1;
	}

	// Performance stat
	unsigned int record_time;
	unsigned int updates;
	unsigned int frames;

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
			updates++;
		}
		render();
		frames++;

		float elapsed_record_time = (current_time - record_time) / 1000;
		if (elapsed_record_time >= 0.5f) {
			// std::cout << "FPS: " << frames / elapsed_record_time << std::endl;
			// std::cout << "UPS: " << updates / elapsed_record_time << std::endl;
			// std::cout << "Frames: " << frames << std::endl;
			// std::cout << "Updates: " << updates << std::endl;
			// std::cout << "Time: " << current_time << std::endl;
			// std::cout << std::endl;
			record_time = current_time;
			fps = frames / elapsed_record_time;
			ups = updates / elapsed_record_time;
			updates = 0;
			frames = 0;
		}

		if (Input::is_key_pressed(SDLK_UP) ||
			Input::is_key_pressed(SDLK_DOWN) ||
			Input::is_key_pressed(SDLK_LEFT) ||
			Input::is_key_pressed(SDLK_RIGHT))
			test_start = true;

		// if (Input::is_key_pressed(SDLK_UP))
		// 	std::cout << "SDLK_UP" << std::endl;
		// if (Input::is_key_pressed(SDLK_DOWN))
		// 	std::cout << "SDLK_DOWN" << std::endl;
		// if (Input::is_key_pressed(SDLK_LEFT))
		// 	std::cout << "SDLK_LEFT" << std::endl;
		// if (Input::is_key_pressed(SDLK_RIGHT))
		// 	std::cout << "SDLK_RIGHT" << std::endl;

		// if (Input::is_mouse_pressed(SDL_BUTTON_LEFT))
		// 	std::cout << "SDL_BUTTON_LEFT" << std::endl;
		// if (Input::is_mouse_pressed(SDL_BUTTON_RIGHT))
		// 	std::cout << "SDL_BUTTON_RIGHT" << std::endl;
		// if (Input::is_mouse_pressed(SDL_BUTTON_MIDDLE))
		// 	std::cout << "SDL_BUTTON_MIDDLE" << std::endl;
		// if (Input::is_mouse_pressed(SDL_BUTTON_X1))
		// 	std::cout << "SDL_BUTTON_X1" << std::endl;
		// if (Input::is_mouse_pressed(SDL_BUTTON_X2))
		// 	std::cout << "SDL_BUTTON_X2" << std::endl;
	}

	// Free resources and close SDL
	close();

	return 0;
}