#include "projectile.h"

void projectile::render(SDL_Renderer* renderer){
    Vector2f sp1 = RenderingEngine::world_to_screen(Vector2f((int)(p.x() - r), (int)(p.y() - r)));
    Vector2f sp2 = RenderingEngine::world_to_screen(Vector2f((int)(p.x() + r), (int)(p.y() + r)));
    SDL_Rect box = {
        (int)(sp1.x()),
        (int)(sp1.y()),
        (int)(sp2.x()-sp1.x()),
        (int)(sp2.y()-sp1.y())
    };
    // Fill
#ifdef USE_SDL2_RENDERER
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_RenderFillRect(renderer, &box);
#else
    RenderingEngine::ogl_set_color(255, 0, 255, 255);
    RenderingEngine::ogl_fill_rect(box);
#endif
}

// return true if still alive
bool projectile::update(){
    auto np = p + v;
    
    // check for collisions
    auto tiles = bresenham(map.to_tile_space(p), map.to_tile_space(np));

    int closest_id = -1;
    float closest_dist = std::numeric_limits<float>::infinity();

    for(auto& tile : tiles){
        if(!map.inbounds(map.get_obgrid(), tile)) continue;

        for(int tm = 0; tm < map.get_teams(); tm++){
            if(tm == team) continue;

            for(auto id : map.get_unitgrid()[tm][tile.first][tile.second]){
                auto& unit = map.get_units()[id];

                auto comps = components(v, unit->p() - p);

                // hit
                if(comps.y() > 0 && comps.y() < v.len() && std::abs(comps.x()) < (r + unit->r())){
                    if(comps.y() < closest_dist){
                        closest_dist = comps.y();
                        closest_id = id;
                    }
                }
            }
        }
    }

    if(closest_id != -1){
        (map.get_units()[closest_id]->health)--;
    }
    
    p = np;

    life--;
    if(life <= 0){
        return false;
    }

    return true;
}