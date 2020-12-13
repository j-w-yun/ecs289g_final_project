#include "rts_unit.h"

struct land_factory : rts_unit{
    int spawntime;
    int spawncharge = 0;
    Vector2f rally_point;
    Vector2f spawn_point;

    land_factory(Vector2f p, float r, int w, int h, int xt, int yt, int t, int hlt, int stime, MapLevel& mp): rts_unit(p, Vector2f(0, 0), r, w, h, xt, yt, t, hlt, 0, 0, mp), spawntime(stime){
        // set spawn point
        Vector2f d = Vector2f(wwidth/2, wheight/2);
        spawn_point = p + (d - p).unit() * r * 1.5;

        std::cout << "team " << team << " spawn point is " << spawn_point << std::endl;

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