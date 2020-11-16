#pragma once

#include "Vector2f.h"

class GameObject {
	private:
		Vector2f* _p;
		Vector2f* _v;
		float _r;
		bool is_visible;
		bool is_movable;

	public:
		GameObject(Vector2f* p, Vector2f* v, float r);
		Vector2f* p();
		Vector2f* v();
		float r();
		void set_p(Vector2f* p);
		void set_v(Vector2f* v);
		void set_r(float r);
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
};
