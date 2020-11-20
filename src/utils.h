#pragma once

#include <time.h>

namespace utils {

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

	void test() {
		std::cout << std::endl;
		std::cout << "SDL_GetPerformanceCounter(): " << SDL_GetPerformanceCounter() << std::endl;
		std::cout << "SDL_GetTicks(): " << SDL_GetTicks() << std::endl;
		std::cout << "utils::get_counts(): " << utils::get_counts() << std::endl;
		std::cout << "utils::get_nanoseconds(): " << utils::get_nanoseconds() << std::endl;
		std::cout << "utils::get_microseconds(): " << utils::get_microseconds() << std::endl;
		std::cout << "utils::get_milliseconds(): " << utils::get_milliseconds() << std::endl;
		std::cout << "utils::get_seconds(): " << utils::get_seconds() << std::endl;
		std::cout << std::endl;
	}
};
