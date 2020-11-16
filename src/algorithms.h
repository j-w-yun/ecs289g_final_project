#pragma once

#include<vector>
#include<map>
#include<set>
#include<algorithm>
#include<iostream>

typedef std::pair<int, int> ip;

template<class T1, class T2>
bool in(T1 elem, T2 cont) {
	return find(cont.begin(), cont.end(), elem) != cont.end();
}

struct astar_node {
	ip pos;
	ip prev;
	double sdist;
	double ddist;
	bool obs;

	astar_node() = default;
	astar_node(ip pos, ip prev, double sd, double dd, bool obs = false): pos(pos), prev(prev), sdist(sd), ddist(dd), obs(obs) {}
};

template<class T1, class T2>
std::ostream& operator<<(std::ostream& os, std::pair<T1, T2> p) {
	os << "[" << p.first << ", " << p.second << "]";
	return os;
}

std::ostream& operator<<(std::ostream& os, astar_node& as) {
	os << "as(" << as.pos << ", " << as.prev << ", " << as.sdist << ", " << as.ddist << ", " << as.obs << ")";
	return os;
}

bool operator==(const astar_node& l, const astar_node& r) {
	return l.pos == r.pos;
}

// takes operator >
struct minheap {
	std::vector<astar_node> vec;
	std::function<bool(astar_node&, astar_node&)> comp;

	minheap(std::vector<astar_node> v, std::function<bool(astar_node&, astar_node&)> comp): comp(comp) {
		vec = v;
		std::make_heap(v.begin(), v.end(), comp);
	}

	astar_node pop() {
		std::pop_heap(vec.begin(), vec.end(), comp);
		auto an = vec.back();
		vec.pop_back();
		return an;
	}

	void insert(astar_node an) {
		vec.push_back(an);
		std::make_heap(vec.begin(), vec.end(), comp);
	}

	size_t size() {
		return vec.size();
	}
};

double dist(ip f, ip s) {
	return sqrt(pow(f.first - s.first, 2) + pow(f.second - s.second, 2));
}

std::vector<ip> neighbors(ip src, int width, int height) {
	auto inbounds = [&](int x, int y) {
		return x >= 0 && x < width && y >= 0 && y < height;
	};

	std::vector<ip> retval;

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (!i && !j)
				continue;

			ip p = {src.first + i, src.second + j};

			// avoid obstacles
			//if (obs.find(p) != obs.end()) continue;

			if (inbounds(p.first, p.second))
				retval.push_back(p);
		}
	}

	return retval;
}

template<class mp>
std::vector<ip> reconstruct_path(mp& map, ip src, ip dest) {
	auto curr = dest;

	std::vector<ip> path;

	// TODO possibly include current node in path?
	while (curr != src) {
		path.push_back(curr);
		curr = map[curr].prev;
	}

	std::reverse(path.begin(), path.end());

	return path;
}

std::vector<ip> astar(level& lev, ip src, ip dest) {
	std::cout << "In A*" << std::endl;

	auto width = lev.width;
	auto height = lev.height;

	//auto comp = [&](ip l, ip r) { return l.first + l.second*height < r.first + r.second*height; };
	//auto heapcomp = [&](astar_node l, astar_node r) { return l.pos.first + l.pos.second*height < r.pos.first + r.pos.second*height; };

	//std::map<ip, astar_node, comp> visited;
	auto visited = std::map<ip, astar_node, std::function<bool(const ip&, const ip&)>> {
		[&](ip l, ip r) {
			return l.first + l.second*height < r.first + r.second*height;
		}
	};

	for (auto& obs : lev.obstructions) {
		auto n = astar_node(obs, obs, 0, 0, true);
		visited[obs] = n;
		std::cout << "Adding obstacle " << n << std::endl;
	}

	astar_node first(src, src, 0, dist(src, dest));

	std::cout << "First is " << first << std::endl;

	//minheap(std::vector<astar_node>({first}), heapcomp);
	//minheap hp(std::vector<astar_node>({first}), [&](astar_node l, astar_node r) { return l.pos.first + l.pos.second*height < r.pos.first + r.pos.second*height; });
	minheap hp(std::vector<astar_node>({first}),
		[&](astar_node l, astar_node r) {
			return l.sdist + l.ddist > r.sdist + r.ddist;
		}
	);

	//int iters = 0;
	//int maxit = 20;

	while (hp.size()) {

		/*if (iters++ == maxit) {
			std::cout << "A* max iters" << std::endl;
			return {};
		}*/

		auto current = hp.pop();

		std::cout << "Processing " << current << std::endl;

		// construct path and return
		if (current.pos == dest)
			return reconstruct_path(visited, src, dest);

		auto neigh = neighbors(current.pos, width, height);

		for (auto& npos : neigh) {

			// initialize if needed
			if (!visited.count(npos)) {
				auto nnode = astar_node(npos, current.pos, current.sdist + 1, dist(npos, dest));
				visited[npos] = nnode;
				hp.insert(nnode);
			}

			// skip obstructions
			if (visited[npos].obs)
				continue;

			// neighbor node
			auto& nnode = visited[npos];

			// update if closer
			if (current.sdist + 1 < nnode.sdist) {
				nnode.sdist = current.sdist + 1;
				nnode.prev = current.pos;

				auto it = std::find(hp.vec.begin(), hp.vec.end(), nnode);
				if (it == hp.vec.end()) {
					hp.insert(nnode);
					std::cout << "Inserting " << nnode << std::endl;
				}
				else {
					*it = nnode;
					std::cout << "Updating " << nnode << std::endl;
				}
			}
		}
	}

	return {};
}