#pragma once

#include <vector>

#include "Vector2f.h"
#include "GameObject.h"
#include "Map.h"

class World {
	private:
		std::vector<MapLevel*> levels;
		std::vector<GameObject*> objects;

	public:
		World();
		void add(GameObject* o);
		GameObject* get_object(int i);
		void remove_object(int i);
		void clear_objects();
		MapLevel* get_level(int i);
		void remove_level(int i);
		void clear_levels();
		std::vector<GameObject*> get_objects();
		std::vector<MapLevel*> get_levels();
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
};
