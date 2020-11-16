#pragma once

#include"v2f.h"
#include<vector>

struct rts_unit{
    v2f pos;
    //v2f vel;
    float radius;

    rts_unit() = default;

    rts_unit(v2f pos, float radius): pos(pos), radius(radius) {}

};