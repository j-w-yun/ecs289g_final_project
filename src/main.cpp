#ifdef _MSC_VER
#include <SDL.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif

#ifdef __linux__
#define USE_SDL2_RENDERER
#else
#define USE_SDL2_RENDERER
#endif

#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <utility>

#include "glad.h"
#include "glad.c"

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
#include "Util.h"
#include "RenderingEngine.h"
#include "projectile.h"
#include "projectile.cpp"
#include "land_factory.h"

#include "algorithms.h"
#include "rts_unit.h"
#include "group.cpp"
// #include "level.h"

// Print stacktrace and exit gracefully
#ifdef __linux__
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/prctl.h>
	#include <sys/wait.h>
	void stacktrace_handler(int sig) {
		char pid_buf[30];
		sprintf(pid_buf, "%d", getpid());
		char name_buf[512];
		name_buf[readlink("/proc/self/exe", name_buf, 511)]=0;
		prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
		int child_pid = fork();
		if (!child_pid) {
			dup2(2,1);
			fprintf(stdout, "stack trace for %s pid=%s\n", name_buf,pid_buf);
			execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt", name_buf, pid_buf, NULL);
			abort();
		}
		else {
			waitpid(child_pid, NULL, 0);
		}
		exit(1);
	}
#endif

// Minimum delta time for update
const Uint64 MIN_UPDATE_INTERVAL = 10;

// Map settings
const int X_TILES = 100;
const int Y_TILES = 50;
const float TILE_WIDTH = 10.0f;
const float TILE_HEIGHT = 10.0f;
const int BASE_PADDING = 5;

// Starts up SDL and creates window
bool init();

// Frees media and shuts down SDL
void close();

// World game objects
World gWorld;

// Update if true
bool is_running = false;

// Temporary variables for tests
std::vector<std::pair<int, int>> bases;
std::vector<std::vector<AStar::Vec2i>> paths;
void run_test() {
	// // Test stacktrace handler
	// int *bad_pointer = (int*)-1;
	// printf("%d\n", *bad_pointer);

	// Util::test();

	// // Generate bases
	// srand(time(NULL));
	// int xh = X_TILES / 2;
	// int yh = Y_TILES / 2;
	// bases = {
	// 	{(float)rand()/RAND_MAX * (xh+1), (float)rand()/RAND_MAX * (yh+1)},
	// 	{(float)rand()/RAND_MAX * (X_TILES+1)+xh, (float)rand()/RAND_MAX * (yh+1)},
	// 	{(float)rand()/RAND_MAX * (xh+1), (float)rand()/RAND_MAX * (Y_TILES+1)+yh},
	// 	{(float)rand()/RAND_MAX * (X_TILES+1)+xh, (float)rand()/RAND_MAX * (Y_TILES+1)+yh},
	// };

	// Bases
	bases = {
		{BASE_PADDING, BASE_PADDING},
		{BASE_PADDING, Y_TILES-(BASE_PADDING+1)},
		{X_TILES-(BASE_PADDING+1), BASE_PADDING},
		{X_TILES-(BASE_PADDING+1), Y_TILES-(BASE_PADDING+1)},
	};

	// Create level
	std::shared_ptr<MapLevel> map_level_ptr = std::make_shared<MapLevel>(X_TILES, Y_TILES, TILE_WIDTH, TILE_HEIGHT, 10000);
	MapLevel& map_level = *map_level_ptr;
	gWorld.add(map_level_ptr);

	// Perlin
	// map_level.set_obstructions(map_level.generate_obstructions(bases, BASE_PADDING));

	// Worms
	// map_level.generate_worms(X_TILES, Y_TILES, SCREEN_WIDTH/X_TILES, SCREEN_HEIGHT/Y_TILES, 4, BASE_PADDING, 4, 10, 30, 1, 2);
	map_level.generate_worms(X_TILES, Y_TILES, TILE_WIDTH, TILE_HEIGHT, 4, BASE_PADDING + 2, 6, 10, 30, 1, 2);
	// map_level.generate_worms(X_TILES, Y_TILES, SCREEN_WIDTH/X_TILES, SCREEN_HEIGHT/Y_TILES, 20, 5, 4, 30, 70, 1, 2);
	//map_level.generate_worms(X_TILES, Y_TILES, SCREEN_WIDTH/X_TILES, SCREEN_HEIGHT/Y_TILES, 0, 5, 4, 30, 70, 1, 2);
	//map_level.generate_worms(X_TILES, Y_TILES, SCREEN_WIDTH/X_TILES, SCREEN_HEIGHT/Y_TILES, 2, 1, 1, 5, 10, 0, 1);
	//map_level.generate_worms(X_TILES, Y_TILES, SCREEN_WIDTH/X_TILES, SCREEN_HEIGHT/Y_TILES, 0, 1, 1, 5, 10, 0, 1);

	// // Test pathfinding
	// for (int j = 0; j < (int)bases.size(); j++) {
	// 	for (int k = j+1; k < (int)bases.size(); k++) {
	// 		auto path = find_path(map_level, bases.at(j), bases.at(k));
	// 		paths.push_back(path);
	// 		// for (auto& p : path)
	// 		// 	std::cout << "(" << p.first << ", " << p.second << ")" << std::endl;
	// 	}
	// }

	// initialize managers
	map_level.init_managers();

	// Test RTS units
	float WORLD_WIDTH = TILE_WIDTH*X_TILES;
	float WORLD_HEIGHT = TILE_HEIGHT*Y_TILES;
	// units
	for(int i = 0; i < 250; i++){
		auto position = Vector2f(bases.at(0).first * TILE_WIDTH, rand()%(int)WORLD_HEIGHT);
		auto velocity = Vector2f(0, 0);
		auto rts_ptr = std::make_shared<rts_unit>(
			position,
			velocity,
			3.0f,  // Radius
			WORLD_WIDTH,
			WORLD_HEIGHT,
			X_TILES,
			Y_TILES,
			0, // team
			1, // health
			0.05f,  // Acceleration
			0.5f,  // Travel speed
			map_level
		);
		//rts_ptr->selected = 1;
		map_level.add(rts_ptr);

		auto position2 = Vector2f(X_TILES*TILE_WIDTH - position.x(), rand()%(int)WORLD_HEIGHT);
		rts_ptr = std::make_shared<rts_unit>(
			position2,
			velocity,
			3.0f,  // Radius
			WORLD_WIDTH,
			WORLD_HEIGHT,
			X_TILES,
			Y_TILES,
			1, // team
			1, // health
			0.05f,  // Acceleration
			0.5f,  // Travel speed
			map_level
		);
		map_level.add(rts_ptr);
	}
	// factories
	for(int i = 0; i < 3; i++){
		auto position = Vector2f(2 * TILE_WIDTH, rand()%(int)WORLD_HEIGHT);
		auto factory_ptr = std::make_shared<land_factory>(
			position,
			25.0f,  // Radius
			WORLD_WIDTH,
			WORLD_HEIGHT,
			X_TILES,
			Y_TILES,
			0, // team
			15, // health
			100, // spawn time
			map_level
		);
		map_level.add(factory_ptr);

		auto position2 = Vector2f(X_TILES*TILE_WIDTH - position.x(), rand()%(int)WORLD_HEIGHT);
		factory_ptr = std::make_shared<land_factory>(
			position2,
			25.0f,  // Radius
			WORLD_WIDTH,
			WORLD_HEIGHT,
			X_TILES,
			Y_TILES,
			1, // team
			15, // health
			100, // spawn time
			map_level
		);
		map_level.add(factory_ptr);
	}

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

	RenderingEngine::set_world(gWorld);
}

bool init() {
	if (!RenderingEngine::initialize())
		return false;

	// Initialize world
	gWorld = World();

	run_test();

	return true;
}

void close() {
	RenderingEngine::destroy();
}

void update(float delta_time) {
	Stat::update_tick();
	if (!is_running)
		return;

	gWorld.update(delta_time);
}

void render(float delta_time) {
	Stat::frame_tick();

	// Clear screen
	RenderingEngine::clear();

	// Draw world
	RenderingEngine::render(delta_time);

	std::vector<SDL_Rect> rects;

	// Draw path
	for (auto& path : paths) {
		for (auto& p : path) {
			// SDL_Rect box = {tile_width*p.x, tile_height*p.y, tile_width, tile_height};
			Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(p.x*TILE_WIDTH, p.y*TILE_HEIGHT));
			Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((p.x+1)*TILE_WIDTH, (p.y+1)*TILE_HEIGHT));
			SDL_Rect box = {
				(int)(sp1.x()),
				(int)(sp1.y()),
				(int)(sp2.x()-sp1.x()),
				(int)(sp2.y()-sp1.y())
			};
#ifdef USE_SDL2_RENDERER
			SDL_SetRenderDrawColor(RenderingEngine::gRenderer, 0x22, 0xFF, 0x22, 0x55);
			rects.push_back(box);// SDL_RenderFillRect(RenderingEngine::gRenderer, &box);
#else
			RenderingEngine::ogl_set_color(0x22, 0xFF, 0x22, 0x55);
			RenderingEngine::ogl_fill_rect(box);
#endif
		}
	}

#ifdef USE_SDL2_RENDERER
	if (rects.size() > 0) SDL_RenderFillRects(RenderingEngine::gRenderer, &rects[0], rects.size());
	rects.clear();
#endif

	// Draw bases
	for (int i = 0; i < (int)bases.size(); i++) {
		auto base = bases.at(i);
		// SDL_Rect base_tile = {tile_width*base.first, tile_height*base.second, tile_width, tile_height};
		Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(base.first*TILE_WIDTH, base.second*TILE_HEIGHT));
		Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((base.first+1)*TILE_WIDTH, (base.second+1)*TILE_HEIGHT));
		SDL_Rect box = {
			(int)(sp1.x()),
			(int)(sp1.y()),
			(int)(sp2.x()-sp1.x()),
			(int)(sp2.y()-sp1.y())
		};
#ifdef USE_SDL2_RENDERER
		SDL_SetRenderDrawColor(RenderingEngine::gRenderer, 0x77, 0x22, 0x22, 0xFF);
		rects.push_back(box); // SDL_RenderFillRect(RenderingEngine::gRenderer, &box);
#else
		RenderingEngine::ogl_set_color(0x77, 0x22, 0x22, 0xFF);
		RenderingEngine::ogl_fill_rect(box);
#endif
	}
#ifndef USE_SDL2_RENDERER
	RenderingEngine::ogl_send_rects_to_draw();
#else
	if (rects.size() > 0) SDL_RenderFillRects(RenderingEngine::gRenderer, &rects[0], rects.size());
#endif
	// Update screen
	RenderingEngine::show();
}

#ifdef _MSC_VER
#undef main
#endif

int main(int argc, char* args[]) {
	#ifdef __linux__
		signal(SIGSEGV, stacktrace_handler);
	#endif

	// Start up SDL and create window
	if (!init()) {
		printf("Failed to initialize!\n");
		return 1;
	}

	// Game loop
	// Util::Timer timer;
	Uint64 unprocessed_time = 0;
	Uint64 last_time = 0;
	while (Input::process_inputs()) {
		if (Input::has_input())
			is_running = true;

		// unprocessed_time += timer.reset();
		if (last_time == 0)
			last_time = SDL_GetTicks();
		Uint64 current_time = SDL_GetTicks();
		unprocessed_time += current_time - last_time;
		last_time = current_time;
		render((float)unprocessed_time);
		// std::cout << current_time << ", " << last_time << ", " << unprocessed_time << std::endl;
		while (unprocessed_time >= MIN_UPDATE_INTERVAL) {
			// if (MIN_UPDATE_INTERVAL > unprocessed_time / 2) {
			// 	update(MIN_UPDATE_INTERVAL);
			// 	unprocessed_time -= MIN_UPDATE_INTERVAL;
			// }
			// else {
			// 	update(unprocessed_time / 2);
			// 	unprocessed_time /= 2;
			// }
			update((float)MIN_UPDATE_INTERVAL);
			unprocessed_time -= MIN_UPDATE_INTERVAL;
		}
	}

	// Free resources and close SDL
	close();

	return 0;
}
