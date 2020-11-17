#pragma once

#include "v2f.h"
#include <vector>

struct rts_unit : GameObject {
	rts_unit() = default;

	rts_unit(Vector2f p, Vector2f v, float r): GameObject(p, v, r) {}

	virtual void render(SDL_Renderer* renderer){
		SDL_SetRenderDrawColor(renderer, 0, 0, 0xFF, 255);
		//SDL_RenderDrawPoint(renderer, (int)p().x(), (int)p().y());
		SDL_Rect dragbox = {(int)(p().x() - r()), (int)(p().y() - r()), (int)(2 * r()), (int)(2 * r())};
		//SDL_Rect dragbox = {50, 50, 60, 60};
		SDL_RenderFillRect(renderer, &dragbox);
	}

	virtual void update(float elapsed_time){

	}
};