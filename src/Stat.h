#pragma once

#include <vector>

#include "Vector2f.h"

class Stat {
	private:
		static float fps;
		static float ups;
		static unsigned int frames;
		static unsigned int updates;
		static unsigned int last_time;
		static unsigned int total_frames;
		static unsigned int total_updates;
		static Vector2f screen;
		static Vector2f world;
		static Vector2f camera_position;
		static float camera_zoom;
		static std::vector<SDL_Surface*> surfaces;
		static std::vector<SDL_Texture*> textures;
		static char text_str[64];
		static SDL_Color color;
		static SDL_Rect text_rect;
		static int w;
		static int h;
		static int offset_y;

		Stat() {}
		static void calculate();
		static void render_text(SDL_Renderer* renderer, TTF_Font* font);

	public:

		static void frame_tick();
		static void update_tick();
		static void render(SDL_Renderer* renderer, TTF_Font* font);
		static void free();
		static void mouse_info(Vector2f screen, Vector2f world);
		static void camera_info(Vector2f position, float zoom);
};
