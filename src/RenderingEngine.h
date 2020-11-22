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

struct Dimension {
	int top;
	int right;
	int bottom;
	int left;
};

Dimension get_dimension(const std::vector<Vector2f>& vs) {
	// Find dimensions
	Dimension d;
	d.left = vs[0].x();
	d.right = vs[0].x();
	d.top = vs[0].y();
	d.bottom = vs[0].y();
	for (int j = 0; j < (int)vs.size(); j++) {
		d.left = vs[j].x() < d.left ? vs[j].x() : d.left;
		d.right = vs[j].x() > d.right ? vs[j].x() : d.right;
		d.top = vs[j].y() < d.top ? vs[j].y() : d.top;
		d.bottom = vs[j].y() > d.bottom ? vs[j].y() : d.bottom;
	}
	return d;
}

double cubic_interpolate(double y0, double y1, double y2, double y3, double mu) {
	double a0, a1, a2, a3, mu2;
	mu2 = mu * mu;
	a0 = y3 - y2 - y0 + y1;
	a1 = y0 - y1 - a0;
	a2 = y2 - y0;
	a3 = y1;
	return a0*mu*mu2 + a1*mu2 + a2*mu + a3;
}

std::vector<Vector2f> cubic_interpolate(const std::vector<Vector2f>& vs, int np) {
	std::vector<Vector2f> ps;
	int nv = (int)vs.size();
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++) {
			ps[j*np+k].setx(cubic_interpolate(v1.x(), v2.x(), v3.x(), v4.x(), (double)k/np));
			ps[j*np+k].sety(cubic_interpolate(v1.y(), v2.y(), v3.y(), v4.y(), (double)k/np));
		}
	}
	return ps;
}

double hermite_interpolate(double y0, double y1, double y2, double y3, double mu, double tension, double bias) {
	double a0, a1, a2, a3, m0, m1, mu2, mu3;
	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (y1 - y0) * (1 + bias) * (1 - tension)/2;
	m0 += (y2 - y1) * (1 - bias) * (1 - tension)/2;
	m1 = (y2 - y1) * (1 + bias) * (1 - tension)/2;
	m1 += (y3 - y2) * (1 - bias) * (1 - tension)/2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;
	return a0*y1 + a1*m0 + a2*m1 + a3*y2;
}

std::vector<Vector2f> hermite_interpolate(const std::vector<Vector2f>& vs, int np, double tension, double bias) {
	std::vector<Vector2f> ps;
	int nv = (int)vs.size();
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++) {
			ps[j*np+k].setx(hermite_interpolate(v1.x(), v2.x(), v3.x(), v4.x(), (double)k/np, tension, bias));
			ps[j*np+k].sety(hermite_interpolate(v1.y(), v2.y(), v3.y(), v4.y(), (double)k/np, tension, bias));
		}
	}
	return ps;
}

Vector2f bspline(Vector2f& p1, Vector2f& p2, Vector2f& p3, Vector2f& p4, double t) {
	Vector2f p;
	double t2 = t * t;
	double t3 = t2 * t;
	double mt = 1.0 - t;
	double mt3 = mt * mt * mt;
	double bi3 = mt3;
	double bi2 = 3 * t3 - 6 * t2 + 4;
	double bi1 =-3 * t3 + 3 * t2 + 3 * t + 1;
	double bi  = t3;
	p.setx((p1.x()*bi3 + p2.x()*bi2 +p3.x()*bi1 + p4.x()*bi) / 6.0);
	p.sety((p1.y()*bi3 + p2.y()*bi2 +p3.y()*bi1 + p4.y()*bi) / 6.0);
	return p;
}

std::vector<Vector2f> bspline(const std::vector<Vector2f>& vs, int np) {
	std::vector<Vector2f> ps;
	int nv = (int)vs.size();
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++) {
			ps[j*np+k] = bspline(v1, v2, v3, v4, (double)k/np);
		}
	}
	return ps;
}

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
	World gWorld;

	void set_world(World& world) {
		gWorld = world;

		if (gWorld.num_levels() == 0)
			return;

		// Get world size
		MapLevel& level = gWorld.get_level(0);
		world_width = level.get_tile_width() * level.get_width();
		world_height = level.get_tile_height() * level.get_height();

		// std::vector<std::pair<int, int>> obs = level.get_obstructions();
		// // std::map<std::pair<int, int>, int> groups;
		// std::vector<std::vector<int>> grid;
		// grid.resize(level.get_width());
		// for (auto& g : grid)
		// 	g.resize(level.get_height());
		// int group_id = 1;
		// int size = (int)obs.size();
		// for (int j = 0; j < size; j++) {
		// 	bool next_group = true;
		// 	// grid[obs[j].first][obs[j].second] = group_id;
		// 	for (int k = 0; k < size; k++) {
		// 		std::cout << std::endl;
		// 		std::cout << abs(obs.at(j).first - obs.at(k).first) << std::endl;
		// 		std::cout << abs(obs.at(j).second - obs.at(k).second) << std::endl;
		// 		std::cout << obs.at(j).first << std::endl;
		// 		std::cout << obs.at(k).first << std::endl;
		// 		std::cout << obs.at(j).second << std::endl;
		// 		std::cout << obs.at(k).second << std::endl;
		// 		if ((abs(obs.at(j).first - obs.at(k).first) == 1 && obs.at(j).second == obs.at(k).second) ||
		// 			(abs(obs.at(j).second - obs.at(k).second) == 1 && obs.at(j).first == obs.at(k).first)) {
		// 			// if (groups.find(std::make_pair(j, k)) == groups.end())
		// 			// 	groups.insert(std::make_pair(std::make_pair(j, k), group_id));
		// 			if (grid[obs.at(k).first][obs.at(k).second] == 0) {
		// 				grid[obs.at(k).first][obs.at(k).second] = group_id;
		// 				next_group = false;
		// 			}
		// 			std::cout << "group: " << group_id << " : " << j << ", " << k << std::endl;
		// 		}
		// 		std::cout << std::endl;
		// 	}
		// 	if (next_group)
		// 		group_id++;
		// }
		// // auto& grid = level.get_obgrid();
		// for (int j = 0; j < level.get_width(); j++) {
		// 	for (int k = 0; k < level.get_height(); k++) {
		// 		if (grid[j][k] != 0)
		// 			std::cout << grid[j][k] << " ";
		// 		else
		// 			std::cout << "  ";
		// 	}
		// 	std::cout << std::endl;
		// }
	}

	class Camera {
		public:
			Vector2f position = Vector2f(-1, -1);
			float zoom = 8;
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

	void fill_poly(std::vector<Vector2f> ps) {
		if (ps.size() == 0)
			return;
		// Find dimensions
		Dimension dim = get_dimension(ps);
		// Fill polygon
		std::vector<int> node_x;
		for (int y = dim.top; y < dim.bottom; y++) {
			node_x.resize(ps.size());
			int nodes = 0;
			int j = (int)ps.size() - 1;
			for (int i = 0; i < (int)ps.size(); i++) {
				if ((ps[i].y() < (double)y && ps[j].y() >= (double)y) || (ps[j].y() < (double)y && ps[i].y() >= (double)y))
					node_x[nodes++] = (int)(ps[i].x()+(y-ps[i].y())/(ps[j].y()-ps[i].y())*(ps[j].x()-ps[i].x()));
				j = i;
			}
			node_x.resize(nodes);
			std::sort(node_x.begin(), node_x.end());
			for (int i = 0; i < nodes; i += 2) {
				if (node_x[i] >= dim.right)
					break;
				if (node_x[i+1] > dim.left) {
					if (node_x[i] < dim.left)
						node_x[i] = dim.left;
					if (node_x[i+1] > dim.right)
						node_x[i+1] = dim.right;
					SDL_RenderDrawLine(gRenderer, node_x[i], y, node_x[i+1], y);
					// for (int x = node_x[i]; x < node_x[i+1]; x++)
					// 	SDL_RenderDrawPoint(gRenderer, x, y);
				}
			}
		}
	}

	void clear() {
		// Get current window size
		SDL_GetWindowSize(gWindow, &width, &height);

		// Clear screen
		SDL_SetRenderDrawColor(gRenderer, 16, 16, 16, 255);
		SDL_RenderClear(gRenderer);
	}

	const int PANNING_PAD = 10;
	const float PAN_SPEED = 0.3f;
	const float ZOOM_KEY_SPEED = 0.01f;

	Vector2f ldrag_start = Vector2f(-1, -1);
	Vector2f rdrag_start = Vector2f(-1, -1);
	bool lbutton_down = false;
	bool rbutton_down = false;

	void render(float delta_time) {
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
			if (Input::is_mouse_pressed(SDL_BUTTON_LEFT)) {
				if (!lbutton_down) {
					// Button down
					std::pair<int, int> pos = Input::get_mouse_pos();
					Vector2f world_pos = screen_to_world(pos.first, pos.second);
					gWorld.click(world_pos, 0, 0);
					lbutton_down = true;
				}
			}
			else if (lbutton_down) {
				// Button released
				std::pair<int, int> pos = Input::get_mouse_pos();
				Vector2f world_pos = screen_to_world(pos.first, pos.second);
				gWorld.click(world_pos, 0, 1);
				lbutton_down = false;
			}
			if (Input::is_mouse_pressed(SDL_BUTTON_RIGHT)) {
				if (!rbutton_down) {
					// Button down
					std::pair<int, int> pos = Input::get_mouse_pos();
					Vector2f world_pos = screen_to_world(pos.first, pos.second);
					gWorld.click(world_pos, 1, 0);
					rbutton_down = true;
				}
			}
			else if (rbutton_down) {
				// Button released
				std::pair<int, int> pos = Input::get_mouse_pos();
				Vector2f world_pos = screen_to_world(pos.first, pos.second);
				gWorld.click(world_pos, 1, 1);
				rbutton_down = false;
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

		// Interpolate and fillpoly demo
		float cx = width/2;
		float cy = height/2;
		const float DX = 100;
		const float DY = 100;

		// Make test vertices
		std::vector<Vector2f> vs;
		vs.push_back(Vector2f(cx-DX, cy+DY));
		vs.push_back(Vector2f(cx-DX, cy-DY));
		vs.push_back(Vector2f(cx+DX, cy-DY));
		vs.push_back(Vector2f(cx+DX*2, cy+DY*2));
		vs.push_back(Vector2f(cx+DX*3, cy+DY));

		// Draw original vertices as polygon
		Vector2f last_v = vs.at(vs.size()-1);
		for (auto& v : vs) {
			SDL_SetRenderDrawColor(gRenderer, 0, 255, 255, 255);
			SDL_RenderDrawLine(gRenderer, v.x(), v.y(), last_v.x(), last_v.y());
			last_v = v;
		}

		// Draw interpolated vertices as polygon
		std::vector<Vector2f> ps;
		Vector2f last_p;
		// Cubic
		ps = cubic_interpolate(vs, 100);
		last_p = ps.at(ps.size()-1);
		for (auto& p : ps) {
			SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 200);
			SDL_RenderDrawLine(gRenderer, p.x(), p.y(), last_p.x(), last_p.y());
			last_p = p;
		}
		// Hermite
		ps = hermite_interpolate(vs, 100, 0.5, 0);
		last_p = ps.at(ps.size()-1);
		for (auto& p : ps) {
			SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 200);
			SDL_RenderDrawLine(gRenderer, p.x(), p.y(), last_p.x(), last_p.y());
			last_p = p;
		}
		// Bspline
		ps = bspline(vs, 100);
		last_p = ps.at(ps.size()-1);
		for (auto& p : ps) {
			SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 200);
			SDL_RenderDrawLine(gRenderer, p.x(), p.y(), last_p.x(), last_p.y());
			last_p = p;
		}

		// Fill interpolated vertices as polygon
		SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 64);
		fill_poly(ps);
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
