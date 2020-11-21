#pragma once

// #include <algorithm>
#include <vector>

#include "Vector2f.h"
#include "Vector2f.cpp"
#include "Map.h"
#include "Map.cpp"

World::World() {
	activemap = -1;
}

void World::add(std::shared_ptr<MapLevel> o) {
	// TODO add way to make other maps active
	activemap = 0;
	levels.push_back(o);
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

std::vector<std::shared_ptr<MapLevel>> World::get_levels() {
	return levels;
}

void World::select(Vector2f drag_start, Vector2f drag_end, int drag_type) {
	std::cout << "World::select(): implement me!" << std::endl;
	std::cout << "drag_start: " << drag_start << std::endl;
	std::cout << "drag_end: " << drag_end << std::endl;
	std::cout << "drag_type: " << drag_type << std::endl;
	std::cout << std::endl;
}

void World::click(Vector2f position, int click_type) {
	std::cout << "World::click(): implement me!" << std::endl;
	std::cout << "drag_startposition: " << position << std::endl;
	std::cout << "drag_endclick_type: " << click_type << std::endl;
	std::cout << std::endl;
}

void World::render(SDL_Renderer* renderer) {
	// Draw active level
	if(activemap >= 0 && (size_t)activemap < levels.size())
		levels[activemap]->render(renderer);

	// Draw levels
	//for (auto& level : levels)
	//	level->render(renderer);
	// Draw objects
	//for (auto& object : objects)
	//	object->render(renderer);
}

void World::update(float elapsed_time) {
	// Update active level
	if(activemap >= 0 && (size_t)activemap < levels.size())
		levels[activemap]->update(elapsed_time);
	
	// Update levels
	//for (auto& level : levels)
	//	level->update(elapsed_time);
	// Update objects
	//for (auto& object : objects)
	//	object->update(elapsed_time);
}
