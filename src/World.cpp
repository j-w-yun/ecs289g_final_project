#pragma once

// #include <algorithm>
#include <vector>

#include "Vector2f.h"
#include "Vector2f.cpp"
#include "Map.h"
#include "Map.cpp"

World::World() {

}

void World::add(std::shared_ptr<GameObject> o) {
	objects.push_back(o);
}

void World::add(std::shared_ptr<MapLevel> o) {
	levels.push_back(o);
}

GameObject& World::get_object(int i) {
	return *(objects.at(i));
}

void World::remove_object(int i) {
	objects.erase(objects.begin() + i);
}

void World::clear_objects() {
	objects.clear();
}

MapLevel& World::get_level(int i) {
	return *(levels.at(i));
}

void World::remove_level(int i) {
	levels.erase(levels.begin() + i);
}

void World::clear_levels() {
	levels.clear();
}

std::vector<std::shared_ptr<GameObject>> World::get_objects() {
	return objects;
}

std::vector<std::shared_ptr<MapLevel>> World::get_levels() {
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
