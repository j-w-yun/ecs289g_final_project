#pragma once

#include <vector>
#include <set>
#include <functional>
#include <memory>
#include <time.h>
#include <stdlib.h>
#include <limits>

#include "GameObject.h"
#include "GameObject.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "AStar.hpp"
#include "algorithms.h"
#include "projectile.h"
#include "manager.h"

struct rect{
	int xl, yl, xh, yh;

	rect() = default;
	rect(int xl, int yl, int xh, int yh): xl(xl), yl(yl), xh(xh), yh(yh) {}
	rect(Vector2f v){
		xl = xh = (int)v.x();
		yl = yh = (int)v.y();
	}

	rect center() const{
		int x = xl + (xh - xl)/2;
		int y = yl + (yh - yl)/2;
		return rect(x, y, x, y);
	}
};

std::ostream& operator<<(std::ostream& os, rect r){
	os << "r(" << r.xl << ", " << r.yl << ", " << r.xh << ", " << r.yh << ")";
	return os;
}

typedef std::pair<int, int> ip;

class MapLevel: public GameObject {
	private:
		unsigned long seed = time(NULL);
		const int N_SAND = 8;
		const int N_WATER = 8;
		float texture_resolution = 5.0f;
		std::vector<std::vector<double>> noise2d;
		std::vector<std::vector<std::vector<Vector2f>>> obstruction_vertices;
		SDL_Surface* loaded_surface;
		SDL_Surface* formatted_surface;
		SDL_Texture* texture;

		int min_octave;
		int max_octave;
		int tiles_x;
		int tiles_y;
		float tile_width;
		float tile_height;
		std::vector<ip> obstructions;
		std::vector<std::vector<bool>> obgrid;
		std::vector<std::shared_ptr<GameObject>> units;
		std::vector<size_t> idstack;
		std::vector<size_t> idbuffer;
		std::vector<std::vector<std::vector<std::vector<size_t>>>> unitgrid;
		size_t unitcap;
		std::vector<rect> rectcover;
		std::vector<rect> pathcover;
		std::vector<std::vector<int>> grid_to_rectcover;
		std::vector<std::vector<size_t>> rectgraph;
		bool climb(std::vector<std::pair<int, int>>* obs, double noise[], float threshold, std::vector<std::pair<int, int>> bases, int padding);
		std::vector<std::shared_ptr<projectile>> projectiles;
		std::vector<size_t> pstack;
		size_t projcap;
		std::vector<manager> managers;

		int teams = 2;

		static int group_size;
		static char group_size_keydown;

		bool lbutton_down = false;
		bool rbutton_down = false;

		int dissolve_key = SDLK_d;
		bool dissolve_down = false;

	public:
		MapLevel() = default;
		MapLevel(int tx, int ty, float tw, float th, size_t uc = 200);
		void init_managers();
		bool add(std::shared_ptr<GameObject> o);
		bool add_proj(std::shared_ptr<projectile> o);
		void kill(int id);
		void kill_proj(int id);
		void set_size(int x, int y, int w, int h);
		void set_obstructions(std::vector<std::pair<int, int>> o);
		void set(int x, int y, int w, int h, std::vector<std::pair<int, int>> o);
		std::vector<std::pair<int, int>> random_obstructions(std::vector<std::pair<int, int>> bases, int min, int max, int padding);
		std::vector<std::pair<int, int>> generate_obstructions(std::vector<std::pair<int, int>> bases, int padding);
		int get_width() {return tiles_x;};
		int get_height() {return tiles_y;};
		int get_tile_width() {return tile_width;};
		int get_tile_height() {return tile_height;};
		const std::vector<ip>& get_obstructions() {return obstructions;};
		const std::vector<std::vector<bool>>& get_obgrid() {return obgrid;};
		const std::vector<std::vector<std::vector<std::vector<size_t>>>>& get_unitgrid() {return unitgrid;};
		const std::vector<std::shared_ptr<GameObject>>& get_units() {return units;}
		std::vector<std::shared_ptr<GameObject>>& get_objects();
		size_t get_unitcap() {return unitcap;}
		int get_teams() {return teams;}
		void generate_texture();
		void render_texture(SDL_Renderer* renderer);
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
		static std::string static_class() {return "MapLevel";};
		std::vector<manager>& get_managers() { return managers;}
		static int get_group_size() { return group_size; }

		void compute_obstruction_vertices();
		std::vector<Vector2f> find_rect_path(Vector2f s, Vector2f d);
		std::vector<Vector2f> reconstruct_path(std::vector<int>& from, std::vector<Vector2f>& points, int src, int dest, Vector2f v2fdest);
		std::vector<Vector2f> reconstruct_better_path(std::vector<int>& from, std::vector<Vector2f>& points, int src, int dest, Vector2f v2fsrc, Vector2f v2fdest);

		std::pair<int, int> to_tile_space(std::pair<float, float> p){
			int x = (int)p.first;
			int y = (int)p.second;

			return std::make_pair(x/(int)tile_width, y/(int)tile_height);
		}

		std::pair<int, int> to_tile_space(Vector2f p){
			return to_tile_space(std::make_pair(p.x(), p.y()));
		}

		Vector2f to_world_space(std::pair<int, int> p){
			float x = (float)p.first;
			float y = (float)p.second;

			return Vector2f(tile_width*x + (float)tile_width/2, tile_height*y + (float)tile_height/2);
		}

		template<class T>
		bool inbounds(const std::vector<std::vector<T>>& grid, int x, int y){
			return x >= 0 && x < (int)grid.size() && y >= 0 && y < (int)grid[0].size();
		}

		template<class T>
		bool inbounds(const std::vector<std::vector<T>>& grid, ip pr){
			return pr.first >= 0 && pr.first < (int)grid.size() && pr.second >= 0 && pr.second < (int)grid[0].size();
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

		// orient 0 x 1 y 2 both
		// considers touching to be intersection
		bool intersects(const rect& l, const rect& r, int orient = 2){
			bool ix = l.xh >= r.xl && l.xl <= r.xh;
			bool iy = l.yh >= r.yl && l.yl <= r.yh;

			switch(orient){
				case 0: return ix;
				case 1: return iy;
				case 2: return ix&&iy;
			}

			return 0;
		}

		rect intersection(const rect& l, const rect& r){
			if(!intersects(l, r)) return rect(0, 0, 0, 0);

			int xl = std::max(l.xl, r.xl);
			int yl = std::max(l.yl, r.yl);
			int xh = std::min(l.xh, r.xh);
			int yh = std::min(l.yh, r.yh);

			return rect(xl, yl, xh, yh);
		}

		// orient 0 x 1 y 2 both
		bool abuts(const rect& l, const rect& r){
			bool ax = (l.xh == r.xl || l.xl == r.xh) && intersects(l, r, 1);
			bool ay = (l.yh == r.yl || l.yl == r.yh) && intersects(l, r, 0);

			return ax || ay;
		}

		rect shrink(const rect& r, int amt){
			rect nr(r.xl + amt, r.yl + amt, r.xh - amt, r.yh - amt);
			if(nr.xl > nr.xh || nr.yl > nr.yh) nr = r.center();
			return nr;
		}

		void compute_rectcover(){
			rectcover = {};
			pathcover = {};

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
				pathcover.push_back(shrink(r, 1));

			}

			std::cout << "Rectangle cover: " << std::endl;
			//printgrid(to_cover);

			std::vector<rect> temp;
			double factor = 1.5;

			// divide rectangles
			for(int i = 0; i < 3; i++){
				temp = {};

				for(auto& r : cover){
					bool wide = (r.xh - r.xl) > factor*(r.yh - r.yl);
					bool tall = (r.yh - r.yl) > factor*(r.xh - r.xl);

					if(wide){
						int midx = (r.xh - r.xl)/2 + r.xl;
						temp.push_back(rect(r.xl, r.yl, midx, r.yh));
						temp.push_back(rect(midx, r.yl, r.xh, r.yh));
					}
					else if(tall){
						int midy = (r.yh - r.yl)/2 + r.yl;
						temp.push_back(rect(r.xl, r.yl, r.xh, midy));
						temp.push_back(rect(r.xl, midy, r.xh, r.yh));
					}
					else{
						temp.push_back(r);
					}
				}

				cover = temp;
			}


			// recompute to_cover
			to_cover = std::vector<std::vector<int>>(x_tiles, std::vector<int>(y_tiles, -1));
			for(int i = 0; i < x_tiles; i++){
				for(int j = 0; j < y_tiles; j++){
					if(obgrid[i][j]){
						to_cover[i][j] = -1;
					}
					else{
						for(int ri = 0; ri < (int)cover.size(); ri++){
							auto& r = cover[ri];
							
							if(i >= r.xl && i < r.xh && j >= r.yl && j < r.yh){
								to_cover[i][j] = ri;
								break;
							}
						}
					}
				}
			}


			rectcover = cover;
			grid_to_rectcover = to_cover;

			rectgraph = std::vector<std::vector<size_t>>(cover.size());

			for(int i = 0; i < (int)cover.size() - 1; i++){
				for(int j = i + 1; j < (int)cover.size(); j++){
					if(i == j) continue;

					if(abuts(cover[i], cover[j])){
						std::cout << i << " abuts " << j << std::endl;
						rectgraph[i].push_back(j);
						rectgraph[j].push_back(i);
					}
				}
			}

		}

		void generate_worms(int x_tiles, int y_tiles, int tile_width, int tile_height, int nshapes, int basepad, int wormspacing, int minarea, int maxarea, int minwormpad, int maxwormpad){
			srand(time(NULL));
			//srand(0); // reasonably interesting map

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

			// // show path for debugging
			// auto path = find_rect_path(Vector2f(10, 10), Vector2f(1178, 787));
			// std::cout << "Path: " << std::endl;
			// for(auto& v2f : path){
			// 	std::cout << v2f << std::endl;
			// }

			compute_obstruction_vertices();
		}

		float dist(Vector2f l, Vector2f r){
			return (l - r).len();
		}

		float dist(int x1, int y1, int x2, int y2){
			return dist(Vector2f((float)x1, (float)y1), Vector2f((float)x2, (float)y2));
		}

		float rdist(const rect& a, const rect& b){
			if(intersects(a, b)) return 0;

			bool r = b.xl > a.xh;
			bool u = b.yl > a.yh;
			bool l = b.xh < a.xl;
			bool d = b.yh < a.yl;

			if(r && u) return dist(a.xh, a.yh, b.xl, b.yl);
			if(l && u) return dist(a.xl, a.yh, b.xh, b.yl);
			if(l && d) return dist(a.xl, a.yl, b.xh, b.yh);
			if(r && d) return dist(a.xh, a.yl, b.xl, b.yh);
			if(r) return b.xl - a.xh;
			if(u) return b.yl - a.yh;
			if(l) return a.xl - b.xh;
			if(d) return a.yl - b.yh;

			return 0;
		}

		// as opposed to tile_rect
		struct world_rect{
			float xl, yl, xh, yh;

			world_rect() = default;
			world_rect(float xl, float yl, float xh, float yh): xl(xl), yl(yl), xh(xh), yh(yh) {}
			//world_rect(const rect& r){
			//	auto lows = to_world_space(std::make_pair(r.xl, r.yl));
			//	auto highs = to_world_space(std::make_pair(r.xh, r.yh));

			//	xl = lows.x();
			//	yl = lows.y();
			//	xh = highs.x();
			//	yh = highs.y();
			//}
			world_rect(Vector2f v){
				xl = xh = v.x();
				yl = yh = v.y();
			}

			world_rect center(){
				float x = xl + (xh - xl)/2;
				float y = yl + (yh - yl)/2;
				return world_rect(x, y, x, y);
			}
		};

		//rect get_edge(const rect& )

		world_rect to_world_rect(const rect& _a){
			auto lows = to_world_space(std::make_pair(_a.xl, _a.yl));
			auto highs = to_world_space(std::make_pair(_a.xh, _a.yh));

			return world_rect(lows.x(), lows.y(), highs.x(), highs.y());
		}

		// p is in world space, a is in tile space
		// transforms
		Vector2f closest_point(Vector2f p, const rect& _a){
			auto lows = to_world_space(std::make_pair(_a.xl, _a.yl));
			auto highs = to_world_space(std::make_pair(_a.xh, _a.yh));

			world_rect a(lows.x(), lows.y(), highs.x(), highs.y());



			world_rect b(p.x(), p.y(), p.x(), p.y());

			bool r = b.xl > a.xh;
			bool u = b.yl > a.yh;
			bool l = b.xh < a.xl;
			bool d = b.yh < a.yl;

			if(r && u) return Vector2f(a.xh, a.yh);
			if(l && u) return Vector2f(a.xl, a.yh);
			if(l && d) return Vector2f(a.xl, a.yl);
			if(r && d) return Vector2f(a.xh, a.yl);
			if(r) return Vector2f(a.xh, p.y());
			if(u) return Vector2f(p.x(), a.yh);
			if(l) return Vector2f(a.xl, p.y());
			if(d) return Vector2f(p.x(), a.yl);

			return p;

		}
};

std::ostream& operator<<(std::ostream& os, MapLevel::world_rect r){
	os << "r(" << r.xl << ", " << r.yl << ", " << r.xh << ", " << r.yh << ")";
	return os;
}