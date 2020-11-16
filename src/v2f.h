#pragma once

#include <math.h>

struct v2f {
	float x, y;

	v2f() = default;
	v2f(float x, float y): x(x), y(y) {}

	float n() {
		return sqrt(x*x + y*y);
	}

	float n2() {
		return x*x + y*y;
	}
};