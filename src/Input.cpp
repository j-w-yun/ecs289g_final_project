#pragma once

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

struct Scroll {
	int up;
	int down;
	int left;
	int right;
};

std::map<SDL_Keycode, bool> Input::keydown = {};
std::map<Uint8, bool> Input::mousedown = {};
std::map<Uint8, DragBox> Input::mousedrag = {};
Scroll Input::mousescroll;
std::pair<int, int> Input::mousepos;
SDL_Event Input::e;

void Input::set_wheel(SDL_Event* e) {
	if (e->wheel.y > 0) {
		// std::cout << "scroll up" << std::endl;
		Input::mousescroll.up++;
	}
	else if (e->wheel.y < 0) {
		// std::cout << "scroll down" << std::endl;
		Input::mousescroll.down++;
	}
	else if (e->wheel.x > 0) {
		// std::cout << "scroll right" << std::endl;
		Input::mousescroll.right++;
	}
	else if (e->wheel.x < 0) {
		// std::cout << "scroll left" << std::endl;
		Input::mousescroll.left++;
	}
}

/**
Parses key event to update keydown states.
List of SDLK symbols:
	https://wiki.libsdl.org/SDL_Keycode
*/
void Input::set_key(SDL_Event* e) {
	const bool is_pressed = e->type == SDL_KEYDOWN;

	// Initialize map pair if it does not exist
	if (Input::keydown.find(e->key.keysym.sym) == Input::keydown.end())
		Input::keydown.insert(std::make_pair(e->key.keysym.sym, is_pressed));
	else
		Input::keydown[e->key.keysym.sym] = is_pressed;
	if (DEBUG)
		std::cout << (is_pressed ? "pressed " : "released ") << e->key.keysym.sym << std::endl;
}

/**
Parses mouse event to update mousedown and mousedrag states.

Because SDL_MOUSEMOTION event does not tell us what buttons are currently down,
	we encode button state in dragbox. x1 and y1 of dragbox will be -1 or the
	dragbox will be undefined if the button is not down.

More information about mouse button events:
	https://wiki.libsdl.org/SDL_MouseButtonEvent
*/
void Input::set_mouse(SDL_Event* e) {
	// Event type may be SDL_MOUSEMOTION
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
		Input::mousepos = std::make_pair(e->button.x, e->button.y);
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

Check if a keyboard key is down or up using is_key_pressed(SDL_Keycode key).
Check if a mouse button is down or up using is_mouse_pressed(Uint8 button).
Check if a mouse button is dragging using has_dragbox(Uint8 button).
Get the mouse button dragbox using get_dragbox(Uint8 button).

Examples of SDL_Keycode key: SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT
Examples of Uint8 button: SDL_BUTTON_LEFT, SDL_BUTTON_MIDDLE, SDL_BUTTON_RIGHT

More information about SDL events:
	https://wiki.libsdl.org/SDL_Event

Returns false if user wants to quit.
*/
bool Input::process_inputs() {
	// Clear scroll counts
	Input::mousescroll.up = 0;
	Input::mousescroll.down = 0;
	Input::mousescroll.left = 0;
	Input::mousescroll.right = 0;

	// Handle events on queue
	while (SDL_PollEvent(&Input::e) != 0) {
		if (Input::e.type == SDL_QUIT) {
			return false;
		}
		else if (Input::e.type == SDL_MOUSEWHEEL) {
			Input::set_wheel(&Input::e);
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

std::pair<int, int> Input::get_mouse_pos() {
	// return std::make_pair(e.button.x, e.button.y);
	return Input::mousepos;
}

int Input::get_scrolly() {
	return Input::mousescroll.up - Input::mousescroll.down;
}

int Input::get_scrollx() {
	return Input::mousescroll.right - Input::mousescroll.left;
}

bool Input::has_input() {
	return Input::keydown.size() + Input::mousedown.size() > 0;
}

void Input::render(SDL_Renderer* renderer) {
	// Left mouse drag
	if (Input::has_dragbox(SDL_BUTTON_LEFT)) {
		DragBox box = Input::get_dragbox(SDL_BUTTON_LEFT);
		SDL_Rect dragbox = {box.x1, box.y1, box.x2-box.x1, box.y2-box.y1};
		// Render filled quad
		SDL_SetRenderDrawColor(renderer, 0x99, 0xFF, 0x99, 0x33);
		SDL_RenderFillRect(renderer, &dragbox);
		// Render outline quad
		SDL_SetRenderDrawColor(renderer, 0x99, 0xFF, 0x99, 0xFF);
		SDL_RenderDrawRect(renderer, &dragbox);
	}

	// Right mouse drag
	if (Input::has_dragbox(SDL_BUTTON_RIGHT)) {
		DragBox box = Input::get_dragbox(SDL_BUTTON_RIGHT);
		SDL_Rect dragbox = {box.x1, box.y1, box.x2-box.x1, box.y2-box.y1};
		// Render filled quad
		SDL_SetRenderDrawColor(renderer, 0x77, 0xAA, 0xFF, 0x44);
		SDL_RenderFillRect(renderer, &dragbox);
		// Render outline quad
		SDL_SetRenderDrawColor(renderer, 0x77, 0xAA, 0xFF, 0xFF);
		SDL_RenderDrawRect(renderer, &dragbox);
	}
}
