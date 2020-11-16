#pragma once

#include "Vector2f.h"
#include "Vector2f.cpp"

GameObject::GameObject(Vector2f* p, Vector2f* v, float r) {
	_p = p;
	_v = v;
	_r = r;
}

Vector2f* GameObject::p() {
	return _p;
}

Vector2f* GameObject::v() {
	return _v;
}

float GameObject::r() {
	return _r;
}

void GameObject::set_p(Vector2f* p) {
	_p = p;
}

void GameObject::set_v(Vector2f* v) {
	_v = v;
}

void GameObject::set_r(float r) {
	_r = r;
}

void GameObject::render(SDL_Renderer* renderer) {
	SDL_Rect box = {(int)(_p->x()-_r/2), (int)(_p->y()-_r/2), (int)_r, (int)_r};
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xAA, 0x77, 0xFF);
	SDL_RenderFillRect(renderer, &box);
}

void GameObject::update(float t) {
	_p = _p->add(_v->mul(t));
}
