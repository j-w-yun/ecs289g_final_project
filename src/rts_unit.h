#pragma once

#include "v2f.h"
#include "AStar.hpp"
#include "AStar.cpp"
#include "algorithms.h"
#include<vector>
#include<utility>

typedef std::pair<int, int> ip;

struct rts_unit : GameObject {
	rts_unit() = default;

	float acc;
	float topspeed;
	Vector2f dest = Vector2f(0, 0);
	std::vector<Vector2f> path;
	int selected = 0;
	MapLevel& map;
	int layers = 1;
	int side = layers*2 + 1;
	int size = side * side - 1;

	rts_unit(Vector2f p, Vector2f v, float r, int w, int h, int xt, int yt, float a, float ts, MapLevel& mp): GameObject(p, v, r, w, h, xt, yt), acc(a), topspeed(ts), map(mp) {}

	virtual void render(SDL_Renderer* renderer){
		// int padding = 4;
		if(selected == 1){
			// SDL_Rect box = {(int)(p().x() - r()) - padding, (int)(p().y() - r()) - padding, (int)(2 * r()) + 2*padding, (int)(2 * r()) + 2*padding};
			Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f((int)(p().x() - r()), (int)(p().y() - r())));
			Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((int)(p().x() + r()), (int)(p().y() + r())));
			SDL_Rect box = {
				(int)(sp1.x()),
				(int)(sp1.y()),
				(int)(sp2.x()-sp1.x()),
				(int)(sp2.y()-sp1.y())
			};
			// Fill
			SDL_SetRenderDrawColor(renderer, 91, 192, 222, 255);
			SDL_RenderFillRect(renderer, &box);
			// Outline
			SDL_SetRenderDrawColor(renderer, 2, 117, 216, 255);
			SDL_RenderDrawRect(renderer, &box);

			if(path.size()){
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				Vector2f p1 = RenderingEngine::world_to_screen(p());
				Vector2f p2 = RenderingEngine::world_to_screen(path.back());
				SDL_RenderDrawLine(renderer, p1.x(), p1.y(), p2.x(), p2.y());
				for(size_t i = path.size() - 1; i; i--){
					SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
					Vector2f p1 = RenderingEngine::world_to_screen(path[i]);
					Vector2f p2 = RenderingEngine::world_to_screen(path[i - 1]);
					SDL_RenderDrawLine(renderer, p1.x(), p1.y(), p2.x(), p2.y());
				}
			}
		}
	}

	virtual void update_path(){
		std::vector<Vector2f> find_rect_path(Vector2f s, Vector2f d);
		auto tpath = map.find_rect_path(p(), dest);

		path = tpath;
	}

	// returns unit
	virtual Vector2f traverse_path(){
		if(!path.size()){
			set_v(v() * .95);
			return Vector2f(0, 0);
		}

		auto wpoint = path.back();

		auto d = wpoint - p();
		auto dist = d.len();

		bool recomp = false;

		//if(dist > std::max((float)xtwidth/2, (float)xtwidth/2)){
		//	update_path();
		//	recomp = true;
		//}
		if(dist < (float)xtwidth/5){
			path.pop_back();
			recomp = true;
		}
		if(recomp){
			if(!path.size()){
				set_v(v() * .95);
				return Vector2f(0, 0);
			}

			wpoint = path.back();
			d = wpoint - p();
		}

		return d.unit();
	}

	inline bool inbounds(int x, int y) {
		return x >= 0 && x < x_tiles && y >= 0 && y < y_tiles;
	}

	/*std::vector<ip> neighbors() {
		auto src = to_tile_space(p());

		short ctr = 0;
		for (int i = -layers; i <= layers; i++) {
			for (int j = -layers; j <= layers; j++) {
				if (!i && !j)
					continue;

				ip p = {src.first + i, src.second + j};

				if (inbounds(p.first, p.second))
					retval.push_back(p);

				ctr++;
			}
		}

		return retval;
	}*/

	// returns unit
	virtual Vector2f avoid_obstacles(){
		Vector2f retval(0, 0);
		Vector2f d;
		//Vector2f l;
		int x, y;

		auto pr = to_tile_space(p());

		for(int i = 0; i < size; i++){
			x = pr.first + i/side - layers;
			y = pr.second + i%side - layers;

			if(inbounds(x, y)){
				// wall
				if(map.get_obgrid()[x][y]){
					d = p() - to_world_space({x, y});
					//auto l = d.len();
					retval += 40.0f*d.unit()/(d.len2());
				}
				// units
				else{
					for(auto uind : map.get_unitgrid()[x][y]){
						if(uind == id) continue;

						d = p() - map.get_units()[uind]->p();
						//auto l = d.len();
						retval += 20.0f*d.unit()/(std::max(d.len2(), .001f));
					}
				}
			}
		}

		return retval;
	}

	virtual Vector2f avoid_units(){
		Vector2f retval(0, 0);
		Vector2f d;
		//Vector2f l;
		int x, y;

		Vector2f vu = v().unit();
		Vector2f pu = par_unit(v());

		auto pr = to_tile_space(p());

		for(int i = 0; i < size; i++){
			x = pr.first + i/side - layers;
			y = pr.second + i%side - layers;
			if(inbounds(x, y) && !map.get_obgrid()[x][y]){
				for(auto uind : map.get_unitgrid()[x][y]){
					//std::cout << "unid is  " << uind << std::endl;

					// don't avoid self
					if(uind == id) continue;

					auto& unit = *(map.get_units()[uind]);

					d = unit.p() - p();

					d = components(v(), d);

					// only influenced by units in front
					//if(d.y() < 0) continue;

					//float slow = -d.y() / d.x();
					//float turn = d.x()/d.y();

					int sign = (d.x() > 0) * 2 - 1;

					auto fun = v().len() * exp(-(d.y() * d.x()));
					float slow = fun/std::max(d.y(), .001f);
					float turn = sign * fun/std::max(abs(d.x()), .001f);

					retval = retval + slow*vu + turn*pu;
					//std::cout << "done" << std::endl;
				}
			}
		}

		return retval;
	}

	virtual void update(float elapsed_time, bool calc){
		//std::cout << "unit pos before " << p() << std::endl;

		auto temp = p() + v();

		auto temp_tile = to_tile_space(temp);

		if(map.inbounds(map.get_obgrid(), temp_tile) && !map.get_obgrid()[temp_tile.first][temp_tile.second]){
			set_p(temp);
		}

		//set_p(p() + v());

		auto x = p().x(), y = p().y();

		float o = .0001;

		// bounce
		if(x - r() < 0){
			set_p(Vector2f(r() + o, p().y()));
			set_v(Vector2f(-v().x(), v().y()));
		}
		if(y - r() < 0){
			set_p(Vector2f(p().x(), r() + 0));
			set_v(Vector2f(v().x(), -v().y()));
		}
		if(x + r() > wwidth){
			set_p(Vector2f(wwidth - r() - o, p().y()));
			set_v(Vector2f(-v().x(), v().y()));
		}
		if(y + r() > wheight){
			set_p(Vector2f(p().x(), wheight - r() - o));
			set_v(Vector2f(v().x(), -v().y()));
		}
		
		//std::cout << "unit pos after " << p() << std::endl;

		if(Input::is_mouse_pressed(SDL_BUTTON_RIGHT)){
			auto temp = Input::get_mouse_pos();
			dest = RenderingEngine::screen_to_world(Vector2f(temp.first, temp.second));
		}

		if(calc){
			update_path();
		}

		auto deliberate = traverse_path()*acc;
		auto avoidance = avoid_obstacles();
		//auto group = avoid_units();
		//Vector2f group(0, 0);
		//std::cout << "deliberate is " << deliberate << std::endl;
		//std::cout << "avoidance is " << avoidance << std::endl;
		//std::cout << "group is " << group << std::endl;
		//Vector2f avoidance(0, 0);
		auto dv = deliberate+avoidance;

		//std::cout << "dv is " << dv << std::endl;
		//std::cout << "v is " << v() << std::endl;

		auto nv = v() + dv;
		if(nv.len() > topspeed){
			//std::cout << "In if" << std::endl;
			nv = (nv/nv.len())*topspeed;
		}

		//std::cout << "nv is " << nv << std::endl;

		set_v(nv);
	}
};