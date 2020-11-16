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
		GameObject();
		GameObject(const Vector2f& p, const Vector2f& v, float r);
		Vector2f p() const;
		Vector2f v() const;
		float r() const;
		void set_p(Vector2f& p);
		void set_v(Vector2f& v);
		void set_r(float r);
		void set_visible(bool b);
		void set_movable(bool b);
		bool is_visible();
		bool is_movable();
		bool overlaps(GameObject& other);
		void set_render_callback(std::function<void(SDL_Renderer*)> callback);
		void set_update_callback(std::function<void(float)> callback);
		void render(SDL_Renderer* renderer);
		void update(float elapsed_time);
		std::string get_class() {return class_string;};
		bool is_a(std::string str) {return class_string == str;};

		// Inheriting classes must override this static function
		// Is there a better way to check class types?
		static std::string static_class() {return "GameObject";};
};
