#pragma once

// #include <algorithm>
#include <vector>

#include "Vector2f.h"
#include "Vector2f.cpp"
#include "Map.h"
#include "Map.cpp"

World::World() {

}

void World::add(GameObject* o) {
	if (o->is_a(MapLevel::static_class()))
		levels.push_back(static_cast<MapLevel*>(o));
	else
		objects.push_back(o);
}

GameObject* World::get_object(int i) {
	return objects.at(i);
}

void World::remove_object(int i) {
	objects.erase(objects.begin() + i);
}

void World::clear_objects() {
	objects.clear();
}

MapLevel* World::get_level(int i) {
	return levels.at(i);
}

void World::remove_level(int i) {
	levels.erase(levels.begin() + i);
}

void World::clear_levels() {
	levels.clear();
}

std::vector<GameObject*> World::get_objects() {
	return objects;
}

std::vector<MapLevel*> World::get_levels() {
	return levels;
}

void World::render(SDL_Renderer* renderer) {
	// Draw levels
	for (auto& level : levels)
		level->render(renderer);
	// Draw objects
	for (auto& object : objects)
		object->render(renderer);
}

void World::update(float elapsed_time) {
	// Update levels
	for (auto& level : levels)
		level->update(elapsed_time);
	// Update objects
	for (auto& object : objects)
		object->update(elapsed_time);
}
