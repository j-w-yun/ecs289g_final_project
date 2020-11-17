#pragma once

#include "v2f.h"
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

	rts_unit(Vector2f p, Vector2f v, float r, float a, float ts): GameObject(p, v, r), acc(a), topspeed(ts) {}

	virtual void render(SDL_Renderer* renderer){
		SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 255);
		//SDL_RenderDrawPoint(renderer, (int)p().x(), (int)p().y());
		SDL_Rect dragbox = {(int)(p().x() - r()), (int)(p().y() - r()), (int)(2 * r()), (int)(2 * r())};
		//SDL_Rect dragbox = {50, 50, 60, 60};
		SDL_RenderFillRect(renderer, &dragbox);
	}

	virtual void update(float elapsed_time){
		set_p(p() + v());

		if(Input::is_mouse_pressed(SDL_BUTTON_RIGHT)){
			auto temp = Input::get_mouse_pos();
			goal.set(temp.first, temp.second);
		}

		auto d = goal - p();


		if(d.len2() > 100){
			auto dv = (d/d.len())*acc;
			auto nv = v() + dv;
			if(nv.len() > topspeed){
				nv = (nv/nv.len())*topspeed;
			}

			set_v(nv);
		}
		else{
			set_v(v() * .95);
		}
	}
};