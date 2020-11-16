#pragma once

#include <functional>
#include <set>
#include <time.h>
#include <vector>

#include "algorithms.h"
#include "GameObject.h"
#include "GameObject.cpp"
#include "Noise.cpp"

const int MIN_OCTAVE = 3;
const int MAX_OCTAVE = 20;
const float DENSITY = 0.35;

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
			if (noise[(j*tiles_y)+k] > threshold) {
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
		srand(SDL_GetTicks() + time(NULL));
		float persistence = (float)rand() / RAND_MAX;
		int n_octaves = (int)((float)rand() / RAND_MAX * (MAX_OCTAVE - MIN_OCTAVE + 1)) + MIN_OCTAVE;
		int prime_index = (int)((float)rand() / RAND_MAX * 11);
		int offset_x = (int)((float)rand() / RAND_MAX * 10000);
		int offset_y = (int)((float)rand() / RAND_MAX * 10000);

		obs.clear();
		std::cout << " n_octaves: " << n_octaves << " persistence: " << persistence << " prime_index: " << prime_index << std::endl;
		float threshold = 0.1f;
		double noise[tiles_x*tiles_y-1];
		for (int j = 0; j < tiles_x; j++)
			for (int k = 0; k < tiles_y; k++)
				noise[(j*tiles_y)+k] = ValueNoise_2D(j+offset_x, k+offset_y, n_octaves, persistence, prime_index);

		while ((int)obs.size() < min && threshold < 1.0f) {
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
	// std::vector<std::pair<int, int>> obs;
	std::vector<std::pair<int, int>> path;
	bool found = false;
	while (!found) {
		obstructions.clear();

		// Create random obstructions
		obstructions = random_obstructions(bases, tiles_x * tiles_y * DENSITY);

		// Check if path exists among all bases
		found = true;
		for (int j = 0; j < (int)bases.size(); j++) {
			for (int k = j+1; k < (int)bases.size(); k++) {
				auto a = bases.at(j);
				auto b = bases.at(k);
				for (auto& o : obstructions) {
					if (o.first == a.first && o.second == a.second) {
						found = false;
						break;
					}
				}
				path.clear();
				path = astar(this, a, b);
				std::cout << path.size() << std::endl;
				for (auto& p : path)
					std::cout << "(" << p.first << ", " << p.second << ")" << std::endl;
				if (path.size() == 0) {
					std::cout << "not found" << std::endl;
					found = false;
					break;
				}
			}
			if (!found)
				break;
		}
	}
	return obstructions;
}

void MapLevel::render(SDL_Renderer* renderer) {
	// Tiles
	for (int j = 0; j < tiles_x; j++) {
		for (int k = 0; k < tiles_y; k++) {
			// Outline
			SDL_Rect box = {tile_width*j, tile_height*k, tile_width, tile_height};
			float opacity = sin(j+2*k+SDL_GetTicks()/2000.0f)/2.0f+0.5f;
			SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, (int)(255*opacity));
			SDL_RenderDrawRect(renderer, &box);
			// Fill
			opacity = sin(j+k+SDL_GetTicks()/2000.0f)/2.0f+0.5f;
			SDL_SetRenderDrawColor(renderer, 0x77, 0x77, 0x77, (int)(55*opacity)+200);
			SDL_RenderFillRect(renderer, &box);
		}
	}
	// Obstacles
	for (auto& o : obstructions) {
		SDL_Rect box = {tile_width*o.first, tile_height*o.second, tile_width, tile_height};
		// float opacity = sin(o.first+o.second+SDL_GetTicks()/2000.0f)/2.0f+0.5f;
		// SDL_SetRenderDrawColor(renderer, 0x99, 0x99, 0x99, (int)(200*opacity)+55);
		SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xEE);
		SDL_RenderFillRect(renderer, &box);
	}
}