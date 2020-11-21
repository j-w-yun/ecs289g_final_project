#pragma once

#include "Util.h"
#include "Vector2f.h"
#include "Vector2f.cpp"

float Stat::fps = 0;
float Stat::ups = 0;
unsigned int Stat::frames = 0;
unsigned int Stat::updates = 0;
unsigned int Stat::last_time = 0;
unsigned int Stat::total_frames = 0;
unsigned int Stat::total_updates = 0;
Vector2f Stat::screen;
Vector2f Stat::world;
Vector2f Stat::camera_position;
float Stat::camera_zoom;
std::vector<SDL_Surface*> Stat::surfaces;
std::vector<SDL_Texture*> Stat::textures;

SDL_Color Stat::color = {0, 255, 0};
SDL_Rect Stat::text_rect = {x: 8};
char Stat::text_str[64];
int Stat::w;
int Stat::h;
int Stat::offset_y;

void Stat::render_text(SDL_Renderer* renderer, TTF_Font* font) {
	Stat::surfaces.push_back(TTF_RenderText_Solid(font, Stat::text_str, Stat::color));
	Stat::textures.push_back(SDL_CreateTextureFromSurface(renderer, Stat::surfaces.back()));
	TTF_SizeText(font, Stat::text_str, &Stat::w, &Stat::h);
	Stat::text_rect.w = Stat::w;
	Stat::text_rect.h = Stat::h;
	Stat::text_rect.y = Stat::offset_y;
	SDL_RenderCopy(renderer, Stat::textures.back(), NULL, &Stat::text_rect);
	Stat::offset_y += Stat::h;
}

void Stat::calculate() {
	unsigned int current_time = (unsigned int)SDL_GetTicks();
	float delta_seconds = (current_time - Stat::last_time) / 1000;
	if (delta_seconds >= 0.5f && delta_seconds > 0) {
		Stat::last_time = current_time;
		Stat::fps = Stat::frames / delta_seconds;
		Stat::ups = Stat::updates / delta_seconds;
		Stat::updates = 0;
		Stat::frames = 0;
	}
}

void Stat::frame_tick() {
	Stat::frames++;
	Stat::total_frames++;
}

void Stat::update_tick() {
	Stat::updates++;
	Stat::total_updates++;
}

void Stat::render(SDL_Renderer* renderer, TTF_Font* font) {
	Stat::calculate();

	// Reset offset
	Stat::offset_y = 4;

	// Display time
	sprintf(text_str, "Time: %ims", (unsigned int)Util::get_milliseconds());
	render_text(renderer, font);

	// Display Frames
	sprintf(text_str, "Frames: %i", Stat::total_frames);
	render_text(renderer, font);

	// Display updates
	sprintf(text_str, "Updates: %i", Stat::total_updates);
	render_text(renderer, font);

	// Display FPS
	sprintf(text_str, "FPS: %.0f", Stat::fps);
	render_text(renderer, font);

	// Display UPS
	sprintf(text_str, "UPS: %.0f", Stat::ups);
	render_text(renderer, font);

	// Display world position
	sprintf(text_str, "World: (%.0f, %.0f)", Stat::world.x(), Stat::world.y());
	render_text(renderer, font);

	// Display screen position
	sprintf(text_str, "Screen: (%.0f, %.0f)", Stat::screen.x(), Stat::screen.y());
	render_text(renderer, font);

	// Display camera position
	sprintf(text_str, "Camera: (%.0f, %.0f)", Stat::camera_position.x(), Stat::camera_position.y());
	render_text(renderer, font);

	// Display camera zoom
	sprintf(text_str, "Zoom: %.0f", Stat::camera_zoom);
	render_text(renderer, font);
}

void Stat::free() {
	// Free stat rendering resources
	for (SDL_Surface* surface : Stat::surfaces)
		SDL_FreeSurface(surface);
	Stat::surfaces.clear();
	for (SDL_Texture* texture : Stat::textures)
		SDL_DestroyTexture(texture);
	Stat::textures.clear();
}

void Stat::mouse_info(Vector2f screen, Vector2f world) {
	Stat::screen = screen;
	Stat::world = world;
}

void Stat::camera_info(Vector2f position, float zoom) {
	Stat::camera_position = position;
	Stat::camera_zoom = zoom;
}
