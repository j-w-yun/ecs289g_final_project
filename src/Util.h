#pragma once

#include <iostream>
#include <time.h>
#include <random>


namespace Util {

	/*
	Time utilities
	*/

	Uint64 INIT_COUNT = SDL_GetPerformanceCounter();
	Uint64 COUNT_PER_S = SDL_GetPerformanceFrequency();

	Uint64 get_counts(bool is_relative=true) {
		if (is_relative)
			return SDL_GetPerformanceCounter() - INIT_COUNT;
		return SDL_GetPerformanceCounter();
	}

	Uint64 get_nanoseconds(bool is_relative=true) {
		return get_counts(is_relative) / SDL_GetPerformanceFrequency() * 1000000000.0;
	}

	Uint64 get_microseconds(bool is_relative=true) {
		return get_counts(is_relative) / SDL_GetPerformanceFrequency() * 1000000.0;
	}

	Uint64 get_milliseconds(bool is_relative=true) {
		return get_counts(is_relative) / SDL_GetPerformanceFrequency() * 1000.0;
	}

	Uint64 get_seconds(bool is_relative=true) {
		return get_counts(is_relative) / SDL_GetPerformanceFrequency();
	}

	class Timer {
		public:
			Uint64 start_time;
			Timer() {
				// start_time = get_nanoseconds(true);
				start_time = SDL_GetTicks();
			}
			Uint64 delta() {
				// return get_nanoseconds(true) - start_time;
				return SDL_GetTicks() - start_time;
			}
			Uint64 reset() {
				Uint64 _delta = delta();
				// start_time = get_nanoseconds(true);
				start_time = SDL_GetTicks();
				return _delta;
			}
	};

	/*
	Random utilities
	*/

	void seed_random() {
		srand(get_counts(false));
	}

	float uniform_random(float min, float max) {
		seed_random();
		return ((float)rand()) / (RAND_MAX/(max-min)) + min;
	}

	float normal_random(float mean, float std) {
		seed_random();
		std::default_random_engine generator;
		std::normal_distribution<double> distribution(mean, std);
		return (float)distribution(generator);
	}

	void test() {
		std::cout << std::endl;
		std::cout << "SDL_GetPerformanceCounter(): " << SDL_GetPerformanceCounter() << std::endl;
		std::cout << "SDL_GetTicks(): " << SDL_GetTicks() << std::endl;
		// std::cout << "COUNT_PER_S: " << COUNT_PER_S << std::endl;
		std::cout << "Util::get_counts(): " << Util::get_counts() << std::endl;
		std::cout << "Util::get_nanoseconds(): " << Util::get_nanoseconds() << std::endl;
		std::cout << "Util::get_microseconds(): " << Util::get_microseconds() << std::endl;
		std::cout << "Util::get_milliseconds(): " << Util::get_milliseconds() << std::endl;
		std::cout << "Util::get_seconds(): " << Util::get_seconds() << std::endl;
		std::cout << std::endl;
	}
};
