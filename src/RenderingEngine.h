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
	// Find range of x and y
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

double cubic_interpolate(const double y0, const double y1, const double y2, const double y3, const double mu) {
	double mu2 = mu * mu;
	double a0 = y3 - y2 - y0 + y1;
	double a1 = y0 - y1 - a0;
	double a2 = y2 - y0;
	double a3 = y1;
	return a0*mu*mu2 + a1*mu2 + a2*mu + a3;
}

std::vector<Vector2f> cubic_interpolate(const std::vector<Vector2f>& vs, const int np) {
	int nv = (int)vs.size();
	std::vector<Vector2f> ps;
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

double hermite_interpolate(const double y0, const double y1, const double y2, const double y3, const double mu, const double tension, const double bias) {
	double mu2 = mu * mu;
	double mu3 = mu2 * mu;
	double m0 = (y1-y0) * (1+bias) * (1-tension)/2 + (y2-y1) * (1-bias) * (1-tension)/2;
	double m1 = (y2-y1) * (1+bias) * (1-tension)/2 + (y3-y2) * (1-bias) * (1-tension)/2;
	double a0 = 2 * mu3 - 3 * mu2 + 1;
	double a1 = mu3 - 2 * mu2 + mu;
	double a2 = mu3 - mu2;
	double a3 = -2 * mu3 + 3 * mu2;
	return a0*y1 + a1*m0 + a2*m1 + a3*y2;
}

std::vector<Vector2f> hermite_interpolate(const std::vector<Vector2f>& vs, const int np, const double tension, const double bias) {
	int nv = (int)vs.size();
	std::vector<Vector2f> ps;
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

Vector2f bspline_interpolate(const Vector2f& p0, const Vector2f& p1, const Vector2f& p2, const Vector2f& p3, const double mu) {
	double mu2 = mu * mu;
	double mu3 = mu2 * mu;
	double mt = 1 - mu;
	double mt3 = mt * mt * mt;
	double bi0 = mt3;
	double bi1 = 3 * mu3 - 6 * mu2 + 4;
	double bi2 = -3 * mu3 + 3 * mu2 + 3 * mu + 1;
	double bi3 = mu3;
	return Vector2f(
		(p0.x()*bi0 + p1.x()*bi1 +p2.x()*bi2 + p3.x()*bi3)/6,
		(p0.y()*bi0 + p1.y()*bi1 +p2.y()*bi2 + p3.y()*bi3)/6
	);
}

std::vector<Vector2f> bspline_interpolate(const std::vector<Vector2f>& vs, const int np) {
	int nv = (int)vs.size();
	std::vector<Vector2f> ps;
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++)
			ps[j*np+k] = bspline_interpolate(v1, v2, v3, v4, (double)k/np);
	}
	return ps;
}

namespace RenderingEngine {

	// Default window dimensions
	const int DEFAULT_WINDOW_WIDTH = 1200;
	const int DEFAULT_WINDOW_HEIGHT = 800;

	// Font for all rendered text
	const char* TTF_FILE = "./res/fonts/Roboto-Light.ttf";
	const int FONT_SIZE = 16;

	// Distance of mouse from window edge to activate map panning
	const int PAN_REGION_SIZE = 10;

	// Camera settings
	const float PAN_SPEED = 0.3f;
	const float ZOOM_KEY_SPEED = 0.01f;
	const float DEFAULT_ZOOM = 8.0f;
	const float MIN_ZOOM = 1.0f;
	const float MAX_ZOOM = 20.0f;

	/**
	Used to convert world space into screen coordinates.
	*/
	class Camera {
		public:
			Vector2f position = Vector2f(-1, -1);
			float zoom = DEFAULT_ZOOM;
			float get_zoom_factor() {
				return zoom*zoom/100.0f + 0.5f;
			}
	};

	// Player camera
	Camera cam;

	// Track mouse input states
	Vector2f ldrag_start = Vector2f(-1, -1);
	Vector2f rdrag_start = Vector2f(-1, -1);
	bool lbutton_down = false;
	bool rbutton_down = false;

	// Screen dimensions
	int width = DEFAULT_WINDOW_WIDTH;
	int height = DEFAULT_WINDOW_HEIGHT;

	// World dimensions
	float world_width;
	float world_height;

	// SDL subsystems to free / destory later
	SDL_Window* gWindow;
	SDL_Renderer* gRenderer;
	TTF_Font* gFont;
	World gWorld;

	/**
	Set the world to render.
	*/
	std::vector<std::vector<Vector2f>> vertices;
	void set_world(const  World& world) {
		gWorld = world;

		if (gWorld.num_levels() == 0)
			return;

		// Get world size
		MapLevel& level = gWorld.get_level(0);
		world_width = level.get_tile_width() * level.get_width();
		world_height = level.get_tile_height() * level.get_height();

		// Get obstruction grid
		std::vector<std::vector<bool>> bool_grid = level.get_obgrid();
		int grid_x = (int)bool_grid.size();
		int grid_y = (int)bool_grid[0].size();

		// Create obstruction group grid
		std::vector<std::vector<int>> grid;
		grid.resize(grid_x);
		for (int x = 0; x < grid_x; x++)
			for (int y = 0; y < grid_y; y++)
				grid[x].push_back(bool_grid[x][y] ? 1 : 0);

		// Define grid value check
		auto grid_contains = [&grid, grid_x, grid_y](int value) -> bool {
			for (int x = 0; x < grid_x; x++)
				for (int y = 0; y < grid_y; y++)
					if (grid[x][y] == value)
						return true;
			return false;
		};

		// Define recursive assignment for a group
		std::function<void(int, int, int)> neighbor_assign;
		neighbor_assign = [&grid, grid_x, grid_y, &neighbor_assign](int x, int y, int group) -> void {
			grid[x][y] = group;
			if (x-1 >= 0 && grid[x-1][y] == 1)
				neighbor_assign(x-1, y, group);
			if (x+1 < grid_x && grid[x+1][y] == 1)
				neighbor_assign(x+1, y, group);
			if (y-1 >= 0 && grid[x][y-1] == 1)
				neighbor_assign(x, y-1, group);
			if (y+1 < grid_y && grid[x][y+1] == 1)
				neighbor_assign(x, y+1, group);
		};

		// Define search and group assignment
		auto group_assign = [&grid, grid_x, grid_y, &neighbor_assign](int group) -> void {
			for (int x = 0; x < grid_x; x++)
				for (int y = 0; y < grid_y; y++)
					if (grid[x][y] == 1)
						return neighbor_assign(x, y, group);
		};

		// Assign groups
		int group = 2;
		while (grid_contains(1))
			group_assign(group++);

		for (const auto& row : grid) {
			for (const auto& v : row)
				std::cout << v << " ";
			std::cout << std::endl;
		}

		// Define safe grid value getter
		auto get_value = [&grid, grid_x, grid_y](int x, int y) -> int {
			if (x >= 0 && x < grid_x && y >= 0 && y < grid_y)
				return grid[x][y];
			return 0;
		};

		// Define neighbor count
		auto n_neighbors = [&grid, grid_x, grid_y, &get_value](int x, int y) -> int {
			int ns = 0;
			if (get_value(x, y-1) != 0)
				ns++;
			if (get_value(x+1, y) != 0)
				ns++;
			if (get_value(x, y+1) != 0)
				ns++;
			if (get_value(x-1, y) != 0)
				ns++;
			return ns;
		};

		// Define next perimeter
		auto next_perimeter = [&grid, grid_x, grid_y, &get_value, &n_neighbors](int x, int y, int& direction, int last_direction) -> std::pair<int, int> {
			int nn = n_neighbors(x, y);
			// Solo
			if (nn == 0)
				return std::make_pair(x, y);
			// // Deadend
			// if (nn == 1)
			// 	direction = (direction+2)%4;

			for (int j = 0; j < 4; j++) {
				if (direction == 0) {
					// Up
					if (get_value(x, y-1) != 0) {
						if (get_value(x-1, y-1) != 0)  // Something to the left
							direction = 3;
						return std::make_pair(x, y-1);
					}
				}
				else if (direction == 1) {
					// Right
					if (get_value(x+1, y) != 0) {
						if (get_value(x+1, y-1) != 0)  // Something to the top
							direction = 0;
						return std::make_pair(x+1, y);
					}
				}
				else if (direction == 2) {
					// Down
					if (get_value(x, y+1) != 0) {
						if (get_value(x+1, y+1) != 0)  // Something to the right
							direction = 1;
						return std::make_pair(x, y+1);
					}
				}
				else if (direction == 3) {
					// Left
					if (get_value(x-1, y) != 0) {
						if (get_value(x-1, y+1) != 0)  // Something to the bottom
							direction = 2;
						return std::make_pair(x-1, y);
					}
				}
				direction = (direction+1)%4;
				// if (direction == (last_direction+2)%4)
				// 	direction = (direction+1)%4;
			}
			std::cout << "Error: no perimeter found" << std::endl;
			return std::make_pair(x, y);
		};

		// Define vertex search for group starting at x and y
		float grid_w = level.get_tile_width();
		float grid_h = level.get_tile_height();
		auto group_vertices = [&grid, grid_x, grid_y, grid_w, grid_h, &next_perimeter](int x, int y, int group) -> std::vector<Vector2f> {
			std::vector<Vector2f> vs;
			vs.push_back(Vector2f(x*grid_w + grid_w/2, y*grid_h + grid_h/2));

			std::pair<int, int> next_grid = {-1, -1};
			std::pair<int, int> last_grid = {x, y};
			int direction = 1;
			int last_direction = 1;
			bool rewind = false;
			while (true) {
				if (next_grid.first == x && next_grid.second == y) {
					if (!rewind)
						rewind = true;
					else
						break;
				}
				// Compute next grid
				next_grid = next_perimeter(last_grid.first, last_grid.second, direction, last_direction);

				// std::cout << std::endl;
				// std::cout << "group: " << group << std::endl;
				// std::cout << "last_direction: " << last_direction << std::endl;
				// std::cout << "direction: " << direction << std::endl;
				// std::cout << "last_grid: " << last_grid.first << ", " << last_grid.second << std::endl;
				// std::cout << "next_grid: " << next_grid.first << ", " << next_grid.second << std::endl;
				// std::cout << "origin: " << x << ", " << y << std::endl;
				// std::cout << std::endl;

				if (last_direction != direction) {
					// Vertex found
					if (rewind)
						vs.insert(vs.begin(), 1, Vector2f(last_grid.first*grid_w + grid_w/2, last_grid.second*grid_h + grid_h/2));
					else
						vs.push_back(Vector2f(last_grid.first*grid_w + grid_w/2, last_grid.second*grid_h + grid_h/2));
				}
				last_direction = direction;
				last_grid = next_grid;
			}

			// Remove duplicates
			std::set<std::pair<float, float>> set;
			for (auto i = vs.begin(); i != vs.end();) {
				std::pair<float, float> v = std::make_pair((*i).x(), (*i).y());
				if (set.find(v) != set.end()) {
					i = vs.erase(i);
				}
				else {
					set.insert(v);
					++i;
				}
			}
			return vs;
		};

		// Build final vector
		int process_group = 2;
		for (int x = 0; x < grid_x; x++)
			for (int y = 0; y < grid_y; y++)
				if (grid[x][y] == process_group && process_group <= group)
					vertices.push_back(group_vertices(x, y, process_group++));
	}

	/**
	Convert world position to screen coordinate.
	*/
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

	/**
	Convert screen coordinate to world position.
	*/
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

	/**
	Fill inside a polygon.
	Set color using SDL_SetRenderDrawColor() beforehand.
	*/
	void fill_poly(std::vector<Vector2f> ps) {
		if (ps.size() == 0)
			return;
		// Find dimensions
		Dimension dim = get_dimension(ps);
		// Sweep line
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
			// Sort
			node_x.resize(nodes);
			std::sort(node_x.begin(), node_x.end());
			// Draw lines
			for (int i = 0; i < nodes; i += 2) {
				if (node_x[i] >= dim.right)
					break;
				if (node_x[i+1] > dim.left) {
					if (node_x[i] < dim.left)
						node_x[i] = dim.left;
					if (node_x[i+1] > dim.right)
						node_x[i+1] = dim.right;
					SDL_RenderDrawLine(gRenderer, node_x[i], y, node_x[i+1], y);
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
			if (cam.zoom < MIN_ZOOM)
				cam.zoom = MIN_ZOOM;
			else if (cam.zoom > MAX_ZOOM)
				cam.zoom = MAX_ZOOM;

			// Pan map
			Vector2f dp = Vector2f(0, 0);
			if (mp.first < PAN_REGION_SIZE || Input::is_key_pressed(SDLK_LEFT))
				dp.setx(-PAN_SPEED);
			if (mp.first > width - PAN_REGION_SIZE || Input::is_key_pressed(SDLK_RIGHT))
				dp.setx(PAN_SPEED);
			if (mp.second < PAN_REGION_SIZE || Input::is_key_pressed(SDLK_UP))
				dp.sety(-PAN_SPEED);
			if (mp.second > height - PAN_REGION_SIZE || Input::is_key_pressed(SDLK_DOWN))
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

		// Draw obstruction vertices
		for (auto& vs : vertices) {
			if (vs.size() == 0)
				continue;

			// Interpolate
			std::vector<Vector2f> ps;
			Vector2f last_p;

			// // Hermite
			// ps = hermite_interpolate(vs, 4, -1.0, 0);
			// last_p = world_to_screen(ps.at(ps.size()-1));
			// for (auto& p : ps)
			// 	p = world_to_screen(p);
			// // Fill poly
			// SDL_SetRenderDrawColor(gRenderer, 128, 128, 255, 255);
			// fill_poly(ps);

			const int N_ITER = 10;
			for (int j = 0; j < N_ITER; j++) {
				float t = ((float)j/N_ITER)-1;
				// Hermite 2
				ps = hermite_interpolate(vs, 4, t, 0);
				last_p = world_to_screen(ps.at(ps.size()-1));
				for (auto& p : ps)
					p = world_to_screen(p);
				// Fill poly
				SDL_SetRenderDrawColor(gRenderer, 64, 32, 200, 64);
				fill_poly(ps);
			}

			// Cubic
			ps = cubic_interpolate(vs, 4);
			last_p = world_to_screen(ps.at(ps.size()-1));
			for (auto& p : ps)
				p = world_to_screen(p);
			// Fill poly
			SDL_SetRenderDrawColor(gRenderer, 64, 32, 200, 127);
			fill_poly(ps);

			// Bspline
			ps = bspline_interpolate(vs, 4);
			last_p = world_to_screen(ps.at(ps.size()-1));
			// Interpolate vertices
			for (auto& p : ps)
				p = world_to_screen(p);
			// Fill poly
			SDL_SetRenderDrawColor(gRenderer, 64, 32, 200, 127);
			fill_poly(ps);

			// Grid vertices
			last_p = world_to_screen(vs[(int)vs.size()-1]);
			for (auto& v : vs) {
				Vector2f p = world_to_screen(v);
				SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 64);
				SDL_RenderDrawLine(gRenderer, last_p.x(), last_p.y(), p.x(), p.y());
				last_p = p;
			}
		}

		// // Interpolate and fillpoly demo
		// float cx = width/2;
		// float cy = height/2;
		// const float DX = 100;
		// const float DY = 100;
		// // Make test vertices
		// std::vector<Vector2f> vs;
		// vs.push_back(Vector2f(cx-DX, cy+DY));
		// vs.push_back(Vector2f(cx-DX, cy-DY));
		// vs.push_back(Vector2f(cx+DX, cy-DY));
		// vs.push_back(Vector2f(cx+DX*2, cy+DY*2));
		// vs.push_back(Vector2f(cx+DX*3, cy+DY));
		// // Draw original vertices as polygon
		// Vector2f last_v = vs.at(vs.size()-1);
		// for (auto& v : vs) {
		// 	SDL_SetRenderDrawColor(gRenderer, 0, 255, 255, 255);
		// 	SDL_RenderDrawLine(gRenderer, v.x(), v.y(), last_v.x(), last_v.y());
		// 	last_v = v;
		// }
		// // Draw interpolated vertices as polygon
		// std::vector<Vector2f> ps;
		// Vector2f last_p;
		// // Cubic
		// ps = cubic_interpolate(vs, 100);
		// last_p = ps.at(ps.size()-1);
		// for (auto& p : ps) {
		// 	SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 200);
		// 	SDL_RenderDrawLine(gRenderer, p.x(), p.y(), last_p.x(), last_p.y());
		// 	last_p = p;
		// }
		// // Hermite
		// ps = hermite_interpolate(vs, 100, 0.5, 0);
		// last_p = ps.at(ps.size()-1);
		// for (auto& p : ps) {
		// 	SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 200);
		// 	SDL_RenderDrawLine(gRenderer, p.x(), p.y(), last_p.x(), last_p.y());
		// 	last_p = p;
		// }
		// // Bspline
		// ps = bspline_interpolate(vs, 100);
		// last_p = ps.at(ps.size()-1);
		// for (auto& p : ps) {
		// 	SDL_SetRenderDrawColor(gRenderer, 0, 0, 255, 200);
		// 	SDL_RenderDrawLine(gRenderer, p.x(), p.y(), last_p.x(), last_p.y());
		// 	last_p = p;
		// }
		// // Fill interpolated vertices as polygon
		// SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 64);
		// fill_poly(ps);
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
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
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

		// Create a OpenGL context on SDL2
		SDL_GLContext gl_context = SDL_GL_CreateContext(gWindow);

		// Load GL extensions using glad
		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
			std::cerr << "Failed to initialize the OpenGL context." << std::endl;
			exit(1);
		}

		// Create font from TrueType Font file
		gFont = TTF_OpenFont(
			TTF_FILE,
			FONT_SIZE
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
