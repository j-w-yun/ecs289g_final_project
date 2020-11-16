#pragma once

float Stat::fps = 0;
float Stat::ups = 0;
unsigned int Stat::frames = 0;
unsigned int Stat::updates = 0;
unsigned int Stat::last_time = 0;
SDL_Surface* Stat::fps_surface = NULL;
SDL_Texture* Stat::fps_texture = NULL;
SDL_Surface* Stat::ups_surface = NULL;
SDL_Texture* Stat::ups_texture = NULL;

void Stat::calculate() {
	unsigned int current_time = SDL_GetTicks();
	float delta_seconds = (current_time - Stat::last_time) / 1000;
	if (delta_seconds >= 0.5f && delta_seconds > 0) {
		Stat::last_time = current_time;
		Stat::fps = Stat::frames / delta_seconds;
		Stat::ups = Stat::updates / delta_seconds;

		// std::cout << std::endl;
		// std::cout << "FPS: " << Stat::fps << std::endl;
		// std::cout << "UPS: " << Stat::ups << std::endl;
		// std::cout << "Frames: " << Stat::frames << std::endl;
		// std::cout << "Updates: " << Stat::updates << std::endl;
		// std::cout << "Time: " << current_time << std::endl;
		// std::cout << std::endl;

		Stat::updates = 0;
		Stat::frames = 0;
	}
}

void Stat::frame_tick() {
	Stat::frames++;
}

void Stat::update_tick() {
	Stat::updates++;
}

void Stat::render(SDL_Renderer* renderer, TTF_Font* font) {
	Stat::calculate();
	SDL_Color color = {50, 255, 50};
	// Render stats
	SDL_Rect text_rect;
	text_rect.h = 20;
	text_rect.x = 8;
	text_rect.y = 4;
	// Display FPS
	char text_str[64];
	sprintf(text_str, "FPS: %.0f", Stat::fps);
	fps_surface = TTF_RenderText_Solid(font, text_str, color);
	fps_texture = SDL_CreateTextureFromSurface(renderer, Stat::fps_surface);
	text_rect.w = strlen(text_str) * 10;
	SDL_RenderCopy(renderer, Stat::fps_texture, NULL, &text_rect);
	// Display UPS
	sprintf(text_str, "UPS: %.0f", Stat::ups);
	ups_surface = TTF_RenderText_Solid(font, text_str, color);
	ups_texture = SDL_CreateTextureFromSurface(renderer, Stat::ups_surface);
	text_rect.w = strlen(text_str) * 10;
	text_rect.y += text_rect.h;
	SDL_RenderCopy(renderer, Stat::ups_texture, NULL, &text_rect);
}

void Stat::free() {
	// Free stat rendering resources
	SDL_FreeSurface(Stat::fps_surface);
	SDL_DestroyTexture(Stat::fps_texture);
	SDL_FreeSurface(Stat::ups_surface);
	SDL_DestroyTexture(Stat::ups_texture);
}
