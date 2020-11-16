#pragma once

#include <functional>

#include "Vector2f.h"
#include "Vector2f.cpp"

GameObject::GameObject() {
	set_defaults();
}

GameObject::GameObject(Vector2f* p, Vector2f* v, float r) {
	set_defaults();
	position = p;
	velocity = v;
	radius = r;
}

void GameObject::set_defaults() {
	class_string = GameObject::static_class();
	_is_visible = true;
	_is_movable = true;
	position = new Vector2f(0, 0);
	velocity = new Vector2f(0, 0);
	radius = 0;
}

Vector2f* GameObject::p() {
	return position;
}

Vector2f* GameObject::v() {
	return velocity;
}

float GameObject::r() {
	return radius;
}

void GameObject::set_p(Vector2f* p) {
	position = p;
}

void GameObject::set_v(Vector2f* v) {
	velocity = v;
}

void GameObject::set_r(float r) {
	radius = r;
}

void GameObject::set_visible(bool b) {
	_is_visible = b;
}

void GameObject::set_movable(bool b) {
	_is_movable = b;
}

bool GameObject::is_visible() {
	return _is_visible;
}

bool GameObject::is_movable() {
	return _is_movable;
}

bool GameObject::overlaps(GameObject* other) {
	float len = radius + other->r();
	float len2 = len * len;
	float dist2 = position->sub(other->p())->len2();
	return dist2 < len2;
}

void GameObject::set_render_callback(std::function<void(SDL_Renderer*)> callback) {
	_render_callback = callback;
}

void GameObject::set_update_callback(std::function<void(float)> callback) {
	_update_callback = callback;
}

void GameObject::render(SDL_Renderer* renderer) {
	if (!_is_visible)
		return;

	if (_render_callback)
		_render_callback(renderer);
}

void GameObject::update(float t) {
	if (!_is_movable)
		return;

	position = position->add(velocity->scale(t));

	if (_update_callback)
		_update_callback(t);
}
