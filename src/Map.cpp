#pragma once

#include <functional>
#include <math.h>
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

const float DENSITY = 0.3;
const int update_groups = 100;

MapLevel::MapLevel(int tx, int ty, int tw, int th, size_t uc): tiles_x(tx), tiles_y(ty), tile_width(tw), tile_height(th), unitcap(uc) {
	class_string = MapLevel::static_class();

	// const int MIN_L = tx<ty ? tx : ty;
	// const int MAX_L = tx>=ty ? tx : ty;
	// min_octave = MAX_L - MIN_L;
	// max_octave = sqrt(tx*ty)/4;
	min_octave = 1;
	max_octave = sqrt((tx*ty)/(tx+ty));
	max_octave = max_octave < 3 ? 3 : max_octave;
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
				// for (auto& b : bases)
				// 	if (j == b.first && k == b.second)
				// 		valid = false;
				// if (!valid)
				// 	return false;
				// obs->push_back(std::make_pair(j, k));

				// // Yikes
				// for (auto& base : bases) {
				// 	for (int n = -padding; n <= padding; n++) {
				// 		for (int m = -padding; m <= padding; m++) {
				// 			int x = base.first+n;
				// 			int y = base.second+m;
				// 			if (j == x && k == y) {
				// 				valid = false;
				// 				break;
				// 			}
				// 		}
				// 		if (!valid) break;
				// 	}
				// 	if (!valid) break;
				// }
				// if (valid)
				// 	obs->push_back(std::make_pair(j, k));

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

std::vector<std::pair<int, int>> MapLevel::random_obstructions(std::vector<std::pair<int, int>> bases, int min, int padding) {
	std::vector<std::pair<int, int>> obs;
	float threshold;
	float persistence = ((float)rand()/RAND_MAX * 0.8f) + 0.4f;
	int n_octaves = (int)((float)rand()/RAND_MAX * (max_octave-min_octave+1)) + min_octave;
	int prime_index = (int)((float)rand()/RAND_MAX * 11);
	int offset_x = (int)((float)rand()/RAND_MAX * 10000);
	int offset_y = (int)((float)rand()/RAND_MAX * 10000);
	while ((int)obs.size() < min) {
		// Seed random
		seed += SDL_GetTicks() + 1;
		srand(seed);

		// Random noise params
		persistence = ((float)rand()/RAND_MAX * 0.6f) + 0.5f;
		n_octaves = (int)((float)rand()/RAND_MAX * (max_octave-min_octave+1)) + min_octave;
		prime_index = (int)((float)rand()/RAND_MAX * 11);
		offset_x = (int)((float)rand()/RAND_MAX * 10000);
		offset_y = (int)((float)rand()/RAND_MAX * 10000);

		obs.clear();
		// std::cout << " n_octaves: " << n_octaves << " persistence: " << persistence << " prime_index: " << prime_index << std::endl;
		threshold = 0.0005f;
		// Use std::vector here since variable length arrays are not supported in MSVC, as it is not standard.
		std::vector<double> noise;
		noise.resize(tiles_x*tiles_y);
		for (int j = 0; j < tiles_x; j++)
			for (int k = 0; k < tiles_y; k++)
				noise[(j*tiles_y)+k] = ValueNoise_2D(j+offset_x, k+offset_y, n_octaves, persistence, prime_index);

		while ((int)obs.size() < min && threshold < persistence) {
			obs.clear();
			if (!climb(&obs, &noise[0], threshold, bases, padding)) {
				std::cout << "failed at threshold: " << threshold << std::endl;
				break;
			}
			threshold += 0.0005f;
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
		obs = random_obstructions(bases, tiles_x * tiles_y * DENSITY, padding);

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

	// // Clear obstructions around base
	// for (auto& base : bases) {
	// 	for (int j = -padding; j <= padding; j++) {
	// 		for (int k = -padding; k <= padding; k++) {
	// 			int x = base.first+j;
	// 			int y = base.second+k;
	// 			auto i = std::begin(obs);
	// 			while (i != std::end(obs)) {
	// 				if ((*i).first == x && (*i).second == y)
	// 					i = obs.erase(i);
	// 				else
	// 					++i;
	// 			}
	// 		}
	// 	}
	// }

	return obs;
}

void MapLevel::render(SDL_Renderer* renderer) {
	// // Tiles
	// for (int j = 0; j < tiles_x; j++) {
	// 	for (int k = 0; k < tiles_y; k++) {
	// 		// Fill
	// 		SDL_Rect box = {tile_width*j, tile_height*k, tile_width, tile_height};
	// 		SDL_SetRenderDrawColor(renderer, 0x77, 0x77, 0x77, 0xFF);
	// 		SDL_RenderFillRect(renderer, &box);
	// 		// Outline
	// 		SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
	// 		SDL_RenderDrawRect(renderer, &box);
	// 	}
	// }
	// // Obstacles
	// for (auto& o : obstructions) {
	// 	SDL_Rect box = {tile_width*o.first, tile_height*o.second, tile_width, tile_height};
	// 	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	// 	SDL_RenderFillRect(renderer, &box);
	// }

	// Tiles
	for (int j = 0; j < tiles_x; j++) {
		for (int k = 0; k < tiles_y; k++) {
			// Fill
			// SDL_Rect box = {tile_width*j, tile_height*k, tile_width, tile_height};
			Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(j, k));
			Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f(j+1, k+1)); // +1 should be world tile width or height
			SDL_Rect box = {
				(int)(sp1.x()),
				(int)(sp1.y()),
				(int)(sp2.x()-sp1.x()),
				(int)(sp2.y()-sp1.y())
			};
			SDL_SetRenderDrawColor(renderer, 0x77, 0x77, 0x77, 0xFF);
			SDL_RenderFillRect(renderer, &box);
			// Outline
			SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
			SDL_RenderDrawRect(renderer, &box);
		}
	}
	// Obstacles
	for (auto& o : obstructions) {
		// SDL_Rect box = {tile_width*o.first, tile_height*o.second, tile_width, tile_height};
		Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f(o.first, o.second));
		Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f(o.first+1, o.second+1)); // +1 should be world tile width or height
		SDL_Rect box = {
			(int)(sp1.x()),
			(int)(sp1.y()),
			(int)(sp2.x()-sp1.x()),
			(int)(sp2.y()-sp1.y())
		};
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderFillRect(renderer, &box);
	}

	for (auto unit : units) {
		if (unit.get()) {
			unit->render(renderer);
		}
	}
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
	int s_ind = grid_to_rectcover[s_tile.first][s_tile.second];
	auto d_tile = to_tile_space(d);
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