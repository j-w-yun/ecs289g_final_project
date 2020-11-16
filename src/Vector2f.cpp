#pragma once

#include <math.h>

Vector2f::Vector2f(float x, float y) {
	_x = x;
	_y = y;
}

void Vector2f::set(float x, float y) {
	_x = x;
	_y = y;
}

float Vector2f::x() {
	return _x;
}

float Vector2f::y() {
	return _y;
}

Vector2f* Vector2f::add(Vector2f* v) {
	return new Vector2f(_x + v->x(), _y + v->y());
}

Vector2f* Vector2f::mul(float v) {
	return new Vector2f(_x * v, _y * v);
}

float Vector2f::dot(Vector2f* v) {
	return _x * v->x() + _y * v->y();
}

float Vector2f::len2() {
	return dot(this);
}

float Vector2f::len() {
	return sqrt(len2());
}
