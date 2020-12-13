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
}

void attack_group::render(SDL_Renderer* renderer){

}

bool attack_group::update(){
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