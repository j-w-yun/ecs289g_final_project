#pragma once

#include <functional>
#include <set>
#include <time.h>
#include <vector>

#include "algorithms.h"
#include "AStar.hpp"
#include "AStar.cpp"
#include "GameObject.h"
#include "GameObject.cpp"
#include "Noise.cpp"

const int MIN_OCTAVE = 1;
const int MAX_OCTAVE = 12;
const float DENSITY = 0.5;

MapLevel::MapLevel() {
	class_string = MapLevel::static_class();
}

void MapLevel::set_size(int x, int y, int w, int h) {
	tiles_x = x;
	tiles_y = y;
	tile_width = w;
	tile_height = h;
}

void MapLevel::set_obstructions(std::vector<std::pair<int, int>> o) {
	obstructions = o;
}

void MapLevel::set(int x, int y, int w, int h, std::vector<std::pair<int, int>> o) {
	set_size(x, y, w, h);
	set_obstructions(o);
}

bool MapLevel::climb(std::vector<std::pair<int, int>>* obs, double noise[], float threshold, std::vector<std::pair<int, int>> bases) {
	for (int j = 0; j < tiles_x; j++) {
		for (int k = 0; k < tiles_y; k++) {
			if (noise[(j*tiles_y)+k] < threshold) {
				bool valid = true;
				for (auto& b : bases)
					if (j == b.first && k == b.second)
						valid = false;
				if (!valid)
					return false;
				obs->push_back(std::make_pair(j, k));
			}
		}
	}
	return true;
}

std::vector<std::pair<int, int>> MapLevel::random_obstructions(std::vector<std::pair<int, int>> bases, int min) {
	std::vector<std::pair<int, int>> obs;
	while ((int)obs.size() < min) {
		// Seed random
		srand((int)time(NULL)+(int)SDL_GetTicks());
		float persistence = ((float)rand() / RAND_MAX * 0.8f) + 0.2f;
		int n_octaves = (int)((float)rand() / RAND_MAX * (MAX_OCTAVE - MIN_OCTAVE + 1)) + MIN_OCTAVE;
		int prime_index = (int)((float)rand() / RAND_MAX * 11);
		int offset_x = (int)((float)rand() / RAND_MAX * 10000);
		int offset_y = (int)((float)rand() / RAND_MAX * 10000);

		obs.clear();
		std::cout << " n_octaves: " << n_octaves << " persistence: " << persistence << " prime_index: " << prime_index << std::endl;
		float threshold = 0.01f;
		double noise[tiles_x*tiles_y-1];
		for (int j = 0; j < tiles_x; j++)
			for (int k = 0; k < tiles_y; k++)
				noise[(j*tiles_y)+k] = ValueNoise_2D(j+offset_x, k+offset_y, n_octaves, persistence, prime_index);

		while ((int)obs.size() < min && threshold < 0.9f) {
			obs.clear();
			std::cout << "threshold: " << threshold << std::endl;
			if (!climb(&obs, noise, threshold, bases))
				break;
			threshold += 0.01f;
		}
	}
	return obs;
}

std::vector<std::pair<int, int>> MapLevel::generate_obstructions(std::vector<std::pair<int, int>> bases) {
	std::vector<std::pair<int, int>> obs;
	// std::vector<std::pair<int, int>> path;
	std::vector<AStar::Vec2i> path;
	MapLevel* level = new MapLevel();
	level->set_size(tiles_x, tiles_y, tile_width, tile_height);
	bool found = false;
	while (!found) {
		obs.clear();

		// Create random
		obs = random_obstructions(bases, tiles_x * tiles_y * DENSITY);

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
				// path = astar(*level, a, b);
				path = find_path(*level, a, b);
				if (path.size() == 0) {
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

void MapLevel::render(SDL_Renderer* renderer) {
	// Tiles
	for (int j = 0; j < tiles_x; j++) {
		for (int k = 0; k < tiles_y; k++) {
			// Fill
			SDL_Rect box = {tile_width*j, tile_height*k, tile_width, tile_height};
			// opacity = sin(j+k+SDL_GetTicks()/2000.0f)/2.0f+0.5f;
			// SDL_SetRenderDrawColor(renderer, 0x77, 0x77, 0x77, (int)(55*opacity)+200);
			SDL_SetRenderDrawColor(renderer, 0x77, 0x77, 0x77, 0xFF);
			SDL_RenderFillRect(renderer, &box);
			// Outline
			// float opacity = sin(j+2*k+SDL_GetTicks()/2000.0f)/2.0f+0.5f;
			// SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, (int)(255*opacity));
			SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
			SDL_RenderDrawRect(renderer, &box);
		}
	}
	// Obstacles
	for (auto& o : obstructions) {
		SDL_Rect box = {tile_width*o.first, tile_height*o.second, tile_width, tile_height};
		// float opacity = sin(o.first+o.second+SDL_GetTicks()/2000.0f)/2.0f+0.5f;
		// SDL_SetRenderDrawColor(renderer, 0x99, 0x99, 0x99, (int)(200*opacity)+55);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderFillRect(renderer, &box);
	}
}
