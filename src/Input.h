#pragma once

#include <map>
#include <utility>

struct DragBox;
struct Scroll;

class Input {
	private:
		static std::map<SDL_Keycode, bool> keydown;
		static std::map<Uint8, bool> mousedown;
		static std::map<Uint8, DragBox> mousedrag;
		static Scroll mousescroll;
		static std::pair<int, int> mousepos;
		static SDL_Event e;
		Input() {}
		static void set_key(SDL_Event* e);
		static void set_mouse(SDL_Event* e);
		static void set_wheel(SDL_Event* e);

	public:
		static bool process_inputs();
		static bool is_key_pressed(SDL_Keycode key);
		static bool is_mouse_pressed(Uint8 button);
		static bool has_dragbox(Uint8 button);
		static DragBox get_dragbox(Uint8 button);
		static std::pair<int, int> get_mouse_pos();
		static int get_scrolly();
		static int get_scrollx();
		static bool has_input();
		static void render(SDL_Renderer* renderer);
};
