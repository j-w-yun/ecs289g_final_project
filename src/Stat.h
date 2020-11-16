#pragma once

class Stat {
	private:
		static float fps;
		static float ups;
		static unsigned int frames;
		static unsigned int updates;
		static unsigned int last_time;
		static SDL_Surface* fps_surface;
		static SDL_Texture* fps_texture;
		static SDL_Surface* ups_surface;
		static SDL_Texture* ups_texture;
		Stat() {}
		static void calculate();

	public:
		static void frame_tick();
		static void update_tick();
		static void render(SDL_Renderer* renderer, TTF_Font* font);
		static void free();
};
