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
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
    SDL_RenderFillRect(renderer, &box);
}

// return true if still alive
bool projectile::update(){
    p = p + v;

    life--;
    if(life <= 0){
        return false;
    }

    return true;
}