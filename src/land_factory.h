#include "rts_unit.h"

struct land_factory : rts_unit{
    int spawntime;
    int spawncharge = 0;
    Vector2f rally_point;
    Vector2f spawn_point;
    int max_health;

    land_factory(Vector2f p, float r, int w, int h, int xt, int yt, int t, int hlt, int stime, MapLevel& mp): rts_unit(p, Vector2f(0, 0), r, w, h, xt, yt, t, hlt, 0, 0, mp), spawntime(stime){
        max_health = hlt;
        // set spawn point
        Vector2f d = Vector2f(wwidth/2, wheight/2);
        spawn_point = p + (d - p).unit() * r * 1.5;

        std::cout << "team " << team << " spawn point is " << spawn_point << std::endl;

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
#ifdef USE_SDL2_RENDERER
		// Fill
        float hp = (float)health/max_health;
		SDL_SetRenderDrawColor(renderer, hp*255*(team==1), hp*125*(team==0), hp*255*(team==0), 255);
		SDL_RenderFillRect(renderer, &box);

#else
		RenderingEngine::ogl_set_color(hp*255*(team==1), hp*125*(team==0), hp*255*(team==0), 255);
		RenderingEngine::ogl_fill_rect(box);

#endif
	}

    virtual bool update(float elapsed_time, bool calc){
        // spawning

        //std::cout << "team " << team << " charge is " << spawncharge  << "/" << spawntime << std::endl;

        spawncharge++;
        if(spawncharge >= spawntime){
            spawncharge = 0;

            auto rts_ptr = std::make_shared<rts_unit>(
			spawn_point,
			Vector2f(0, 0),
			3.0f,  // Radius
			wwidth,
			wheight,
			x_tiles,
			y_tiles,
			team, // team
			1, // health
			0.05f,  // Acceleration
			0.5f,  // Travel speed
			map
		    );

            // FIXME
            //if(!team){
            //    rts_ptr->selected = 1;
            //}

            map.add(rts_ptr);
        }

        return health > 0;
    }

};