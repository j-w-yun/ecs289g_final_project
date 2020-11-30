#pragma once

#include "Vector2f.h"

class MapLevel;

struct projectile{
    Vector2f p, v;
    float r;
    int life;
    int team;
    int damage;
    MapLevel& map;
    size_t id;

    projectile(Vector2f p, Vector2f v, float r, int l, int t, int d, MapLevel& m): p(p), v(v), r(r), life(l), damage(d), map(m){

    }

    virtual void render(SDL_Renderer* renderer);

    virtual bool update();
};