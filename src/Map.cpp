#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <math.h>
#include <numeric>
#include <set>
#include <time.h>
#include <vector>

#include "algorithms.h"
#include "AStar.hpp"
#include "AStar.cpp"
#include "GameObject.h"
#include "GameObject.cpp"
#include "Noise.cpp"
#include "RenderingEngine.h"
#include "Util.h"

const float MIN_DENSITY = 0.05;
const float MAX_DENSITY = 0.25;
const int update_groups = 100;

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

	// init stack
	for (size_t i = 0; i < uc; i++) {
		idstack.push_back(i);
	}

	// init objects
	units = std::vector<std::shared_ptr<GameObject>>(unitcap);

	// init unitgrid
	unitgrid = std::vector<std::vector<std::vector<size_t>>>(tiles_x, std::vector<std::vector<size_t>>(tiles_y));

	// init obgrid
	obgrid = std::vector<std::vector<bool>>(tx, std::vector<bool>(ty, 0));
}

bool MapLevel::add(std::shared_ptr<GameObject> o) {
	if (!idstack.size()) return false;

	auto id = idstack.back();
	idstack.pop_back();

	units[id] = o;
	auto tile = o->get_tile();

	unitgrid[tile.first][tile.second].push_back(id);
	o->id = id;

	std::cout << "Adding unit " << id << std::endl;

	return true;
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

	// init unitgrid
	unitgrid = std::vector<std::vector<std::vector<size_t>>>(tiles_x, std::vector<std::vector<size_t>>(tiles_y));

	// init obgrid
	obgrid = std::vector<std::vector<bool>>(tiles_x, std::vector<bool>(tiles_y, 0));
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

const int n_types = 3;
std::map<std::pair<int, int>, int> generate_texture(int width, int height) {
	std::map<std::pair<int, int>, int> tex;
	std::vector<std::pair<int, int>> ind;
	float persistence = Util::uniform_random(0.5, 1.2);
	int n_octaves = Util::uniform_random(1, 12);
	int prime_index = (int)Util::uniform_random(0, 11);
	int offset_x = (int)Util::uniform_random(0, 100000);
	int offset_y = (int)Util::uniform_random(0, 100000);
	std::vector<double> noise;
	noise.resize(width * height);
	for (int j = 0; j < width; j++) {
		for (int k = 0; k < height; k++) {
			double z = perlin_noise(j+offset_x, k+offset_y, n_octaves, persistence, prime_index);
			noise[(j*height)+k] = z;
			// TODO
		}
	}
	for (auto i : sort_indexes(noise)) {
		std::cout << noise[i] << std::endl;
	}
	return tex;
}

void MapLevel::render(SDL_Renderer* renderer) {
	// Draw tiles
	const float X_MIN = 0;
	const float Y_MIN = 0;
	const float X_MAX = tile_width * tiles_x;
	const float Y_MAX = tile_height * tiles_y;
	Vector2f world1 = RenderingEngine::world_to_screen(Vector2f(X_MIN, Y_MIN));
	Vector2f world2 = RenderingEngine::world_to_screen(Vector2f(X_MAX, Y_MAX));
	SDL_Rect world_box = {
		(int)(world1.x()),
		(int)(world1.y()),
		(int)(world2.x()-world1.x()),
		(int)(world2.y()-world1.y())
	};
	SDL_SetRenderDrawColor(renderer, 239, 196, 121, 255);
	SDL_RenderFillRect(renderer, &world_box);

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
			// Fill
			SDL_SetRenderDrawColor(renderer, 239, 196, 121, 255);
			SDL_RenderFillRect(renderer, &box);
			// Outline
			SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 255);
			SDL_RenderDrawRect(renderer, &box);
		}
	}*/

	// Draw perlin noise
	const float RESOLUTION = 5.0f;
	if (noise2d.size() == 0 || Input::is_key_pressed(SDLK_SPACE)) {
		noise2d.clear();
		noise2d.resize(ceil((X_MAX-X_MIN)/RESOLUTION));

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
		for (float x = X_MIN; x < X_MAX; x += RESOLUTION) {
			for (float y = Y_MIN; y < Y_MAX; y += RESOLUTION) {
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
	}
	if (noise2d.size() > 0) {
		// Normalize noise from 0 to 1 and draw
		int xi = 0;
		for (float x = X_MIN; x < X_MAX; x += RESOLUTION) {
			int yi = 0;
			for (float y = Y_MIN; y < Y_MAX; y += RESOLUTION) {
				// Choose color according to Z
				double z = noise2d[xi][yi++];
				// Draw overlapping squares
				Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(x-RESOLUTION, y-RESOLUTION));
				Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f(x+RESOLUTION*2, y+RESOLUTION*2));
				SDL_Rect box = {
					(int)(sp1.x()),
					(int)(sp1.y()),
					(int)(sp2.x()-sp1.x())+1,
					(int)(sp2.y()-sp1.y())+1
				};
				float f = ((float)SDL_GetTicks()/400.0f+(x+y+1)/10);
				SDL_SetRenderDrawColor(renderer, 40, 90*z+40, 30, 150+60*sin(f));
				// SDL_SetRenderDrawColor(renderer, 60, 90*z+40, 30, 150);
				SDL_RenderFillRect(renderer, &box);
			}
			xi++;
		}
	}

	// Draw obstacles
	for (auto& o : obstructions) {
		Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(o.first*tile_width, o.second*tile_height));
		Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((o.first+1)*tile_width, (o.second+1)*tile_height));
		SDL_Rect box = {
			(int)(sp1.x()),
			(int)(sp1.y()),
			(int)(sp2.x()-sp1.x())+1,
			(int)(sp2.y()-sp1.y())+1
		};
		// Fill
		float f = ((float)SDL_GetTicks()/1000.0f+(o.first+o.second+1)/4);
		SDL_SetRenderDrawColor(renderer, 40, 40, 190+50*sin(f), 127);
		// SDL_SetRenderDrawColor(renderer, 32, 32, 190, 255);
		SDL_RenderFillRect(renderer, &box);
	}

	// Draw rectangles
	for(auto& r : rectcover){
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 127);
		auto lows = RenderingEngine::world_to_screen(Vector2f(r.xl * tile_width, r.yl * tile_height));
		auto highs = RenderingEngine::world_to_screen(Vector2f(r.xh * tile_width, r.yh * tile_height));
		SDL_Rect box = {
			(int)lows.x(),
			(int)lows.y(),
			(int)(highs.x() - lows.x()),
			(int)(highs.y() - lows.y())
		};
		SDL_RenderDrawRect(renderer, &box);
	}

	for (auto unit : units)
		if (unit.get())
			unit->render(renderer);
}

template<class T>
void remove(std::vector<T>& vec, T val) {
	for (size_t i = 0; i < vec.size(); i++) {
		if (vec[i] == val) {
			vec.erase(vec.begin() + i);
			return;
		}
	}
}

void MapLevel::update(float elapsed_time) {
	//static int ctr = 0;

	/*if(Input::is_mouse_pressed(SDL_BUTTON_RIGHT)){
		auto temp = Input::get_mouse_pos();
		auto path = find_rect_path(Vector2f(10, 10), Vector2f(temp.first, temp.second));

		std::cout << "path from 10, 10 to " << temp.first << ", " << temp.second << ": " << std::endl;
		for(auto& a : path){
			std::cout << "\t" << a << std::endl;
		}
	}*/

	static int ctr = 0;
	ctr = (ctr+1)%update_groups;

	for (int i = 0; i < (int)units.size(); i++) {
		auto& unit = units[i];
		if (unit.get()) {
			// FIXME CANNOT BE PARALLELIZED
			auto tile = unit->get_tile();
			unit->update(elapsed_time, i % update_groups == ctr);
			auto ntile = unit->get_tile();

			//std::cout << "pos " << unit->p() << std::endl;
			//std::cout << "ntile " << ntile.first << ", " << ntile.second << std::endl;

			if (ntile != tile) {
				auto& vec = unitgrid[tile.first][tile.second];
				remove(vec, unit->id);
				unitgrid[ntile.first][ntile.second].push_back(unit->id);
			}
		}

		/*if(!ctr){
			for(int i = 0; i < (int)unitgrid.size(); i++){
				for(int j = 0; j < (int)unitgrid[0].size(); j++){
					if(unitgrid[i][j].size()){
						std::cout << i << ", " << j << ": ";
						for(auto& a : unitgrid[i][j]){
							std::cout << a << " ";
						}
						std::cout << std::endl;
					}
				}
			}
		}
		ctr = (ctr + 1)%100;*/
	}
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

std::vector<Vector2f> MapLevel::find_rect_path(Vector2f s, Vector2f d) {
	//std::cout << "In find_rect_path" << std::endl;

	//std::vector<rect>& rectcover = pathcover;

	std::vector<int> from(rectcover.size(), -1);
	std::vector<float> gscore(rectcover.size(), std::numeric_limits<float>::infinity());
	std::vector<float> fscore(rectcover.size(), std::numeric_limits<float>::infinity());
	std::vector<Vector2f> point_in_r(rectcover.size());

	auto dv = [&](int li, int ri){
		rect& r = rectcover[ri];

		return closest_point(point_in_r[li], r);
	};

	auto h = [&](int i){
		return (point_in_r[i] - d).len();
	};

	auto s_tile = to_tile_space(s);
	if(!inbounds(obgrid, s_tile) || obgrid[s_tile.first][s_tile.second]) return {};
	int s_ind = grid_to_rectcover[s_tile.first][s_tile.second];
	auto d_tile = to_tile_space(d);
	if(!inbounds(obgrid, d_tile) || obgrid[d_tile.first][d_tile.second]) return {};
	int d_ind = grid_to_rectcover[d_tile.first][d_tile.second];


	//std::cout << "d_tile " << d_tile.first << ", " << d_tile.second << std::endl;
	//std::cout << "sind/dind: " << s_ind << ", " << d_ind << std::endl;

	if(s_ind == -1 || d_ind == -1){
		return {};
	}

	point_in_r[s_ind] = s;
	gscore[s_ind] = 0;
	fscore[s_ind] = h(s_ind);
	from[s_ind] = s_ind;

	minheap hp(std::vector<int>({s_ind}),
		[&](int l, int r) {
			return fscore[l] > fscore[r];
		}
	);

	while (hp.size()) {

		auto current = hp.pop();

		//std::cout << "Processing " << current << std::endl;

		if (current == d_ind){
			//std::cout << "done" << std::endl;
			return reconstruct_path(from, point_in_r, s_ind, d_ind, d);
		}

		// rectangle neighbors
		for (auto& nind : rectgraph[current]) {
			//std::cout << "neighbor " << nind << std::endl;

			Vector2f tentative_npoint = dv(current, nind);
			float tentative_gscore = gscore[current] + (tentative_npoint - point_in_r[current]).len();

			//std::cout << "tentative gscore vs current: " << tentative_gscore << " vs " << gscore[nind] << std::endl;

			// update if closer
			if (tentative_gscore < gscore[nind]) {
				// update point
				point_in_r[nind] = tentative_npoint;

				// update gscore
				gscore[nind] = tentative_gscore;

				// update fscore
				fscore[nind] = h(nind);

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

	return {};
}