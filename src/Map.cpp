#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <math.h>
#include <numeric>
#include <set>
#include <time.h>
#include <vector>
#include <algorithm>

#include "algorithms.h"
#include "AStar.hpp"
#include "AStar.cpp"
#include "GameObject.h"
#include "GameObject.cpp"
#include "Noise.cpp"
#include "RenderingEngine.h"
#include "Util.h"

#define USE_PERLIN_NOISE_SHADER 1

const float MIN_DENSITY = 0.05;
const float MAX_DENSITY = 0.25;
const int update_groups = 100;

int MapLevel::group_size = 100;
char MapLevel::group_size_keydown = 0;

template<class T>
void remove(std::vector<T>& vec, T val) {
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i] == val) {
			vec.erase(vec.begin() + i);
			return;
		}
	}
}

MapLevel::MapLevel(int tx, int ty, float tw, float th, size_t uc): tiles_x(tx), tiles_y(ty), tile_width(tw), tile_height(th), unitcap(uc) {
	class_string = MapLevel::static_class();



	// const int MIN_L = tx<ty ? tx : ty;
	// const int MAX_L = tx>=ty ? tx : ty;
	// min_octave = MAX_L - MIN_L;
	// max_octave = sqrt(tx*ty)/4;
	min_octave = 1;
	max_octave = sqrt((tx*ty)/(tx+ty))+1;
	if (max_octave < 3)
		max_octave = 3;
	// std::cout << "min_octave: " << min_octave << " max_octave: " << max_octave << std::endl;

	// === unit stuff ===

	// init stack
	for (size_t i = 0; i < uc; i++) {
		idstack.push_back(i);
	}

	// init objects
	units = std::vector<std::shared_ptr<GameObject>>(unitcap);

	// init unitgrid (4d)
	for(int i = 0; i < teams; i++){
		unitgrid.push_back(std::vector<std::vector<std::vector<size_t>>>(tiles_x, std::vector<std::vector<size_t>>(tiles_y)));
	}
	

	// init obgrid
	obgrid = std::vector<std::vector<bool>>(tx, std::vector<bool>(ty, 0));

	// === projectile stuff ===
	projcap = 2*unitcap;

	// init proj stack
	for (size_t i = 0; i < projcap; i++) {
		pstack.push_back(i);
	}

	// init projectiles
	projectiles = std::vector<std::shared_ptr<projectile>>(projcap);

	// === Generate texture ===
	generate_texture();
}

void MapLevel::init_managers(){
	// manager stuff

	std::vector<float> x_vals = {.1, .9};

	for(int t = 0; t < teams; t++){
		// find rally point
		std::pair<int, int> point = {(int)(tiles_x * x_vals[t]), (int)(tiles_y * .5)};
		Vector2f pointf = to_world_space(point);
		bool init = false;
		Vector2f closest;

		for(auto& r : rectcover){
			if(!init){
				closest = closest_point(pointf, r);
				std::cout << "Closest point between " << pointf << " and " << r << " is " << closest << std::endl;
				init = true;
			}
			else{
				auto p = closest_point(pointf, r);
				std::cout << "Closest point between " << pointf << " and " << r << " is " << p << std::endl;

				if((closest - pointf).len() > (p - pointf).len()){
					closest = p;
				}
			}
		}

		managers.push_back(manager(t, closest));
		std::cout << "Manager " << t << " rally: " << closest << std::endl;
	}

	// default team 1 groups
	managers[1].groups.push_back(std::make_shared<defense_group>(100, Vector2f(tiles_x*tile_width * .6, 0), Vector2f(0, 0), std::shared_ptr<MapLevel>(this), 6.0f));
	managers[1].groups.push_back(std::make_shared<attack_group>(50, Vector2f(5, tiles_y*tile_height * .33), managers[1].rally_point, std::shared_ptr<MapLevel>(this)));
	managers[1].groups.push_back(std::make_shared<attack_group>(50, Vector2f(5, tiles_y*tile_height * .66), managers[1].rally_point, std::shared_ptr<MapLevel>(this)));

	// test attack group
	//managers[0].groups = {};
	//managers[0].groups.push_back(std::make_shared<attack_group>(50, managers[1].rally_point, managers[0].rally_point, std::shared_ptr<MapLevel>(this)));
}

bool MapLevel::add(std::shared_ptr<GameObject> o) {
	if(o->team < 0 || o-> team >= teams) return false;

	if(!idstack.size()) return false;

	auto id = idstack.back();
	idstack.pop_back();

	units[id] = o;
	auto tile = o->get_tile();

	// 4d
	unitgrid[o->team][tile.first][tile.second].push_back(id);
	o->id = id;

	//std::cout << "Adding unit " << id << std::endl;

	return true;
}

bool MapLevel::add_proj(std::shared_ptr<projectile> o){
	if(!pstack.size()) return false;

	auto id = pstack.back();
	pstack.pop_back();

	projectiles[id] = o;
	o->id = id;

	return true;
}

void MapLevel::kill(int id){
	if(id < 0 || id >= (int)unitcap)
		return;

	auto unit = units[id];

	if(!unit)
		return;

	// remove from grid
	auto tile = unit->get_tile();

	// 4d
	auto& vec = unitgrid[units[id]->team][tile.first][tile.second];
	remove(vec, (size_t)id);

	// buffer id
	idbuffer.push_back(id);

	// clear space
	units[id].reset();
}

void MapLevel::kill_proj(int id){
	if(id < 0 || id >= (int)projcap)
		return;

	if(!projectiles[id])
		return;

	projectiles[id].reset();

	pstack.push_back(id);
}

void MapLevel::set_size(int x, int y, int w, int h) {
	tiles_x = x;
	tiles_y = y;
	tile_width = w;
	tile_height = h;

	idstack = {};
	for (size_t i = 0; i < unitcap; i++) {
		idstack.push_back(i);
	}

	// init objects
	units = std::vector<std::shared_ptr<GameObject>>(unitcap);

	// init unitgrid (4d)
	for(int i = 0; i < teams; i++){
		unitgrid.push_back(std::vector<std::vector<std::vector<size_t>>>(tiles_x, std::vector<std::vector<size_t>>(tiles_y)));
	}

	// init obgrid
	obgrid = std::vector<std::vector<bool>>(tiles_x, std::vector<bool>(tiles_y, 0));

	// init proj stack
	pstack = {};
	for (size_t i = 0; i < projcap; i++) {
		pstack.push_back(i);
	}

	// init projectiles
	projectiles = std::vector<std::shared_ptr<projectile>>(projcap);
}

void MapLevel::set_obstructions(std::vector<std::pair<int, int>> o) {
	obstructions = o;

	obgrid = std::vector<std::vector<bool>>(tiles_x, std::vector<bool>(tiles_y, 0));

	for (auto& ob : obstructions) {
		obgrid[ob.first][ob.second] = 1;
	}
}

void MapLevel::set(int x, int y, int w, int h, std::vector<std::pair<int, int>> o) {
	set_size(x, y, w, h);
	set_obstructions(o);
}

bool MapLevel::climb(std::vector<std::pair<int, int>>* obs, double noise[], float threshold, std::vector<std::pair<int, int>> bases, int padding) {
	for (int j = 0; j < tiles_x; j++) {
		for (int k = 0; k < tiles_y; k++) {
			if (noise[(j*tiles_y)+k] < threshold) {
				bool valid = true;
				// Radius padding
				for (auto& base : bases) {
					float dx = (float)base.first - j;
					float dy = (float)base.second - k;
					float distance = dx*dx + dy*dy;
					float max_length = padding*padding;
					if (distance <= max_length) {
						valid = false;
						continue;
					}
				}
				if (valid)
					obs->push_back(std::make_pair(j, k));
			}
		}
	}
	return true;
}

std::vector<std::pair<int, int>> MapLevel::random_obstructions(std::vector<std::pair<int, int>> bases, int min, int max, int padding) {
	std::vector<std::pair<int, int>> obs;
	float threshold;
	float persistence;
	int n_octaves;
	int prime_index;
	int offset_x;
	int offset_y;
	while ((int)obs.size() < min || (int)obs.size() > max) {
		persistence = Util::uniform_random(0.5, 1.2);
		n_octaves = Util::uniform_random(min_octave, max_octave);
		prime_index = (int)Util::uniform_random(0, 11);
		offset_x = (int)Util::uniform_random(0, 100000);
		offset_y = (int)Util::uniform_random(0, 100000);

		obs.clear();
		// std::cout << " n_octaves: " << n_octaves << " persistence: " << persistence << " prime_index: " << prime_index << std::endl;
		threshold = 0.00005f;
		// Use std::vector here since variable length arrays are not supported in MSVC, as it is not standard.
		std::vector<double> noise;
		noise.resize(tiles_x*tiles_y);
		for (int j = 0; j < tiles_x; j++)
			for (int k = 0; k < tiles_y; k++)
				noise[(j*tiles_y)+k] = perlin_noise(j+offset_x, k+offset_y, n_octaves, persistence, prime_index);

		while ((int)obs.size() < min && threshold < persistence) {
			obs.clear();
			if (!climb(&obs, &noise[0], threshold, bases, padding) || (int)obs.size() > max) {
				// std::cout << "failed at threshold: " << threshold << std::endl;
				break;
			}
			threshold += 0.00005f;
		}
	}
	std::cout << std::endl;
	std::cout << "min_octave: " << min_octave << std::endl;
	std::cout << "max_octave: " << max_octave << std::endl;
	std::cout << "n_octaves: " << n_octaves << std::endl;
	std::cout << "persistence: " << persistence << std::endl;
	std::cout << "prime_index: " << prime_index << std::endl;
	std::cout << "threshold: " << threshold << std::endl;
	std::cout << "offset_x: " << offset_x << std::endl;
	std::cout << "offset_y: " << offset_y << std::endl;
	std::cout << std::endl;
	return obs;
}

std::vector<std::pair<int, int>> MapLevel::generate_obstructions(std::vector<std::pair<int, int>> bases, int padding) {
	std::vector<std::pair<int, int>> obs;
	std::vector<AStar::Vec2i> path;
	MapLevel* level = new MapLevel();
	level->set_size(tiles_x, tiles_y, tile_width, tile_height);
	bool found = false;
	while (!found) {
		obs.clear();

		// Create random
		obs = random_obstructions(bases, tiles_x*tiles_y*MIN_DENSITY, tiles_x*tiles_y*MAX_DENSITY, padding);

		// Check if path exists among all bases
		found = true;
		for (int j = 0; j < (int)bases.size(); j++) {
			for (int k = j+1; k < (int)bases.size(); k++) {
				auto a = bases.at(j);
				auto b = bases.at(k);
				for (auto& o : obs) {
					if (o.first == a.first && o.second == a.second) {
						found = false;
						break;
					}
				}

				level->set_obstructions(obs);
				path.clear();
				path = find_path(*level, a, b, true);

				// No path found
				if (path.size() == 0) {
					found = false;
					break;
				}

				// Last position is not target
				auto last = path.at(0);
				if (last.x != b.first || last.y != b.second) {
					found = false;
					break;
				}
			}
			if (!found)
				break;
		}
	}

	return obs;
}

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v) {
	std::vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);
	std::stable_sort(idx.begin(), idx.end(), [&v](size_t i1, size_t i2) {
		return v[i1] < v[i2];
	});
	return idx;
}

void MapLevel::generate_texture() {
	float width = tile_width * tiles_x;
	float height = tile_height * tiles_y;

#ifndef USE_SDL2_RENDERER
#if USE_PERLIN_NOISE_SHADER
	RenderingEngine::update_uniform_buffer(true);
#endif
#endif

	noise2d.clear();
	noise2d.resize(ceil(width/texture_resolution));

	// Noise settings
	int n_octaves = (int)Util::uniform_random(4, 6);  // More octaves generates bigger structures
	float persistence = Util::uniform_random(0.7, 1);  // Between 0 to 1
	int prime_index = (int)Util::uniform_random(0, 10);  // Between 0 to 9
	float offset_x = Util::uniform_random(0, 100000);
	float offset_y = Util::uniform_random(0, 100000);

	// Print generated map settings
	std::cout << std::endl;
	std::cout << "Generated map settings" << std::endl;
	std::cout << "n_octaves: " << n_octaves << std::endl;
	std::cout << "persistence: " << persistence << std::endl;
	std::cout << "prime_index: " << prime_index << std::endl;
	std::cout << std::endl;

	// Used for normalizing noise
	double min_z = 0;
	double max_z = 0;

	// Get noise array
	int xi = 0;
	for (float x = 0; x < width; x += texture_resolution) {
		for (float y = 0; y < height; y += texture_resolution) {
			double z = perlin_noise(x+offset_x, y+offset_y, n_octaves, persistence, prime_index);
			noise2d[xi].push_back(z);
			min_z = z < min_z ? z : min_z;
			max_z = z > max_z ? z : max_z;
		}
		xi++;
	}

	// Normalize noise from 0 to 1
	for (int xi = 0; xi < (int)noise2d.size(); xi++)
		for (int yi = 0; yi < (int)noise2d[xi].size(); yi++)
			noise2d[xi][yi] = (noise2d[xi][yi] - min_z) / (max_z - min_z);


	// void* mPixels = NULL;
	// int mPitch = 0;
	// int mWidth = 0;
	// int mHeight = 0;

	// loaded_surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	// formatted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_GetWindowPixelFormat(RenderingEngine::gWindow), 0);
	// texture = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(RenderingEngine::gWindow), SDL_TEXTUREACCESS_STREAMING, formatted_surface->w, formatted_surface->h);
	// SDL_LockTexture(texture, NULL, &mPixels, &mPitch);
	// memcpy(mPixels, formatted_surface->pixels, formatted_surface->pitch * formatted_surface->h);
	// SDL_UnlockTexture(texture);
	// mPixels = NULL;
	// mWidth = formatted_surface->w;
	// mHeight = formatted_surface->h;

	// // Lock
	// SDL_LockTexture(texture, NULL, &mPixels, &mPitch);
	// //Allocate format from window
	// Uint32 format = SDL_GetWindowPixelFormat( gWindow );
	// SDL_PixelFormat* mappingFormat = SDL_AllocFormat( format );
	// //Get pixel data
	// Uint32* pixels = (Uint32*)gFooTexture.getPixels();
	// int pixelCount = ( gFooTexture.getPitch() / 4 ) * gFooTexture.getHeight();
	// //Map colors
	// Uint32 colorKey = SDL_MapRGB( mappingFormat, 0, 0xFF, 0xFF );
	// Uint32 transparent = SDL_MapRGBA( mappingFormat, 255, 0, 0, 255);
	// // Unlock
	// SDL_UnlockTexture(texture);

	// SDL_FreeSurface(formatted_surface);
	// SDL_FreeSurface(loaded_surface);
}

void MapLevel::render_texture(SDL_Renderer* renderer) {
	float width = tile_width * tiles_x;
	float height = tile_height * tiles_y;

	if (noise2d.size() == 0)
		return;
	// Normalize noise from 0 to 1 and draw
	int xi = 0;
	for (float x = 0; x < width; x += texture_resolution) {
		int yi = 0;
		for (float y = 0; y < height; y += texture_resolution) {
			// Choose color according to Z
			double z = noise2d[xi][yi++];
			// Draw overlapping squares
			Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(x-texture_resolution, y-texture_resolution));
			Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f(x+texture_resolution*2, y+texture_resolution*2));
			SDL_Rect box = {
				(int)(sp1.x()),
				(int)(sp1.y()),
				(int)(sp2.x()-sp1.x())+1,
				(int)(sp2.y()-sp1.y())+1
			};
			float f = ((float)SDL_GetTicks()/400.0f+(x+y+1)/10);
#ifdef USE_SDL2_RENDERER
			SDL_SetRenderDrawColor(renderer, 40, 90*z+40, 30, 150+60*sin(f));
			// SDL_SetRenderDrawColor(renderer, 60, 90*z+40, 30, 150);
			SDL_RenderFillRect(renderer, &box);
#else
#if !USE_PERLIN_NOISE_SHADER
			RenderingEngine::ogl_set_color(40, 90 * z + 40, 30, 150 + 60 * sin(f));
			RenderingEngine::ogl_fill_rect(box);
#endif
#endif
		}
		xi++;
	}

#ifndef USE_SDL2_RENDERER
#if USE_PERLIN_NOISE_SHADER
	Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(0, 0));
	Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f(width, height));
	SDL_Rect box = {
		(int)(sp1.x()),
		(int)(sp1.y()),
		(int)(sp2.x() - sp1.x()) + 1,
		(int)(sp2.y() - sp1.y()) + 1
	};

	RenderingEngine::update_uniform_buffer(false);
	RenderingEngine::ogl_set_color(40, 110, 30, 150 + 60);
	RenderingEngine::ogl_fill_rect(box, USE_PERLIN_NOISE_SHADER);
#endif
#endif

	// TODO: use shaders to do this, with bilinear filtering
#ifndef USE_SDL2_RENDERER
	RenderingEngine::ogl_send_rects_to_draw(USE_PERLIN_NOISE_SHADER);
#endif
}

void MapLevel::compute_obstruction_vertices() {
	// Create obstruction group grid
	int grid_x = (int)obgrid.size();
	int grid_y = (int)obgrid[0].size();
	std::vector<std::vector<int>> grid;
	grid.resize(grid_x);
	for (int x = 0; x < grid_x; x++)
		for (int y = 0; y < grid_y; y++)
			grid[x].push_back(obgrid[x][y] ? 1 : 0);

	std::cout << "grid.size(): " << grid.size() << std::endl;

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
	auto next_perimeter = [&grid, grid_x, grid_y, &get_value, &n_neighbors](int x, int y, int& direction) -> std::pair<int, int> {
		int nn = n_neighbors(x, y);
		// Solo obstruction
		if (nn == 0)
			return std::make_pair(x, y);

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
		}
		std::cout << "Error: no perimeter found" << std::endl;
		return std::make_pair(x, y);
	};

	// Define vertex search for group starting at x and y
	float grid_w = get_tile_width();
	float grid_h = get_tile_height();
	auto group_vertices = [&grid, grid_x, grid_y, grid_w, grid_h, &next_perimeter](int x, int y, int group) -> std::vector<Vector2f> {
		std::vector<Vector2f> vs;
		std::pair<int, int> next_grid = {-1, -1};
		std::pair<int, int> last_grid = {x, y};
		int direction = 0;
		int last_direction = 0;

		auto add_grid = [&vs, grid_w, grid_h](std::pair<int, int> grid, int last_direction, int direction) -> void {
			// Initial point is always top left. No need to offset.
			if (vs.size() == 0) {
				vs.push_back(Vector2f(grid.first*grid_w, grid.second*grid_h));
				return;
			}

			float x = grid.first*grid_w;
			float y = grid.second*grid_h;
			if (last_direction == 0) {
				if (direction == 0)
					return;
				vs.push_back(Vector2f(x, y));
				if (direction == 2)
					vs.push_back(Vector2f(x+grid_w, y));
			}
			else if (last_direction == 1) {
				if (direction == 1)
					return;
				vs.push_back(Vector2f(x+grid_w, y));
				if (direction == 3)
					vs.push_back(Vector2f(x+grid_w, y+grid_h));
			}
			else if (last_direction == 2) {
				if (direction == 2)
					return;
				vs.push_back(Vector2f(x+grid_w, y+grid_h));
				if (direction == 0)
					vs.push_back(Vector2f(x, y+grid_h));
			}
			else if (last_direction == 3) {
				if (direction == 3)
					return;
				vs.push_back(Vector2f(x, y+grid_h));
				if (direction == 1)
					vs.push_back(Vector2f(x, y));
			}
		};

		bool rewind = false;
		while (true) {
			if (next_grid.first == x && next_grid.second == y) {
				if (!rewind)
					rewind = true;
				else
					break;
			}

			// Compute next grid
			next_grid = next_perimeter(last_grid.first, last_grid.second, direction);
			if (last_direction != direction)
				add_grid(last_grid, last_direction, direction);
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
	std::vector<std::vector<Vector2f>> ovs;
	int process_group = 2;
	for (int x = 0; x < grid_x; x++)
		for (int y = 0; y < grid_y; y++)
			if (grid[x][y] == process_group && process_group <= group)
				ovs.push_back(group_vertices(x, y, process_group++));

	// Draw obstruction vertices
	// std::cout << "ovs.size(): " << ovs.size() << std::endl;
	std::vector<Vector2f> _ps1;
	std::vector<Vector2f> _ps2;
	// Interpolate
	for (auto& vs : ovs) {
		// std::cout << "vs.size(): " << vs.size() << std::endl;
		if (vs.size() == 0)
			continue;

		std::vector<std::vector<Vector2f>> vertices;
		const int N_INTERPOLATE = 8;

		// Sand
		for (int j = 0; j < N_SAND/2; j++) {
			float tension = -(float)j / (N_SAND/2);
			// _ps1 = cubic_interpolate(vs, N_INTERPOLATE);
			// _ps2 = hermite_interpolate(vs, N_INTERPOLATE, tension, 0);
			// vertices.push_back(weighted_average(_ps1, _ps2, 1, 3));
			vertices.push_back(hermite_interpolate(vs, N_INTERPOLATE, tension, 0));
		}
		for (int j = 0; j < N_SAND/2; j++) {
			float tension = (float)j / (N_SAND/2) / 2;
			_ps1 = bspline_interpolate(vs, N_INTERPOLATE);
			_ps2 = hermite_interpolate(vs, N_INTERPOLATE, tension, 0);
			vertices.push_back(weighted_average(_ps1, _ps2, 1, 3));
		}

		// Sand + 1
		_ps1 = bspline_interpolate(vs, N_INTERPOLATE);
		_ps2 = hermite_interpolate(vs, N_INTERPOLATE, 0.5, 0);
		vertices.push_back(weighted_average(_ps1, _ps2, 2, 1));

		// Water
		for (int j = 0; j < N_WATER/2; j++) {
			float tension = (float)j / (N_WATER/2) / 2 + 0.5;
			_ps1 = bspline_interpolate(vs, N_INTERPOLATE);
			_ps2 = hermite_interpolate(vs, N_INTERPOLATE, tension, 0);
			vertices.push_back(weighted_average(_ps1, _ps2, 1, 2));
		}
		for (int j = 0; j < N_WATER/2; j++) {
			float tension = (float)j / (N_WATER/2) / 2 + 0.5;
			_ps1 = bspline_interpolate(vs, N_INTERPOLATE);
			_ps2 = hermite_interpolate(vs, N_INTERPOLATE, tension, 0);
			vertices.push_back(weighted_average(_ps1, _ps2, 2, 1));
		}

		obstruction_vertices.push_back(vertices);
	}
}

void MapLevel::render(SDL_Renderer* renderer) {
	// Draw tiles
	const float X_MAX = tile_width * tiles_x;
	const float Y_MAX = tile_height * tiles_y;
	Vector2f world1 = RenderingEngine::world_to_screen(Vector2f(0, 0));
	Vector2f world2 = RenderingEngine::world_to_screen(Vector2f(X_MAX, Y_MAX));
	SDL_Rect world_box = {
		(int)(world1.x()),
		(int)(world1.y()),
		(int)(world2.x()-world1.x()),
		(int)(world2.y()-world1.y())
	};
#ifdef USE_SDL2_RENDERER
	SDL_SetRenderDrawColor(renderer, 239, 196, 121, 255);
	SDL_RenderFillRect(renderer, &world_box);
#else
	RenderingEngine::ogl_set_color(239, 196, 121, 255);
	RenderingEngine::ogl_fill_rect(world_box);
#endif

	/*for (int j = 0; j < tiles_x; j++) {
		for (int k = 0; k < tiles_y; k++) {
			Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(j*tile_width, k*tile_height));
			Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((j+1)*tile_width, (k+1)*tile_height));
			SDL_Rect box = {
				(int)(sp1.x())-2,
				(int)(sp1.y())-2,
				(int)(sp2.x()-sp1.x())+2,
				(int)(sp2.y()-sp1.y())+2
			};
			#ifdef USE_SDL2_RENDERER
			// Fill
			SDL_SetRenderDrawColor(renderer, 239, 196, 121, 255);
			SDL_RenderFillRect(renderer, &box);
			// Outline
			SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 255);
			SDL_RenderDrawRect(renderer, &box);
			#endif
		}
	}*/

	// Draw perlin noise
	if (Input::is_key_pressed(SDLK_SPACE))
		generate_texture();
	render_texture(renderer);

	// // Draw raw obstructions
	// for (auto& o : obstructions) {
	// 	Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(o.first*tile_width, o.second*tile_height));
	// 	Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((o.first+1)*tile_width, (o.second+1)*tile_height));
	// 	SDL_Rect box = {
	// 		(int)(sp1.x()),
	// 		(int)(sp1.y()),
	// 		(int)(sp2.x()-sp1.x())+1,
	// 		(int)(sp2.y()-sp1.y())+1
	// 	};
	// 	// Fill
	// 	float f = ((float)SDL_GetTicks()/1000.0f+(o.first+o.second+1)/4);
	// 	SDL_SetRenderDrawColor(renderer, 40, 40, 190+50*sin(f), 255);
	// 	// SDL_SetRenderDrawColor(renderer, 32, 32, 190, 255);
	// 	SDL_RenderFillRect(renderer, &box);
	// }

	// Draw obstructions
	for (auto& ovs : obstruction_vertices) {
		std::vector<Vector2f> ps;
		// Sand
		for (int j = 0; j < N_SAND; j++) {
			ps = ovs.at(j);
			if (ps.size() > 0) {
#ifdef USE_SDL2_RENDERER
				SDL_SetRenderDrawColor(renderer, 194, 178, 128, 32);
#else
				RenderingEngine::ogl_set_color(194, 178, 128, 32);
#endif
				for (auto& p : ps)
					p = RenderingEngine::world_to_screen(p);
				RenderingEngine::fill_poly(ps);
			}
		}

		// Sand + 1
		ps = ovs.at(N_SAND);
		if (ps.size() > 0) {
#ifdef USE_SDL2_RENDERER
			SDL_SetRenderDrawColor(renderer, 97, 69, 64, 255);
#else
			RenderingEngine::ogl_set_color(97, 69, 64, 255);
#endif
			for (auto& p : ps)
				p = RenderingEngine::world_to_screen(p);
			RenderingEngine::fill_poly(ps);
		}

		// Water
		for (int j = 0; j < N_WATER; j++) {
			ps = ovs.at(N_SAND+j+1);
			if (ps.size() > 0) {
#ifdef USE_SDL2_RENDERER
				SDL_SetRenderDrawColor(renderer, 0, 8, 64, 32);
#else
				RenderingEngine::ogl_set_color(0, 8, 64, 32);
#endif
				for (auto& p : ps)
					p = RenderingEngine::world_to_screen(p);
				RenderingEngine::fill_poly(ps);
			}
		}
#ifndef USE_SDL2_RENDERER
		RenderingEngine::ogl_send_lines_to_draw();
#endif
	}
	// Clip map
	Vector2f p1;
	Vector2f p2;
	SDL_Rect box;
	SDL_SetRenderDrawColor(renderer, 2, 2, 4, 255);
	// Top
	p1 = RenderingEngine::world_to_screen(Vector2f(X_MAX*3, 0));
	p2 = RenderingEngine::world_to_screen(Vector2f(-X_MAX*2, -Y_MAX*2));
	box = {
		(int)p1.x(),
		(int)p1.y(),
		(int)(p2.x() - p1.x()),
		(int)(p2.y() - p1.y())
	};
#ifdef USE_SDL2_RENDERER
	SDL_RenderFillRect(renderer, &box);
#else
	RenderingEngine::ogl_set_color(2, 2, 4, 255);
	RenderingEngine::ogl_fill_rect(box);
#endif
	// Right
	p1 = RenderingEngine::world_to_screen(Vector2f(X_MAX, -Y_MAX*2));
	p2 = RenderingEngine::world_to_screen(Vector2f(X_MAX*3, Y_MAX*3));
	box = {
		(int)p1.x(),
		(int)p1.y(),
		(int)(p2.x() - p1.x()),
		(int)(p2.y() - p1.y())
	};
#ifdef USE_SDL2_RENDERER
	SDL_RenderFillRect(renderer, &box);
#else
	RenderingEngine::ogl_fill_rect(box);
#endif
	// Bottom
	p1 = RenderingEngine::world_to_screen(Vector2f(X_MAX*3, Y_MAX));
	p2 = RenderingEngine::world_to_screen(Vector2f(-X_MAX*2, Y_MAX*3));
	box = {
		(int)p1.x(),
		(int)p1.y(),
		(int)(p2.x() - p1.x()),
		(int)(p2.y() - p1.y())
	};
#ifdef USE_SDL2_RENDERER
	SDL_RenderFillRect(renderer, &box);
#else
	RenderingEngine::ogl_fill_rect(box);
#endif
	// Left
	p1 = RenderingEngine::world_to_screen(Vector2f(0, -Y_MAX*2));
	p2 = RenderingEngine::world_to_screen(Vector2f(-X_MAX*2, Y_MAX*3));
	box = {
		(int)p1.x(),
		(int)p1.y(),
		(int)(p2.x() - p1.x()),
		(int)(p2.y() - p1.y())
	};
#ifdef USE_SDL2_RENDERER
	SDL_RenderFillRect(renderer, &box);
#else
	RenderingEngine::ogl_fill_rect(box);
#endif

	// Draw rectangles
	/*std::vector<SDL_Rect> rects;
	for(auto& r : rectcover){
		SDL_SetRenderDrawColor(renderer, 127, 255, 255, 64);
		auto lows = RenderingEngine::world_to_screen(Vector2f(r.xl * tile_width, r.yl * tile_height));
		auto highs = RenderingEngine::world_to_screen(Vector2f(r.xh * tile_width, r.yh * tile_height));
		SDL_Rect box = {
			(int)lows.x(),
			(int)lows.y(),
			(int)(highs.x() - lows.x()),
			(int)(highs.y() - lows.y())
		};
#ifdef USE_SDL2_RENDERER
		rects.push_back(box); //SDL_RenderDrawRect(renderer, &box);
#else
		RenderingEngine::ogl_set_color(127, 255, 255, 64);
		RenderingEngine::ogl_draw_rect(box);
#endif
	}

#ifdef USE_SDL2_RENDERER
	if (rects.size() > 0) SDL_RenderDrawRects(renderer, &rects[0], rects.size());
#else
	RenderingEngine::ogl_send_rects_to_draw();
#endif
	*/

	// Draw units
	for (auto unit : units)
		if (unit.get())
			unit->render(renderer);

	// draw projectiles
	for (auto p : projectiles)
		if (p.get())
			p->render(renderer);

#ifndef USE_SDL2_RENDERER
	RenderingEngine::ogl_send_lines_to_draw();
	RenderingEngine::ogl_send_rects_to_draw();
#endif
}

void MapLevel::update(float elapsed_time) {
	// Make buffered ids available for use
	for(int i = 0; i < (int)idbuffer.size(); i++){
		idstack.push_back(idbuffer.back());
		idbuffer.pop_back();
	}

	//if(!(rand() % 100)){
	//	std::cout << "Adding projectile" << std::endl;
	//	add_proj(std::make_shared<projectile>(Vector2f(100, 100), Vector2f(10, 0), 2, 200, 0, 1, *this));
	//}

	// left click
	if (Input::is_mouse_pressed(SDL_BUTTON_LEFT)) {
		if (!lbutton_down) {
			// Button down
			std::pair<int, int> pos = Input::get_mouse_pos();
			Vector2f world_pos = RenderingEngine::screen_to_world(pos.first, pos.second);
			std::cout << "Click at " << world_pos << std::endl;
			lbutton_down = true;

			Vector2f rally_point(managers[0].rally_point.x(), world_pos.y());
			managers[0].groups.push_back(std::make_shared<attack_group>(group_size, world_pos, rally_point, std::shared_ptr<MapLevel>(this)));
		}
	}
	else{
		if(lbutton_down){
			lbutton_down = false;
		}
	}

	// right click
	if (Input::is_mouse_pressed(SDL_BUTTON_RIGHT)) {
		if (!rbutton_down) {
			// Button down
			std::pair<int, int> pos = Input::get_mouse_pos();
			Vector2f world_pos = RenderingEngine::screen_to_world(pos.first, pos.second);
			std::cout << "Right Click at " << world_pos << std::endl;
			rbutton_down = true;

			//Vector2f rally_point(managers[0].rally_point.x(), world_pos.y());
			managers[0].groups.push_back(std::make_shared<defense_group>(group_size, world_pos, Vector2f(0, 0), std::shared_ptr<MapLevel>(this), 6.0f));
		}
	}
	else{
		if(rbutton_down){
			rbutton_down = false;
		}
	}

	static int ctr = 0;
	ctr = (ctr+1)%update_groups;

	bool alive;

	// update projectiles
	for(auto& p : projectiles){
		if(!p)
			continue;

		alive = p->update();

		if(!alive){
			kill_proj(p->id);
		}
	}

	// update groups
	for(auto& m : managers){
		m.update();
	}

	// update units
	for (int i = 0; i < (int)units.size(); i++) {
		auto& unit = units[i];
		if (unit.get()) {
			// FIXME CANNOT BE PARALLELIZED
			auto tile = unit->get_tile();
			alive = unit->update(elapsed_time, i % update_groups == ctr);
			auto ntile = unit->get_tile();

			if (ntile != tile) {
				// make sure in team appropriate datastructure
				auto& ug = unitgrid[unit->team];
				auto& vec = ug[tile.first][tile.second];
				remove(vec, unit->id);
				ug[ntile.first][ntile.second].push_back(unit->id);
			}

			if(!alive){
				kill(unit->id);
			}
		}
	}

	if(Input::is_key_pressed(dissolve_key)){
		if(!dissolve_down){
			// dissolve all
			for(auto& g : managers[0].groups){
				g->marked = 1;
			}
		}
		
		dissolve_down = true;
	}
	else{
		dissolve_down = 0;
	}

	// update group size
	bool n_keydown = Input::is_key_pressed(SDLK_n);
	bool m_keydown = Input::is_key_pressed(SDLK_m);
	if (!n_keydown && (group_size_keydown & 0x01))
		group_size += 10;
	if (!m_keydown && (group_size_keydown & 0x02))
		group_size -= 10;

	if (group_size < 10)
		group_size = 10;

	group_size_keydown = (n_keydown ? 0x01 : 0x00) | (m_keydown ? 0x02 : 0x00);
}

// takes operator >
struct minheap {
	std::vector<int> vec;
	std::function<bool(int, int)> comp;

	minheap(std::vector<int> v, std::function<bool(int, int)> comp): comp(comp) {
		vec = v;
		std::make_heap(v.begin(), v.end(), comp);
	}

	int pop() {
		std::pop_heap(vec.begin(), vec.end(), comp);
		auto an = vec.back();
		vec.pop_back();
		return an;
	}

	void insert(int an) {
		vec.push_back(an);
		std::make_heap(vec.begin(), vec.end(), comp);
	}

	size_t size() {
		return vec.size();
	}

	void reheap(){
		std::make_heap(vec.begin(), vec.end(), comp);
	}
};

std::vector<Vector2f> MapLevel::reconstruct_path(std::vector<int>& from, std::vector<Vector2f>& points, int src, int dest, Vector2f v2fdest) {
	auto curr = dest;

	std::vector<Vector2f> path;
	path.push_back(v2fdest);

	//std::cout << "Constructing path: " << std::endl;

	// TODO possibly include current node in path?
	while (curr != src && curr != -1) {
		//std::cout << "Rectangle: " << curr << std::endl;
		//std::cout << "Point: " << points[curr] << std::endl;
		//auto t = to_tile_space(points[curr]);
		//std::cout << "Tile: " << t.first << ", " << t.second << std::endl;
		path.push_back(points[curr]);
		curr = from[curr];
	}

	//std::reverse(path.begin(), path.end());
	//path.push_back(v2fdest);

	return path;
}

std::vector<Vector2f> MapLevel::reconstruct_better_path(std::vector<int>& from, std::vector<Vector2f>& points, int src, int dest, Vector2f v2fsrc, Vector2f v2fdest) {
	//std::cout << "\tin reconstruct_better_path" << std::endl;

	auto curr = dest;

	std::vector<world_rect> interfaces;
	std::vector<Vector2f> path;
	interfaces.push_back(world_rect(v2fdest));
	path.push_back(v2fdest);


	// first pass
	while (curr != src && curr != -1) {
		interfaces.push_back(to_world_rect(intersection(rectcover[curr], rectcover[from[curr]])));
		path.push_back(points[curr]);
		curr = from[curr];
	}
	interfaces.push_back(world_rect(v2fsrc));
	path.push_back(v2fsrc);

	/*std::cout << "\tconstructing path" << std::endl;
	path.push_back(v2fdest);
	for(size_t i = 1; i < interfaces.size(); i++){
		std::cout << "\tadding " << interfaces[i] << " center: " << interfaces[i].center() << std::endl;

		auto c = interfaces[i].center();
		path.push_back(Vector2f(c.xl, c.yl));
	}*/

	// pad is specified in terms of tile_width
	auto smooth = [](Vector2f a, world_rect b, Vector2f c, float pad = 0) -> Vector2f {
		bool vert = b.xl == b.xh;

		auto cen = b.center();

		// vertical edge
		if(vert){
			if(a.x() == b.xl || a.x() == c.x() || b.xl == c.x()) return Vector2f(cen.xl, cen.yl);

			float m = (c.y() - a.y()) / (c.x() - a.x());
			float yatb = a.y() + m*(b.xl - a.x());

			pad = std::min(pad, (b.yh - b.yl)/2);
			float yfinal = std::clamp(yatb, b.yl + pad, b.yh - pad);

			return Vector2f(b.xl, yfinal);
		}
		// horizontal edge
		if(a.y() == b.yl || a.y() == c.y() || b.yl == c.y()) return Vector2f(cen.xl, cen.yl);

		float m = (c.x() - a.x()) / (c.y() - a.y());
		float xatb = a.x() + m*(b.yl - a.y());

		pad = std::min(pad, (b.xh - b.xl)/2);
		float xfinal = std::clamp(xatb, b.xl + pad, b.xh - pad);

		return Vector2f(xfinal, b.yl);
	};

	// smoothing
	for(size_t i = 1; i < path.size() - 1; i++){
		path[i] = smooth(path[i - 1], interfaces[i], path[i + 1], 2*tile_width);
	}

	// do not want starting point in path
	path.pop_back();

	/*std::cout << "path:";
	for(auto& v : path){
		std::cout << " (" << v.x() << ", " << v.y() << ")";
	}
	std::cout << std::endl;*/

	//std::cout << "\tleaving reconstruct_better_path" << std::endl;
	return path;
}

std::vector<Vector2f> MapLevel::find_rect_path(Vector2f s, Vector2f d) {

	//std::cout << "In find_rect_path" << std::endl;

	//std::vector<rect>& rectcover = pathcover;

	int coversize = rectcover.size();

	std::vector<int> from(rectcover.size(), -1);
	std::vector<float> gscore(rectcover.size(), std::numeric_limits<float>::infinity());
	std::vector<float> fscore(rectcover.size(), std::numeric_limits<float>::infinity());
	std::vector<Vector2f> point_in_r(rectcover.size());

	for(int i = 0; i < (int)rectcover.size(); i++){
		auto& r = rectcover[i];

		auto lows = to_world_space(std::make_pair(r.xl, r.yl));
		auto highs = to_world_space(std::make_pair(r.xh, r.yh));

		float xc = lows.x() + (highs.x() - lows.x());
		float yc = lows.y() + (highs.y() - lows.y());

		point_in_r[i] = Vector2f(xc, yc);
	}

	/*auto dv = [&](int li, int ri){
		rect& r = rectcover[ri];

		return closest_point(point_in_r[li], r);
	};*/

	auto h = [&](int i){
		return (point_in_r[i] - d).len();
	};

	auto s_tile = to_tile_space(s);
	if(!inbounds(obgrid, s_tile) || obgrid[s_tile.first][s_tile.second]) return {};
	int s_ind = grid_to_rectcover[s_tile.first][s_tile.second];
	auto d_tile = to_tile_space(d);
	if(!inbounds(obgrid, d_tile) || obgrid[d_tile.first][d_tile.second]) return {};
	int d_ind = grid_to_rectcover[d_tile.first][d_tile.second];

	//std::cout << "Not out of bounds" << std::endl;

	//std::cout << "d_tile " << d_tile.first << ", " << d_tile.second << std::endl;
	//std::cout << "sind/dind: " << s_ind << ", " << d_ind << std::endl;

	if(s_ind == -1 || d_ind == -1){
		//std::cout << "Return obstruction" << std::endl;
		return {};
	}

	//point_in_r[s_ind] = s;
	gscore[s_ind] = 0;
	fscore[s_ind] = h(s_ind);
	from[s_ind] = s_ind;

	minheap hp(std::vector<int>({s_ind}),
		[&](int l, int r) {
			return fscore[l] > fscore[r];
		}
	);

	int ctr = 0;

	while (hp.size()) {


		ctr++;


		auto current = hp.pop();

		if(ctr > coversize){
			std::cout << "Iteration " << ctr << "/" << coversize << ", processing " << current << "rectcover.size() is " << rectcover.size() << std::endl;	
		}

		//std::cout << "Processing " << current << std::endl;

		if (current == d_ind){
			//std::cout << "done" << std::endl;
			//return reconstruct_path(from, point_in_r, s_ind, d_ind, d);
			//std::cout << "Return reconstruct" << std::endl;
			return reconstruct_better_path(from, point_in_r, s_ind, d_ind, s, d);
		}

		// rectangle neighbors
		for (auto& nind : rectgraph[current]) {
			//std::cout << "neighbor " << nind << std::endl;

			//Vector2f tentative_npoint = dv(current, nind);
			float tentative_gscore = gscore[current] + (point_in_r[nind] - point_in_r[current]).len();
			float tentative_fscore = (point_in_r[nind] - d).len();

			//std::cout << "tentative gscore vs current: " << tentative_gscore << " vs " << gscore[nind] << std::endl;

			// update if closer
			if (tentative_gscore + tentative_fscore < gscore[nind] + fscore[nind]) {
				// update point
				//point_in_r[nind] = tentative_npoint;

				// update gscore
				gscore[nind] = tentative_gscore;

				// update fscore
				//fscore[nind] = h(nind);
				fscore[nind] = tentative_fscore;

				// update from
				from[nind] = current;

				auto it = std::find(hp.vec.begin(), hp.vec.end(), nind);
				if (it == hp.vec.end()) {
					//std::cout << nind << " not in heap" << std::endl;
					hp.insert(nind);
				}
				else {
					//std::cout << nind << " in heap" << std::endl;
					hp.reheap();
				}
			}
		}
	}

	//std::cout << "Return not found" << std::endl;
	return {};
}