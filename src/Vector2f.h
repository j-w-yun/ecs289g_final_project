#pragma once

class Vector2f {
	private:
		float _x;
		float _y;

	public:
		Vector2f() = default;
		Vector2f(float x, float y);
		void set(float x, float y);
		float x() const;
		float y() const;
		Vector2f add(const Vector2f& v) const;
		Vector2f sub(const Vector2f& v) const;
		Vector2f mul(const Vector2f& v) const;
		Vector2f div(const Vector2f& v) const;
		Vector2f scale(float v) const;
		float dot(const Vector2f& v) const;
		float len2() const;
		float len() const;
};
