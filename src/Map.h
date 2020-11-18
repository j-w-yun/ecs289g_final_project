#pragma once

#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <time.h>

#include "GameObject.h"
#include "GameObject.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "AStar.hpp"

typedef std::pair<int, int> ip;

class MapLevel: public GameObject {
	private:
		unsigned long seed = time(NULL);
		int min_octave;
		int max_octave;
		int tiles_x;
		int tiles_y;
		int tile_width;
		int tile_height;
		std::vector<ip> obstructions;
		std::vector<std::vector<bool>> obgrid;
		std::vector<std::shared_ptr<GameObject>> units;
		std::vector<size_t> idstack;
		std::vector<std::vector<std::vector<size_t>>> unitgrid;
		size_t unitcap;
		bool climb(std::vector<std::pair<int, int>>* obs, double noise[], float threshold, std::vector<std::pair<int, int>> bases, int padding);

	public:
		MapLevel() = default;
		MapLevel(int tx, int ty, int tw, int th, size_t uc = 200);
		bool add(std::shared_ptr<GameObject> o);
		void set_size(int x, int y, int w, int h);
		void set_obstructions(std::vector<std::pair<int, int>> o);
		void set(int x, int y, int w, int h, std::vector<std::pair<int, int>> o);
		std::vector<std::pair<int, int>> random_obstructions(std::vector<std::pair<int, int>> bases, int min, int padding);
		std::vector<std::pair<int, int>> generate_obstructions(std::vector<std::pair<int, int>> bases, int padding);
		int get_width() {return tiles_x;};
		int get_height() {return tiles_y;};
		std::vector<ip> get_obstructions() {return obstructions;};
		std::vector<std::vector<bool>> get_obgrid() {return obgrid;};
		std::vector<std::shared_ptr<GameObject>>& get_objects();
		size_t get_unitcap() {return unitcap;}
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
		static std::string static_class() {return "MapLevel";};
};
