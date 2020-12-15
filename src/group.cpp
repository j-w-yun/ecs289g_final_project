#pragma once

group::group(int c, Vector2f g, Vector2f rally, std::shared_ptr<MapLevel> m, int r): capacity(c), goal(g), rally_point(rally), map(m), recruiting(r){
    members = 0;
    for(int i = 0; i < c; i++){
        roles.push_back(i);
    }
}

// get role, -1 if not recruiting or full
int group::join(){
    if(members == capacity){
        return -1;
    }
    if(!recruiting){
        return -1;
    }

    int role = roles.back();
    roles.pop_back();
    members++;

    return role;
}

// leave group, returning role
void group::leave(int role){
    members--;
    roles.push_back(role);
}

void group::render(SDL_Renderer* renderer){

}

bool group::update(){
    return true;
}

// attack group
attack_group::attack_group(int c, Vector2f g, Vector2f rally, std::shared_ptr<MapLevel> m, float rp, int r, int rt) : group(c, g, rally, m, r), retreat_prop(rp), recruiting_time(rt){
    dest = rally_point;
    move_priority = 2;
}

bool attack_group::update(){
    if(marked){
        recruiting = 0;
        return true;
    }

    // recruit until timer or full, whichever is later
    if(!state){
        if(counter < recruiting_time){
            counter++;
        }
        else{
            if(members > (.95f)*capacity){
                dest = goal;
                state = 1;
                recruiting = false;
                std::cout << "Attack now" << std::endl;
            }
        }
    }
    
    // attack
    if(state == 1){
        // retreat
        if(members < retreat_prop*capacity){
            dest = rally_point;
            counter = 0;
            state = 0;
            recruiting = true;
            std::cout << "Retreat now" << std::endl;
        }
    }

    //if(!(rand() % 50)){
    //    std::cout << "\tstate: " << state << ", ctr " << counter << ", members " << members << ", cap " << capacity << ", dest " << dest << ", goal " << goal << ", rally " << rally_point << std::endl;
    //}

    return true;
}

// defense group
std::vector<Vector2f> fill_column(MapLevel& map, float x_pos, int units, float spacing){
    if(x_pos < 0 || x_pos >= map.get_width() * map.get_tile_width()){
        std::cout << "Early ret, x_pos is " << x_pos << "map world width: " << map.get_width() * map.get_tile_width() << std::endl;

        return {};
    }

    const auto& og = map.get_obgrid();
    std::vector<std::pair<float, float>> intervals;
    bool in_interval = 0;
    int x_ind = (int)x_pos/map.get_tile_width();

    // collect intervals
    for(int i = 0; i < (int)og[0].size(); i++){
        if(og[x_ind][i]){
            in_interval = 0;
            continue;
        }

        auto yval = map.to_world_space(std::make_pair(x_ind, i)).y();

        if(!in_interval){
            in_interval = true;

            intervals.push_back(std::make_pair(yval, yval));
        }
        else{
            intervals.back().second = yval;
        }
    }

    for(auto p : intervals){
        std::cout << "Interval: (" << p.first << ", " << p.second << ")" << std::endl;
    }

    std::cout << "Units to place: " << units << std::endl;

    std::vector<Vector2f> positions;
    int placed = 0;

    for(auto i : intervals){
        for(float f = i.first; f <= i.second; f += spacing){
            positions.push_back(Vector2f(x_pos, f));
            placed++;

            std::cout << "Placing " << Vector2f(x_pos, f) << std::endl;

            if(placed >= units){
                std::cout << "Ret1, positions size is " << positions.size() << std::endl;

                return positions;
            }
        }
    }

    std::cout << "Ret1" << std::endl;

    return positions;
}

defense_group::defense_group(int c, Vector2f g, Vector2f rally, std::shared_ptr<MapLevel> m, float s, int r) : group(c, g, rally, m, r), spacing(s) {
    // calculate placements

    auto tile = map->to_tile_space(g);
    if(!map->inbounds(map->get_obgrid(), tile)){
        recruiting = false;
        return;
    }

    std::cout << "In ctor" << std::endl;

    int ind = 0;

    while((int)positions.size() < capacity){

        float x_pos = g.x() + (ind%2?-1:1) * ind * spacing;

        auto col = fill_column(*map, x_pos, capacity - positions.size(), spacing);

        std::cout << "Column of size " << col.size() << ", positions size is " << positions.size() << std::endl;
        //exit(1);

        int to_push = (int)std::min(col.size(), capacity - positions.size());

        std::cout << "Push back " << to_push << std::endl;


        for(int i = 0; i < to_push; i++){
            positions.push_back(col[i]);
        }

        std::cout << "Positions size now " << positions.size() << std::endl;
    }

    std::cout << "Leaving" << std::endl;
}

bool defense_group::update(){
    if(marked){
        recruiting = 0;
        return true;
    }

    return true;
}

Vector2f defense_group::role_dest(int role){
    if(role < 0 || role >= (int)positions.size()){
        return Vector2f(0, 0);
    }
    return positions[role];
}

