#pragma once

#include <vector>
#include <memory>

#include "Vector2f.h"
#include "GameObject.h"
#include "Map.h"


class World {
	private:
		int activemap;
		std::vector<std::shared_ptr<MapLevel>> levels;

	public:
		World();
		void add(std::shared_ptr<MapLevel> o);
		MapLevel& get_level(int i);
		void remove_level(int i);
		void clear_levels();
		std::vector<std::shared_ptr<MapLevel>> get_levels();
		void select(Vector2f drag_start, Vector2f drag_end, int drag_type);
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
};
