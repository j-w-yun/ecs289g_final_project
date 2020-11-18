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
	ip dest = {0, 0};
	std::vector<ip> path;
	Vector2f goal;
	int selected = 0;
	MapLevel& map;

	rts_unit(Vector2f p, Vector2f v, float r, int w, int h, int xt, int yt, float a, float ts, MapLevel& mp): GameObject(p, v, r, w, h, xt, yt), acc(a), topspeed(ts), map(mp) {}

	virtual void render(SDL_Renderer* renderer){
		int padding = 4;

		if(selected == 1){
			SDL_SetRenderDrawColor(renderer, 0, 0xFF, 0, 255);
			SDL_Rect dragbox = {(int)(p().x() - r()) - padding, (int)(p().y() - r()) - padding, (int)(2 * r()) + 2*padding, (int)(2 * r()) + 2*padding};
			SDL_RenderFillRect(renderer, &dragbox);
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 255);
		//SDL_RenderDrawPoint(renderer, (int)p().x(), (int)p().y());
		SDL_Rect dragbox = {(int)(p().x() - r()), (int)(p().y() - r()), (int)(2 * r()), (int)(2 * r())};
		//SDL_Rect dragbox = {50, 50, 60, 60};
		SDL_RenderFillRect(renderer, &dragbox);
	}

	virtual void update_path(){
		auto tpath = find_path(map, to_tile_space(p()), dest);
		if(tpath.size())
			tpath.pop_back();

		path = {};

		for(auto& vi : tpath){
			path.push_back(std::make_pair(vi.x, vi.y));
		}
	}

	// returns unit
	virtual Vector2f traverse_path(){
		if(!path.size()){
			set_v(v() * .95);
			return Vector2f(0, 0);
		}

		auto wpoint = to_world_space(path.back());

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
			wpoint = to_world_space(path.back());
			d = wpoint - p();
		}

		return d.unit();
	}

	std::vector<ip> neighbors() {
		auto inbounds = [&](int x, int y) {
			return x >= 0 && x < x_tiles && y >= 0 && y < y_tiles;
		};

		auto src = to_tile_space(p());

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

	// returns unit
	virtual Vector2f avoid_obstacles(){
		std::vector<Vector2f> local_obs;

		for(auto n : neighbors()){
			if(map.get_obgrid()[n.first][n.second]){
				local_obs.push_back(to_world_space(n));
			}
		}

		Vector2f retval(0, 0);

		for(auto ob : local_obs){
			auto d = p() - ob;
			auto l = d.len();
			retval += 20.0f*d.unit()/(l*l);
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
			goal.set(temp.first, temp.second);
			dest = to_tile_space(goal);
		}

		if(calc){
			update_path();
		}

		auto deliberate = traverse_path()*acc;
		auto avoidance = avoid_obstacles();
		auto dv = deliberate+avoidance;

		auto nv = v() + dv;
		if(nv.len() > topspeed){
			nv = (nv/nv.len())*topspeed;
		}

		set_v(nv);
	}
};