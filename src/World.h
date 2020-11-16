#pragma once

#include <vector>
#include <memory>

#include "Vector2f.h"
#include "GameObject.h"
#include "Map.h"


class World {
	private:
		std::vector<std::shared_ptr<MapLevel>> levels;
		std::vector<std::shared_ptr<GameObject>> objects;

	public:
		World();
		void add(std::shared_ptr<GameObject> o);
		void add(std::shared_ptr<MapLevel> o);
		GameObject& get_object(int i);
		void remove_object(int i);
		void clear_objects();
		MapLevel& get_level(int i);
		void remove_level(int i);
		void clear_levels();
		std::vector<std::shared_ptr<GameObject>> get_objects();
		std::vector<std::shared_ptr<MapLevel>> get_levels();
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
};
