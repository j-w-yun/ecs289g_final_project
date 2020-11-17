#pragma once

class Vector2f {
	private:
		float _x;
		float _y;

	public:
		Vector2f() = default;
		Vector2f(float x, float y);
		void set(float x, float y);
		float x() const;
		float y() const;
		Vector2f add(const Vector2f& v) const;
		Vector2f sub(const Vector2f& v) const;
		Vector2f mul(const Vector2f& v) const;
		Vector2f div(const Vector2f& v) const;
		Vector2f scale(float v) const;
		float dot(const Vector2f& v) const;
		float len2() const;
		float len() const;
		
		Vector2f operator+(Vector2f r){
			return add(r);
		}

		Vector2f operator+=(Vector2f r){
			auto temp = add(r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f operator-(Vector2f r){
			return sub(r);
		}

		Vector2f operator-=(Vector2f r){
			auto temp = sub(r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f operator*(float r){
			return scale(r);
		}

		Vector2f operator*=(float r){
			auto temp = scale(r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

		Vector2f operator/(float r){
			return scale(1/r);
		}

		Vector2f operator/=(float r){
			auto temp = scale(1/r);
			_x = temp._x;
			_y = temp._y;
			return temp;
		}

};
