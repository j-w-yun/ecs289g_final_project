#pragma once

class Vector2f {
	private:
		float _x;
		float _y;

	public:
		Vector2f(float x, float y);
		void set(float x, float y);
		float x();
		float y();
		Vector2f* add(Vector2f* v);
		Vector2f* sub(Vector2f* v);
		Vector2f* mul(Vector2f* v);
		Vector2f* div(Vector2f* v);
		Vector2f* scale(float v);
		float dot(Vector2f* v);
		float len2();
		float len();
};
