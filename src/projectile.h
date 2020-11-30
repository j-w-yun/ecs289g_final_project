#pragma once

#include "Vector2f.h"

class MapLevel;

struct projectile{
    Vector2f p, v;
    int life;
    int team;
    MapLevel& map;

    projectile(Vector2f p, Vector2f v, int l, int t, MapLevel& m): p(p), v(v), life(l), map(m){

    }

    virtual void update();
};