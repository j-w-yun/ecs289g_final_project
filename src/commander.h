#pragma once

#include "rts_unit.h"

struct commander : rts_unit{
    int max_health;

    int regen_state = 0;
    int regen_cap = 200;

    Vector2f attack_point;
    bool idle = false;
    int state = 0;

    int s_key = SDLK_e;
    bool s_keydown = false;

    commander(Vector2f p, Vector2f v, float r, int w, int h, int xt, int yt, int t, int hlt, float a, float ts, MapLevel& mp, Vector2f ap): rts_unit(p, v, r, w, h, xt, yt, t, hlt, a, ts, mp), max_health(hlt), attack_point(ap) {
        // shooting stuff
        range = 175;
        weapon_state = 0;
        weapon_cap = 25;
        weapon_speed = 5;
        weapon_life = (int)(range/weapon_speed) + 3;
        damage = 1;

        max_priority = true;
    }

    virtual void render(SDL_Renderer* renderer){
		Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f((int)(p().x() - r()), (int)(p().y() - r())));
		Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((int)(p().x() + r()), (int)(p().y() + r())));
		SDL_Rect box = {
			(int)(sp1.x()),
			(int)(sp1.y()),
			(int)(sp2.x()-sp1.x()),
			(int)(sp2.y()-sp1.y())
		};

        float health_prop = (float)health/max_health;

        int r, g, b;

        if(!selected){
            r = (team==1)*255*health_prop;
            g = (team==1)*80*health_prop;
            b = (team==0)*255*health_prop;
        }
        else{
            r = 50;
            g = 255*health_prop;
            b = 50;
        }

#ifdef USE_SDL2_RENDERER
		// Fill
		SDL_SetRenderDrawColor(renderer, r, g, b, 255);
		//if(current_group != -1){
		//	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//}
		SDL_RenderFillRect(renderer, &box);
		// Outline

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
#else
		RenderingEngine::ogl_set_color(r, g, b, 255);

		RenderingEngine::ogl_fill_rect(box);


#endif
	}

    virtual bool update(float elapsed_time, bool calc){
		// check for death
		if(health <= 0){
			return false;
		}

        if(!team){
            if(Input::is_key_pressed(s_key)){
                if(!s_keydown){
                    selected = !selected;
                }
                
                s_keydown = true;
            }
            else{
                s_keydown = false;
            }
        }

        
        if(regen_state < regen_cap){
            regen_state++;
        }
        else{
            if(health < max_health){
                health++;
                regen_state = 0;
            }
        }

        if(state == 0 && health < .4 * max_health){
            std::cout << "Com retreat" << std::endl;
            state = 1;
        }
        if(state == 1 && health > .9 * max_health){
            state = 0;
        }

        if(state == 0){
            dest = attack_point;
        }
        else if(state == 1){
            dest = map.get_managers()[team].rally_point;
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

		update_path();
		find_target();

		Vector2f deliberate;

        if((p() - dest).len() < r()){
            deliberate = Vector2f(0, 0);
        }
        else{
            deliberate = traverse_path()*acc;
        }

		auto avoidance = avoid_obstacles();
		auto dv = deliberate+avoidance;

        if(selected){
            Vector2f dir;

            if(Input::is_key_pressed(SDLK_w)){
                dir += Vector2f(0, -1);
            }
            if(Input::is_key_pressed(SDLK_a)){
                dir += Vector2f(-1, 0);
            }
            if(Input::is_key_pressed(SDLK_s)){
                dir += Vector2f(0, 1);
            }
            if(Input::is_key_pressed(SDLK_d)){
                dir += Vector2f(1, 0);
            }

            if(dir.len() > .001){
                dir = dir.unit();
                dv = dir*acc;
            }
            else{
                dv = Vector2f(0, 0);
                set_v(v() * .99);
            }
        }

		if(dv.len() > acc){
			dv = (dv/dv.len())*acc;
		}

		auto nv = v() + dv;
		if(nv.len() > topspeed){
			nv = (nv/nv.len())*topspeed;
		}

        if(idle && !selected){
            set_v(Vector2f(0, 0));
        }
        else{
		    set_v(nv);
        }

		return true;
	}


};