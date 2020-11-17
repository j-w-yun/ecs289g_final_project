#pragma once

#include <map>
#include <utility>

struct DragBox;

class Input {
	private:
		static std::map<SDL_Keycode, bool> keydown;
		static std::map<Uint8, bool> mousedown;
		static std::map<Uint8, DragBox> mousedrag;
		static SDL_Event e;
		Input() {}
		static void set_key(SDL_Event* e);
		static void set_mouse(SDL_Event* e);

	public:
		static bool process_inputs();
		static bool is_key_pressed(SDL_Keycode key);
		static bool is_mouse_pressed(Uint8 button);
		static bool has_dragbox(Uint8 button);
		static DragBox get_dragbox(Uint8 button);

		static std::pair<int, int> get_mouse_pos(){
			return std::make_pair(e.button.x, e.button.y);
		}
};
