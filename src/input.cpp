// #include <iostream>
#include <map>

std::map<char, bool> keymap = {};
SDL_Event e;

// https://wiki.libsdl.org/SDL_Keycode
bool process_inputs() {
	// Handle events on queue
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			return false;
		}
		else if (e.type == SDL_KEYDOWN) {
			// std::cout << "DOWN " << e.key.keysym.sym << std::endl;
			if (keymap.find(e.key.keysym.sym) == keymap.end())
				keymap.insert(std::make_pair(e.key.keysym.sym, true));
			else
				keymap[e.key.keysym.sym] = true;
		}
		else if (e.type == SDL_KEYUP) {
			// std::cout << "UP " << e.key.keysym.sym << std::endl;
			if (e.key.keysym.sym == SDLK_ESCAPE)
				return false;
			else if (keymap.find(e.key.keysym.sym) == keymap.end())
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
