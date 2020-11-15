// #include "Vector2f.h"
#include "Vector2f.cpp"

GameObject::GameObject(Vector2f* p, Vector2f* v, float r) {
	_position = p;
	_velocity = v;
	_radius = r;
}

Vector2f* GameObject::position() {
	return _position;
}

Vector2f* GameObject::velocity() {
	return _velocity;
}

float GameObject::radius() {
	return _radius;
}

void GameObject::set_position(Vector2f* p) {
	_position = p;
}

void GameObject::set_velocity(Vector2f* v) {
	_velocity = v;
}

void GameObject::set_radius(float r) {
	_radius = r;
}
