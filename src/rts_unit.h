#pragma once

#include "v2f.h"
#include "AStar.hpp"
#include "AStar.cpp"
#include "algorithms.h"
#include<vector>
#include<utility>

typedef std::pair<int, int> ip;

struct rts_unit : GameObject {
	rts_unit() = default;

	float acc;
	float topspeed;
	Vector2f dest = Vector2f(0, 0);
	std::vector<Vector2f> path;
	int selected = 0;
	MapLevel& map;
	int layers = 1;
	int side = layers*2 + 1;
	int size = side * side - 1;
	int target = -1;

	// performance limits
	int avoidance_limit = 10;
	int target_limit = 10;

	// shooting stuff
	float range = 150;
	int weapon_state = 0;
	int weapon_cap = 200;
	float weapon_speed = 5;
	int weapon_life = (int)(range/weapon_speed) + 3;
	int damage = 1;

	// group stuff
	int current_group = -1; // -1 means idle
	int role = -1;


	rts_unit(Vector2f p, Vector2f v, float r, int w, int h, int xt, int yt, int t, int hlt, float a, float ts, MapLevel& mp): GameObject(p, v, r, w, h, xt, yt, t, hlt), acc(a), topspeed(ts), map(mp) {}

	virtual void render(SDL_Renderer* renderer){
		// int padding = 4;
		// SDL_Rect box = {(int)(p().x() - r()) - padding, (int)(p().y() - r()) - padding, (int)(2 * r()) + 2*padding, (int)(2 * r()) + 2*padding};
		Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f((int)(p().x() - r()), (int)(p().y() - r())));
		Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((int)(p().x() + r()), (int)(p().y() + r())));
		SDL_Rect box = {
			(int)(sp1.x()),
			(int)(sp1.y()),
			(int)(sp2.x()-sp1.x()),
			(int)(sp2.y()-sp1.y())
		};
		// Fill
		SDL_SetRenderDrawColor(renderer, 255*(team==1), 255*(team==2), 255*(team==0), 255);
		if(current_group != -1){
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		}
		SDL_RenderFillRect(renderer, &box);
		// Outline
		if(selected){
			SDL_SetRenderDrawColor(renderer, 2, 117, 216, 255);
			SDL_RenderDrawRect(renderer, &box);
		}

		// path lines
		/*if(path.size()){
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			Vector2f p1 = RenderingEngine::world_to_screen(p());
			Vector2f p2 = RenderingEngine::world_to_screen(path.back());
			SDL_RenderDrawLine(renderer, p1.x(), p1.y(), p2.x(), p2.y());
			for(size_t i = path.size() - 1; i; i--){
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				Vector2f p1 = RenderingEngine::world_to_screen(path[i]);
				Vector2f p2 = RenderingEngine::world_to_screen(path[i - 1]);
				SDL_RenderDrawLine(renderer, p1.x(), p1.y(), p2.x(), p2.y());
			}
		}*/
	}

	virtual void update_path(){
		std::vector<Vector2f> find_rect_path(Vector2f s, Vector2f d);
		auto tpath = map.find_rect_path(p(), dest);

		path = tpath;
	}

	// returns unit
	virtual Vector2f traverse_path(){
		if(!path.size()){
			set_v(v() * .95);
			return Vector2f(0, 0);
		}

		auto wpoint = path.back();

		auto d = wpoint - p();
		auto dist = d.len();

		bool recomp = false;

		//if(dist > std::max((float)xtwidth/2, (float)xtwidth/2)){
		//	update_path();
		//	recomp = true;
		//}
		if(dist < (float)xtwidth){
			path.pop_back();
			recomp = true;
		}
		else if(path.size() > 1 && (path[path.size() - 2] - path.back()).dot(p() - path.back()) > 0){
			path.pop_back();
			recomp = true;
		}
		if(recomp){
			if(!path.size()){
				set_v(v() * .95);
				return Vector2f(0, 0);
			}

			wpoint = path.back();
			d = wpoint - p();
		}

		return d.unit();
	}

	inline bool inbounds(int x, int y) {
		return x >= 0 && x < x_tiles && y >= 0 && y < y_tiles;
	}

	/*std::vector<ip> neighbors() {
		auto src = to_tile_space(p());

		short ctr = 0;
		for (int i = -layers; i <= layers; i++) {
			for (int j = -layers; j <= layers; j++) {
				if (!i && !j)
					continue;

				ip p = {src.first + i, src.second + j};

				if (inbounds(p.first, p.second))
					retval.push_back(p);

				ctr++;
			}
		}

		return retval;
	}*/

	// returns unit
	virtual Vector2f avoid_obstacles(){
		Vector2f retval(0, 0);
		Vector2f d;
		//Vector2f l;
		int x, y;

		auto pr = to_tile_space(p());

		int checked = 0;

		float r2 = r() * r();

		for(int i = 0; i < size; i++){
			x = pr.first + i/side - layers;
			y = pr.second + i%side - layers;

			if(inbounds(x, y)){
				// wall
				if(map.get_obgrid()[x][y]){
					d = p() - to_world_space({x, y});
					//auto l = d.len();
					//float force = 40.0f/(d.len2());

					retval += 40.0f*d.unit()/(d.len2()/r2);
				}
				// units
				else if(checked < avoidance_limit){
					for(int t = 0; t < map.get_teams(); t++){
						for(auto uind : map.get_unitgrid()[t][x][y]){
							if(uind == id) continue;

							auto casted = std::dynamic_pointer_cast<rts_unit>(map.get_units()[uind]);

							// don't avoid idling units
							if(casted && current_group != -1 && casted->current_group == -1){
								continue;
							}

							d = p() - map.get_units()[uind]->p();
							//auto l = d.len();
							retval += 5.0f*d.unit()/(std::max(d.len2()/r2, .001f));

							checked++;
							//if(checked >= avoidance_limit){
							//	goto escape_avoid_obstacles;
							//}
						}
					}
				}
			}
		}

		//escape_avoid_obstacles:

		return retval;
	}

	/*virtual Vector2f avoid_units(){
		Vector2f retval(0, 0);
		Vector2f d;
		//Vector2f l;
		int x, y;

		int checked = 0;

		Vector2f vu = v().unit();
		Vector2f pu = par_unit(v());

		auto pr = to_tile_space(p());

		for(int i = 0; i < size; i++){
			x = pr.first + i/side - layers;
			y = pr.second + i%side - layers;
			if(inbounds(x, y) && !map.get_obgrid()[x][y]){
				for(auto uind : map.get_unitgrid()[x][y]){
					//std::cout << "unid is  " << uind << std::endl;

					// don't avoid self
					if(uind == id) continue;

					auto& unit = *(map.get_units()[uind]);

					d = unit.p() - p();

					d = components(v(), d);

					// only influenced by units in front
					//if(d.y() < 0) continue;

					//float slow = -d.y() / d.x();
					//float turn = d.x()/d.y();

					int sign = (d.x() > 0) * 2 - 1;

					auto fun = v().len() * exp(-(d.y() * d.x()));
					float slow = fun/std::max(d.y(), .001f);
					float turn = sign * fun/std::max(abs(d.x()), .001f);

					retval = retval + slow*vu + turn*pu;

					checked++;
					if(checked >= avoidance_limit){
						return retval;
					}
					//std::cout << "done" << std::endl;
				}
			}
		}

		return retval;
	}*/

	void find_target(){
		//std::cout << std::endl << std::endl << "Find target" << std::endl;

		if(target != -1 && map.get_units()[target] && (map.get_units()[target]->p() - p()).len() < range){
			//std::cout << "Retaining target" << std::endl;
			return;
		}

		int tiles = (int)(range/std::min(xtwidth, ytwidth));
		int sd = tiles*2 + 1;
		auto pr = to_tile_space(p());

		std::vector<int> targets;
		std::vector<float> distances;

		//std::cout << "Center is " << pr.first << ", " << pr.second << std::endl;

		int checked = 0;

		for(int i = 0; i < sd*sd; i++){
			int x = pr.first + i/sd - tiles;
			int y = pr.second + i%sd - tiles;

			//std::cout << "Checking block " << x << ", " << y << std::endl;

			if(!inbounds(x, y))
				continue;

			//std::cout << "In bounds" << std::endl;

			//std::cout << "Units here: " << map.get_unitgrid()[x][y].size() << std::endl;

			for(int t = 0; t < map.get_teams(); t++){
				if(t == team) continue;

				for(auto& id : map.get_unitgrid()[t][x][y]){
					auto& unit = map.get_units()[id];
					if(unit->team != team){
						auto d = (unit->p() - p()).len();
						if(d > range)
							continue;

						targets.push_back(id);
						distances.push_back((unit->p() - p()).len());

						checked++;
						if(checked >= target_limit){
							goto escape;
						}
					}
				}
			}
		}

		escape: 
		(void)distances;

		if(!targets.size()){
			target = -1;
			//std::cout << "No targets" << std::endl;
			return;
		}

		target = targets[rand()%targets.size()];
		//std::cout << "Target is " << target << std::endl;
	}

	virtual bool update(float elapsed_time, bool calc){
		//std::cout << "unit pos before " << p() << std::endl;

		// try to join group
		// TODO join fairly somehow
		if(current_group == -1){
			auto mg = map.get_managers()[team];
			for(int g = 0; g < (int)mg.groups.size(); g++){
				auto& gp = *(mg.groups[g]);
				if(gp.recruiting && gp.members < gp.capacity){
					current_group = g;
					role = gp.join();
					//std::cout << "Unit " << id << " joining" << std::endl;
					break;
				}
			}
		}

		if(current_group != -1){
			// destination for current group/role
			dest = map.get_managers()[team].groups[current_group]->role_dest(role);
		}
		else{
			dest = map.get_managers()[team].rally_point;
		}

		//if(current_group != -1 && !(rand() % 50)){
        //	std::cout << "group: " << current_group << ", dest " << dest << std::endl;
    	//}

		// check for death
		if(health <= 0){
			if(current_group != -1){
				map.get_managers()[team].groups[current_group] -> leave(role);
			}
			return false;
		}

		// load weapon
		if(weapon_state < weapon_cap){
			weapon_state++;
		}

		// verify target is still alive
		if(target != -1 && !map.get_units()[target]){
			target = -1;
		}

		// shoot at target
		if(weapon_state == weapon_cap && target != -1){
			auto& unit = map.get_units()[target];
			auto dv = unit->p() - p();
			auto d = dv.len();
		
			if(d < range){
				map.add_proj(std::make_shared<projectile>(p(), (dv/d)*weapon_speed, 2, weapon_life, team, 1, map));
				weapon_state = 0;
			}
		}

		auto temp = p() + v();

		auto temp_tile = to_tile_space(temp);

		if(map.inbounds(map.get_obgrid(), temp_tile) && !map.get_obgrid()[temp_tile.first][temp_tile.second]){
			set_p(temp);
		}

		//set_p(p() + v());

		auto x = p().x(), y = p().y();

		float o = .0001;

		// bounce
		if(x - r() < 0){
			set_p(Vector2f(r() + o, p().y()));
			set_v(Vector2f(-v().x(), v().y()));
		}
		if(y - r() < 0){
			set_p(Vector2f(p().x(), r() + 0));
			set_v(Vector2f(v().x(), -v().y()));
		}
		if(x + r() > wwidth){
			set_p(Vector2f(wwidth - r() - o, p().y()));
			set_v(Vector2f(-v().x(), v().y()));
		}
		if(y + r() > wheight){
			set_p(Vector2f(p().x(), wheight - r() - o));
			set_v(Vector2f(v().x(), -v().y()));
		}
		
		//std::cout << "unit pos after " << p() << std::endl;

		if(selected && Input::is_mouse_pressed(SDL_BUTTON_RIGHT)){
			auto temp = Input::get_mouse_pos();
			dest = RenderingEngine::screen_to_world(Vector2f(temp.first, temp.second));
		}

		if(calc){
			update_path();
			find_target();
		}

		auto deliberate = traverse_path()*acc;
		auto avoidance = avoid_obstacles();
		//auto group = avoid_units();
		//Vector2f group(0, 0);
		//std::cout << "deliberate is " << deliberate << std::endl;
		//std::cout << "avoidance is " << avoidance << std::endl;
		//std::cout << "group is " << group << std::endl;
		//Vector2f avoidance(0, 0);
		auto dv = deliberate+avoidance;

		if(dv.len() > acc){
			//std::cout << "In if" << std::endl;
			dv = (dv/dv.len())*acc;
		}

		//std::cout << "dv is " << dv << std::endl;
		//std::cout << "v is " << v() << std::endl;

		auto nv = v() + dv;
		if(nv.len() > topspeed){
			//std::cout << "In if" << std::endl;
			nv = (nv/nv.len())*topspeed;
		}

		//std::cout << "nv is " << nv << std::endl;

		set_v(nv);

		return true;
	}
};