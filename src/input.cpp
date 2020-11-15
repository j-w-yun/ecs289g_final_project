#include <map>

std::map<char, bool> keymap = {};
SDL_Event e;

/**
Updates keyboard input states.
Check if a key is down or up using is_pressed(char).
List of SDLK symbols:
	https://wiki.libsdl.org/SDL_Keycode
Returns false if user wants to quit.
*/
bool process_inputs() {
	// Handle events on queue
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			return false;
		}
		else if (e.type == SDL_KEYDOWN) {
			// Set key down state. Create mapping if not found.
			if (keymap.find(e.key.keysym.sym) == keymap.end())
				keymap.insert(std::make_pair(e.key.keysym.sym, true));
			else
				keymap[e.key.keysym.sym] = true;
		}
		else if (e.type == SDL_KEYUP) {
			if (e.key.keysym.sym == SDLK_ESCAPE)
				return false;
			// Set key up state. Create mapping if not found.
			if (keymap.find(e.key.keysym.sym) == keymap.end())
				keymap.insert(std::make_pair(e.key.keysym.sym, false));
			else
				keymap[e.key.keysym.sym] = false;
		}
	}
	return true;
}

bool is_pressed(char key) {
	if (keymap.find(key) == keymap.end())
		return false;
	else
		return keymap[key];
}
