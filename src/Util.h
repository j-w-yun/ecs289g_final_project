#pragma once

#include <iostream>
#include <time.h>
#include <random>

namespace Util {

	/*
	Time utilities
	*/

	const Uint64 INIT_COUNT = SDL_GetPerformanceCounter();
	const Uint64 COUNT_PER_S = SDL_GetPerformanceFrequency();

	Uint64 get_counts(bool is_relative=true) {
		return SDL_GetPerformanceCounter() - (is_relative ? INIT_COUNT : 0);
	}

	Uint64 get_nanoseconds(bool is_relative=true) {
		return get_counts(is_relative) / (COUNT_PER_S / (Uint64)1e9);
	}

	Uint64 get_microseconds(bool is_relative=true) {
		return get_counts(is_relative) / (COUNT_PER_S / (Uint64)1e6);
	}

	Uint64 get_milliseconds(bool is_relative=true) {
		return get_counts(is_relative) / (COUNT_PER_S / (Uint64)1e3);
	}

	Uint64 get_seconds(bool is_relative=true) {
		return get_counts(is_relative) / COUNT_PER_S;
	}

	class Timer {
		public:
			Uint64 start_time;
			Timer() {
				start_time = get_nanoseconds(true);
			}
			Uint64 delta() {
				return get_nanoseconds(true) - start_time;
			}
			Uint64 reset() {
				Uint64 _delta = delta();
				start_time = get_nanoseconds(true);
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
		std::cout << "Util::get_counts(): " << Util::get_counts() << std::endl;
		std::cout << "Util::get_nanoseconds(): " << Util::get_nanoseconds() << std::endl;
		std::cout << "Util::get_microseconds(): " << Util::get_microseconds() << std::endl;
		std::cout << "Util::get_milliseconds(): " << Util::get_milliseconds() << std::endl;
		std::cout << "Util::get_seconds(): " << Util::get_seconds() << std::endl;
		std::cout << std::endl;
	}
};