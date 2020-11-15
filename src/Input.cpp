#define DEBUG true

#if DEBUG
#include <iostream>
std::string _mouse_button_str(Uint8 button) {
	if (button == SDL_BUTTON_LEFT) {
		return "left mouse button";
	}
	else if (button == SDL_BUTTON_MIDDLE) {
		return "middle mouse button";
	}
	else if (button == SDL_BUTTON_RIGHT) {
		return "right mouse button";
	}
	else if (button == SDL_BUTTON_X1) {
		return "x1 mouse button";
	}
	else if (button == SDL_BUTTON_X2) {
		return "x2 mouse button";
	}
	return "unknown mouse button";
}
#endif

struct DragBox {
	int x1;
	int y1;
	int x2;
	int y2;
};

std::map<SDL_Keycode, bool> Input::keydown = {};
std::map<Uint8, bool> Input::mousedown = {};
std::map<Uint8, DragBox> Input::mousedrag = {};
SDL_Event Input::e;

void Input::set_key(SDL_Event* e) {
	const bool is_pressed = e->type == SDL_KEYDOWN ? true : false;
	// Initialize map pair if it does not exist
	if (Input::keydown.find(e->key.keysym.sym) == Input::keydown.end())
		Input::keydown.insert(std::make_pair(e->key.keysym.sym, is_pressed));
	else
		Input::keydown[e->key.keysym.sym] = is_pressed;
	if (DEBUG)
		std::cout << (is_pressed ? "pressed " : "released ") << e->key.keysym.sym << std::endl;
}

void Input::set_mouse(SDL_Event* e) {
	const bool is_pressed = e->type == SDL_MOUSEBUTTONDOWN;
	const bool is_released = e->type == SDL_MOUSEBUTTONUP;
	// Initialize map pair if it does not exist
	if (is_pressed || is_released) {
		if (Input::mousedown.find(e->button.button) == Input::mousedown.end())
			Input::mousedown.insert(std::make_pair(e->button.button, is_pressed));
		else
			Input::mousedown[e->button.button] = is_pressed;
		
		if (Input::mousedrag.find(e->button.button) == Input::mousedrag.end()) {
			struct DragBox box;
			if (is_pressed)
				box = {e->button.x, e->button.y, e->button.x, e->button.y};
			else
				box = {-1, -1, -1, -1};
			Input::mousedrag.insert(std::make_pair(e->button.button, box));
		}
	}
	
	// Mutate map pair
	if (is_released) {
		// Mouse up
		Input::mousedrag[e->button.button].x1 = -1;
		Input::mousedrag[e->button.button].y1 = -1;
		Input::mousedrag[e->button.button].x2 = -1;
		Input::mousedrag[e->button.button].y2 = -1;
		if (DEBUG)
			std::cout << "released " << _mouse_button_str(e->button.button) << " : " << e->button.x << ", " << e->button.y << std::endl;
	}
	else if (is_pressed) {
		// Mouse down
		Input::mousedrag[e->button.button].x1 = e->button.x;
		Input::mousedrag[e->button.button].y1 = e->button.y;
		Input::mousedrag[e->button.button].x2 = e->button.x;
		Input::mousedrag[e->button.button].y2 = e->button.y;
		if (DEBUG)
			std::cout << "pressed " << _mouse_button_str(e->button.button) << " : " << e->button.x << ", " << e->button.y << std::endl;
	}
	else {
		// Mouse motion. Update drag box of all down buttons.
		for (auto &x : Input::mousedrag) {
			if (x.second.x1 >= 0 && x.second.y1 >= 0) {
				x.second.x2 = e->button.x;
				x.second.y2 = e->button.y;
				if (DEBUG)
					std::cout << "dragged " << _mouse_button_str(x.first) << " : " << e->button.x << ", " << e->button.y << std::endl;
			}
		}
	}
}

/**
Updates keyboard and mouse input states.
Check if a keyboard key is down or up using is_pressed(char).
List of SDLK symbols:
	https://wiki.libsdl.org/SDL_Keycode
Returns false if user wants to quit.
*/
bool Input::process_inputs() {
	// Handle events on queue
	while (SDL_PollEvent(&Input::e) != 0) {
		if (Input::e.type == SDL_QUIT) {
			return false;
		}
		else if (Input::e.type == SDL_MOUSEMOTION || Input::e.type == SDL_MOUSEBUTTONDOWN || Input::e.type == SDL_MOUSEBUTTONUP) {
			// Set mouse input state
			Input::set_mouse(&Input::e);
		}
		else if (Input::e.type == SDL_KEYDOWN || Input::e.type == SDL_KEYUP) {
			// Exit program on Esc
			if (Input::e.key.keysym.sym == SDLK_ESCAPE)
				return false;
			// Set key input state
			Input::set_key(&Input::e);
		}
	}
	return true;
}

/**
Checks if key was pressed until the last time input queue was processed.
Returns true if key was down and unreleased.
Returns false if key was released or never pressed. 
*/
bool Input::is_key_pressed(SDL_Keycode key) {
	if (Input::keydown.find(key) == Input::keydown.end())
		return false;
	return Input::keydown[key];
}

/**
Checks if mouse was pressed until the last time input queue was processed.
Returns true if mouse was down and unreleased.
Returns false if mouse was released or never pressed. 
*/
bool Input::is_mouse_pressed(Uint8 button) {
	if (Input::mousedown.find(button) == Input::mousedown.end())
		return false;
	return Input::mousedown[button];
}

bool Input::has_dragbox(Uint8 button) {
	if (Input::mousedrag.find(button) == Input::mousedrag.end())
		return false;
	return Input::mousedrag[button].x1 >= 0 && Input::mousedrag[button].y1 >= 0;
}

DragBox Input::get_dragbox(Uint8 button) {
	if (!Input::has_dragbox(button))
		return {-1, -1, -1, -1};
	return Input::mousedrag[button];
}