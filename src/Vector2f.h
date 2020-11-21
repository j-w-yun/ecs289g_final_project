#pragma once

#include<iostream>

class Vector2f {
	private:
		float _x;
		float _y;

	public:
		Vector2f() = default;
		Vector2f(float x, float y);
		void set(float x, float y);
		void setx(float x);
		void sety(float y);
		void set(Vector2f v);
		float x() const;
		float y() const;
		Vector2f add(const Vector2f& v) const;
		Vector2f sub(const Vector2f& v) const;
		Vector2f mul(const Vector2f& v) const;
		Vector2f div(const Vector2f& v) const;
		Vector2f add(const float x, const float y) const;
		Vector2f sub(const float x, const float y) const;
		Vector2f mul(const float x, const float y) const;
		Vector2f div(const float x, const float y) const;
		Vector2f scale(float v) const;
		float dot(const Vector2f& v) const;
		float len2() const;
		float len() const;
		
		Vector2f operator+(Vector2f r) const {
			return add(r);
		}

		Vector2f operator+=(Vector2f r) {
			auto temp = add(r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f operator-(Vector2f r) const {
			return sub(r);
		}

		Vector2f operator-=(Vector2f r) {
			auto temp = sub(r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f operator*(float r) const {
			return scale(r);
		}

		//Vector2f operator*(const Vector2f& r) const {
		//	return mul(r);
		//}

		Vector2f operator*=(float r) {
			auto temp = scale(r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f operator/(float r) const {
			return scale(1/r);
		}

		Vector2f operator/=(float r) {
			auto temp = scale(1/r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f unit() const {
			auto temp = len();
			if(temp > .000001)
				return (*this)/temp;
			return *this;
		}

};

Vector2f operator*(const float& l, const Vector2f& r) {
	return r*l;
}

float operator*(const Vector2f& l, const Vector2f& r) {
	return l.dot(r);
}

std::ostream& operator<<(std::ostream& os, Vector2f v) {
	os << "Vector2f(" << v.x() << ", " << v.y() << ")";
	return os;
}

Vector2f par_unit(const Vector2f& v){
	return Vector2f(v.y(), -v.x()).unit();
}

// components of r perpendicular and parallel to l
Vector2f components(const Vector2f& l, const Vector2f& r){
	float par = l.unit() * r;
	//float par = l.unit().mul((const Vector2f&)r);
	float perp = (r - par*l.unit()).len();
	return Vector2f(perp, par);
}