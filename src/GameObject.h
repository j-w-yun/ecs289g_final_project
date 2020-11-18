#pragma once

#include <functional>
#include <string>
#include <memory>

#include "Vector2f.h"

class GameObject {
	protected:
		std::string class_string;

	private:
		Vector2f position;
		Vector2f velocity;
		float radius;
		bool _is_visible;  // Skip render if false
		bool _is_movable;  // Skip update if false
		std::function<void(SDL_Renderer*)> _render_callback;
		std::function<void(float)> _update_callback;
		void set_defaults();

	public:
		int wwidth;
		int wheight;
		int x_tiles;
		int y_tiles;
		int xtwidth;
		int ytwidth;
		size_t id;
		GameObject();
		GameObject(const Vector2f& p, const Vector2f& v, float r, int w, int h, int xt, int yt);
		Vector2f p() const;
		Vector2f v() const;
		float r() const;
		void set_p(const Vector2f& p);
		void set_v(const Vector2f& v);
		void set_r(float r);
		void set_visible(bool b);
		void set_movable(bool b);
		bool is_visible();
		bool is_movable();
		bool overlaps(GameObject& other);
		void set_render_callback(std::function<void(SDL_Renderer*)> callback);
		void set_update_callback(std::function<void(float)> callback);
		virtual void render(SDL_Renderer* renderer);
		virtual void update(float elapsed_time, bool calc = true);
		std::string get_class() {return class_string;};
		bool is_a(std::string str) {return class_string == str;};

		// Inheriting classes must override this static function
		// Is there a better way to check class types?
		static std::string static_class() {return "GameObject";};

		std::pair<int, int> to_tile_space(std::pair<float, float> p){
			int x = (int)p.first;
			int y = (int)p.second;
			
			return std::make_pair(x/xtwidth, y/ytwidth);
		}

		std::pair<int, int> to_tile_space(Vector2f p){
			return to_tile_space(std::make_pair(p.x(), p.y()));
		}

		Vector2f to_world_space(std::pair<int, int> p){
			float x = (float)p.first;
			float y = (float)p.second;
			
			return Vector2f(xtwidth*x + (float)xtwidth/2, ytwidth*y + (float)ytwidth/2);
		}

		std::pair<int, int> get_tile(){
			return to_tile_space(p());
		}

};
