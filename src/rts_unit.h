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
		int padding = 4;

		if(selected == 1){
			SDL_Rect box = {(int)(p().x() - r()) - padding, (int)(p().y() - r()) - padding, (int)(2 * r()) + 2*padding, (int)(2 * r()) + 2*padding};
			SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 255);
			SDL_RenderFillRect(renderer, &box);
		}

		//SDL_RenderDrawPoint(renderer, (int)p().x(), (int)p().y());
		SDL_Rect box = {(int)(p().x() - r()), (int)(p().y() - r()), (int)(2 * r()), (int)(2 * r())};
		SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 255);
		SDL_RenderFillRect(renderer, &box);
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
			if(inbounds(x, y) && map.get_obgrid()[x][y]){
				d = p() - to_world_space({x, y});
				//auto l = d.len();
				retval += 40.0f*d.unit()/(d.len2());
			}
		}

		return retval;
	}

	virtual void update(float elapsed_time, bool calc){
		set_p(p() + v());

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
		
		if(Input::is_mouse_pressed(SDL_BUTTON_RIGHT)){
			auto temp = Input::get_mouse_pos();
			dest.set(temp.first, temp.second);
		}

		if(calc){
			update_path();
		}

		auto deliberate = traverse_path()*acc;
		auto avoidance = avoid_obstacles();
		//Vector2f avoidance(0, 0);
		auto dv = deliberate+avoidance;

		auto nv = v() + dv;
		if(nv.len() > topspeed){
			nv = (nv/nv.len())*topspeed;
		}

		set_v(nv);
	}
};