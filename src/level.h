#pragma once

#include <vector>
#include <set>
#include <functional>

typedef std::pair<int, int> ip;

struct level {
	int width;
	int height;
	std::vector<ip> obstructions;
	std::function<bool(ip, ip)> comp = [&](ip l, ip r) {
		return l.first + l.second*height < r.first + r.second*height;
	};

	level() = default;
	level(int w, int h, std::vector<std::pair<int, int>> obs = {}): width(w), height(h), obstructions(obs) {}
};
