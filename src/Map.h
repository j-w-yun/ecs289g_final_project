#pragma once

#include <vector>
#include <set>
#include <functional>
#include <memory>

#include "GameObject.h"
#include "GameObject.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"

typedef std::pair<int, int> ip;

class MapLevel: public GameObject {
	private:
		int tiles_x;
		int tiles_y;
		int tile_width;
		int tile_height;
		std::vector<ip> obstructions;
		bool climb(std::vector<std::pair<int, int>>* obs, double noise[], float threshold, std::vector<std::pair<int, int>> bases);

	public:
		MapLevel();
		void set_size(int x, int y, int w, int h);
		void set_obstructions(std::vector<std::pair<int, int>> o);
		void set(int x, int y, int w, int h, std::vector<std::pair<int, int>> o);
		std::vector<std::pair<int, int>> random_obstructions(std::vector<std::pair<int, int>> bases, int min);
		std::vector<std::pair<int, int>> generate_obstructions(std::vector<std::pair<int, int>> bases);
		int get_width() {return tiles_x;};
		int get_height() {return tiles_y;};
		std::vector<ip> get_obstructions() {return obstructions;};
		void render(SDL_Renderer* renderer);
		static std::string static_class() {return "MapLevel";};
};
