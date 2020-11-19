#pragma once

#include <math.h>

Vector2f::Vector2f(float x, float y) {
	_x = x;
	_y = y;
}

/*Vector2f::Vector2f(int x, int y) {
	_x = (float)x;
	_y = (float)y;
}*/

void Vector2f::set(float x, float y) {
	_x = x;
	_y = y;
}

float Vector2f::x() const {
	return _x;
}

float Vector2f::y() const {
	return _y;
}

Vector2f Vector2f::add(const Vector2f& v) const {
	return Vector2f(_x + v.x(), _y + v.y());
}

Vector2f Vector2f::sub(const Vector2f& v) const {
	return Vector2f(_x - v.x(), _y - v.y());
}

Vector2f Vector2f::mul(const Vector2f& v) const {
	return Vector2f(_x * v.x(), _y * v.y());
}

Vector2f Vector2f::div(const Vector2f& v) const {
	return Vector2f(_x / v.x(), _y / v.y());
}

Vector2f Vector2f::scale(float v) const {
	return Vector2f(_x * v, _y * v);
}

float Vector2f::dot(const Vector2f& v) const {
	return _x * v.x() + _y * v.y();
}

float Vector2f::len2() const {
	return dot(*this);
}

float Vector2f::len() const {
	return sqrt(len2());
}
