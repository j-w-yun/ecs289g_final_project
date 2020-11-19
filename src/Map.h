#pragma once

#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <time.h>
#include <stdlib.h>

#include "GameObject.h"
#include "GameObject.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "AStar.hpp"

struct rect{
	int xl, yl, xh, yh;

	rect() = default;
	rect(int xl, int yl, int xh, int yh): xl(xl), yl(yl), xh(xh), yh(yh) {}
};

std::ostream& operator<<(std::ostream& os, rect r){
	os << "r(" << r.xl << ", " << r.yl << ", " << r.xh << ", " << r.yh << ")";
	return os;
}

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
		std::vector<rect> rectcover;
		std::vector<std::vector<size_t>> grid_to_rectcover;
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

		template<class T>
		bool inbounds(std::vector<std::vector<T>>& grid, int x, int y){
			return x >= 0 && x < (int)grid.size() && y >= 0 && y < (int)grid[0].size();
		}

		std::vector<ip> neighbors(std::vector<std::vector<int>>& grid, int x, int y) {
			auto inbounds = [&](int x, int y) {
				return x >= 0 && x < (int)grid.size() && y >= 0 && y < (int)grid[0].size();
			};

			auto src = std::make_pair(x, y);

			std::vector<ip> retval;

			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (!i && !j)
						continue;

					ip p = {src.first + i, src.second + j};

					if (inbounds(p.first, p.second))
						retval.push_back(p);
				}
			}

			return retval;
		}

		// dx xor dy must b zero
		ip crawl(std::vector<std::vector<int>>& grid, int value, int x, int y, int dx, int dy, int len, bool turn){
			//std::cout << "crawl at with len " << x << ", " << y << ", " << len << std::endl;

			if(!len || !((bool)dx ^ (bool)dy)) return std::make_pair(x, y);

			// assume current tile is free
			grid[x][y] = value;

			if(inbounds(grid, x + dx, y + dy) && !grid[x + dx][y + dy]){
				x += dx;
				y += dy;
				return crawl(grid, value, x, y, dx, dy, --len, turn);
			}
			
			if(!turn) return std::make_pair(x, y);

			// perpendicular
			int temp = dx;
			dx = (bool)dy;
			dy = (bool)temp;

			// can go forward/backward
			bool canf = inbounds(grid, x + dx, y + dy) && !grid[x + dx][y + dy];
			bool canb = inbounds(grid, x - dx, y - dy) && !grid[x - dx][y - dy];

			// neither, return
			if(!canf && !canb) return std::make_pair(x, y);

			// if both, pick one
			if(canf && canb){
				canf = rand()%2;
			}

			// go in chosen/possible direction
			dx = canf? dx : -dx;
			dy = canf? dy : -dy;
			x += dx;
			y += dy;
			return crawl(grid, value, x, y, dx, dy, --len, turn);
		}

		template<class T>
		void printgrid(std::vector<std::vector<T>>& grid){
			if(!grid.size() || !grid[0].size()){
				std::cout << "Bad grid" << std::endl;
				return;
			}

			for(size_t j = 0; j < grid[0].size(); j++){
				for(size_t i = 0; i < grid.size(); i++){
					std::cout.width(3);
					std::cout << grid[i][j] << " ";
				}
				std::cout << std::endl;
			}
		}

		void dilate(std::vector<std::vector<int>>& grid, int times, int value, int newval){
			//std::cout << "Dilate " << value << " -> " << newval << std::endl;
			//printgrid(grid);
			//std::cout << "After" << std::endl;

			auto newgrid = grid;

			for(int t = 0; t < times; t++){
				for(int i = 0; i < (int)grid.size(); i++){
					for(int j = 0; j < (int)grid[0].size(); j++){
						if(grid[i][j]) continue;

						for(auto n : neighbors(grid, i, j)){
							if(grid[n.first][n.second] == value){
								newgrid[i][j] = newval;
								break;
							}
						}
					}
				}
				grid = newgrid;
				//std::cout << "After" << std::endl;
				//printgrid(grid);
			}
		}

		// value must be positive
		void worm(std::vector<std::vector<int>>& grid, int value, int x, int y, int segs, int seglen, int dils, int reserve){
			if(value <= 0) return;

			for(int i = 0; i < segs; i++){
				int dir = rand()%4;
				int dx = (dir == 0) - (dir == 1);
				int dy = (dir == 2) - (dir == 3);
				auto p = crawl(grid, value, x, y, dx, dy, seglen, 1);
				x = p.first;
				y = p.second;
			}

			dilate(grid, dils, value, value);

			int resval = -1 * value;

			dilate(grid, reserve, value, resval);
			dilate(grid, reserve - 1, resval, resval);
		}

		std::vector<ip> get_free_tiles(std::vector<std::vector<int>>& grid){
			std::vector<ip> retval;

			for(int i = 0; i < (int)grid.size(); i++){
				for(int j = 0; j < (int)grid[0].size(); j++){
					if(!grid[i][j])
						retval.push_back(std::make_pair(i, j));
				}
			}
			return retval;
		}
		
		void compute_rectcover(){
			int x_tiles = tiles_x;
			int y_tiles = tiles_y;

			//std::cout << "Enter crc" << std::endl;

			// 1 uncovered 0 covered -1 obstruction
			std::vector<std::vector<int>> uncovered(x_tiles, std::vector<int>(y_tiles));
			int n_uncovered = 0;
			std::vector<rect> cover;
			std::vector<std::vector<int>> to_cover(x_tiles, std::vector<int>(y_tiles, -1));

			//std::cout << x_tiles << std::endl;
			//std::cout << "to_cover xs: " << to_cover.size() << std::endl;
			//std::cout << "to_cover ys: " << to_cover[0].size() << std::endl;

			int rectind = -1;

			// initialize uncovered to all
			for(int i = 0; i < x_tiles; i++){
				for(int j = 0; j < y_tiles; j++){
					if(!obgrid[i][j]){
						uncovered[i][j] = 1;
						n_uncovered++;
					}
					else{
						uncovered[i][j] = -1;
					}
				}
			}

			//std::cout << "Uncovered: " << std::endl;
			//printgrid(uncovered);
			//std::cout << "n_uncovered: " << n_uncovered << std::endl;

			// each rectangle
			while(n_uncovered > 0){
				rectind++;
				std::vector<ip> curr;
				rect r;

				//std::cout << "n_uncovered: " << n_uncovered << std::endl;
				//std::cout << "Rect " << rectind;

				// search for uncovered
				for(int i = 0; i < x_tiles; i++){
					for(int j = 0; j < y_tiles; j++){
						if(uncovered[i][j] == 1){
							// found seed of rectangle
							to_cover[i][j] = rectind;
							uncovered[i][j] = 0;
							n_uncovered--;
							r = rect(i, j, i+1, j+1);
							goto break2;	
						}
					}
				}

				break2:

				bool changed = true;
				while(changed){
					changed = false;

					//std::cout << "R" << rectind << ": " << r << std::endl;

					// try to expand left
					std::vector<ip> temp;
					bool allin = true;
					for(int i = r.yl; i < r.yh; i++){
						int k = r.xl - 1;
						if(inbounds(obgrid, k, i) && uncovered[k][i] == 1){
							temp.push_back(std::make_pair(k, i));
						}
						else{
							allin = false;
						}
					}
					if(allin){
						//std::cout << "\tl" << std::endl;
						r.xl--;
						for(auto& p : temp){
							uncovered[p.first][p.second] = 0;
							n_uncovered--;
							to_cover[p.first][p.second] = rectind;
						}
					}
					changed |= allin;

					// try to expand right
					temp = {};
					allin = true;
					for(int i = r.yl; i < r.yh; i++){
						int k = r.xh;
						if(inbounds(obgrid, k, i) && uncovered[k][i] == 1){
							temp.push_back(std::make_pair(k, i));
						}
						else{
							allin = false;
						}
					}
					if(allin){
						//std::cout << "\tr" << std::endl;
						r.xh++;
						for(auto& p : temp){
							uncovered[p.first][p.second] = 0;
							n_uncovered--;
							to_cover[p.first][p.second] = rectind;
						}
					}
					changed |= allin;

					// try to expand up
					temp = {};
					allin = true;
					for(int i = r.xl; i < r.xh; i++){
						int k = r.yh;
						if(inbounds(obgrid, i, k) && uncovered[i][k] == 1){
							temp.push_back(std::make_pair(i, k));
						}
						else{
							allin = false;
						}
					}
					if(allin){
						//std::cout << "\tu" << std::endl;
						r.yh++;
						for(auto& p : temp){
							uncovered[p.first][p.second] = 0;
							n_uncovered--;
							to_cover[p.first][p.second] = rectind;
						}
					}
					changed |= allin;

					// try to expand down
					temp = {};
					allin = true;
					for(int i = r.xl; i < r.xh; i++){
						int k = r.yl - 1;
						if(inbounds(obgrid, i, k) && uncovered[i][k] == 1){
							temp.push_back(std::make_pair(i, k));
						}
						else{
							allin = false;
						}
					}
					if(allin){
						//std::cout << "\td" << std::endl;
						r.yl--;
						for(auto& p : temp){
							uncovered[p.first][p.second] = 0;
							n_uncovered--;
							to_cover[p.first][p.second] = rectind;
						}
					}
					changed |= allin;

					//std::cout << "n_uncovered after iteration: " << n_uncovered << std::endl;


					//printgrid(uncovered);
					//std::cout << "Press Enter to Continue";
					//std::cin.ignore();
				}

				cover.push_back(r);
			}

			std::cout << "Rectangle cover: " << std::endl;
			printgrid(to_cover);
		}

		void generate_worms(int x_tiles, int y_tiles, int tile_width, int tile_height, int nshapes, int basepad, int wormspacing, int minarea, int maxarea, int minwormpad, int maxwormpad){
			srand(time(NULL));

			set_size(x_tiles, y_tiles, tile_width, tile_height);

			std::cout << "obgrid dims " << obgrid.size() << ", " << obgrid[0].size() << std::endl;

			std::vector<std::vector<int>> grid(x_tiles/2, std::vector<int>(y_tiles, 0));

			// worm to middle
			//int pathsegs = rand() % 3 + 1;
			int pathsegs = 2;
			int x = 0;
			int y = rand() % grid[0].size();
			int pathlen = grid.size() / pathsegs;
			std::cout << "pathlen " << pathlen << std::endl;

			for(int i = 0; i < pathsegs - 1; i++){
				auto p = crawl(grid, -1, x, y, 1, 0, pathlen, 0);
				x = p.first;
				y = p.second;
				std::cout << "at1 " << x << ", " << y << std::endl; 
				p = crawl(grid, -1, x, y, 0, (rand()%2)*2-1, pathlen, 0);
				x = p.first;
				y = p.second;
				std::cout << "at2 " << x << ", " << y << std::endl;
			}
			crawl(grid, -1, x, y, 1, 0, pathlen, 0);

			// mirror
			for(size_t i = 0; i < grid.size(); i++){
				for(size_t j = 0; j < grid[0].size(); j++){
					if(grid[i][j]){
						grid[i][grid[0].size() - 1 - j] = grid[i][j];
					}
				}
			}

			// dilate paths
			dilate(grid, 2, -1, -1);

			// pad space for base
			for(int i = 0; i < basepad; i++){
				grid[i] = std::vector<int>(y_tiles, -1);
			}

			// worms of various sizes
			for(int i = 0; i < nshapes; i++){
				auto ftiles = get_free_tiles(grid);
				if(!ftiles.size()) break;

				auto tile = ftiles[rand()%ftiles.size()];

				int area = rand()%(maxarea - minarea + 1) + minarea;
				int wp = rand()%(maxwormpad-minwormpad + 1) + minwormpad;
				int l = area/(1 + 2*wp);
				
				worm(grid, i + 2, tile.first, tile.second, 4, l, wp, wormspacing);
			}

			// mirror/flip to entire map
			for(size_t i = 0; i < grid.size(); i++){
				for(size_t j = 0; j < grid[0].size(); j++){
					int v = grid[i][j] > 0;
					obgrid[i][j] = v;
					obgrid[x_tiles - 1 - i][y_tiles - 1 - j] = v;
				}
			}

			// obgrid to obstructions
			obstructions = {};
			for(size_t i = 0; i < obgrid.size(); i++){
				for(size_t j = 0; j < obgrid[0].size(); j++){
					if(obgrid[i][j]){
						obstructions.push_back(std::make_pair(i, j));
					}
				}
			}

			compute_rectcover();
		}
};
