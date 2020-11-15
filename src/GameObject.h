#include "Vector2f.h"
// #include "Vector2f.cpp"

class GameObject {
	private:
		Vector2f* _position;
		Vector2f* _velocity;
		float _radius;
	
	public:
		GameObject(Vector2f* p, Vector2f* v, float radius);
		Vector2f* position();
		Vector2f* velocity();
		float radius();
		void set_position(Vector2f* p);
		void set_velocity(Vector2f* v);
		void set_radius(float r);
};
