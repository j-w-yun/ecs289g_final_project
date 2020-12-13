#pragma once

#include "group.h"

struct manager{
    int team;
    Vector2f rally_point;
    std::vector<std::shared_ptr<group>> groups;

    manager(int t, Vector2f rp): team(t), rally_point(rp) {

    }

    void update(){
        for(auto& g : groups){
            g->update();
        }
    }
};