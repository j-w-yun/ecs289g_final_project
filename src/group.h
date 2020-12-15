#pragma once

class MapLevel;

struct group{
    int marked = 0;
    int capacity;
    int members;
    Vector2f goal, dest, rally_point;
    std::vector<int> roles;
    std::shared_ptr<MapLevel> map;
    int recruiting;
    int move_priority = 0;

    group() = default;

    group(int c, Vector2f g, Vector2f rally, std::shared_ptr<MapLevel> m, int r = 1);

    // get role, -1 if not recruiting or full
    int join();

    // leave group, returning role
    void leave(int role);

    virtual void render(SDL_Renderer* renderer);

    virtual bool update();

    // where unit with current role should go
    virtual Vector2f role_dest(int role) { return dest; }
};

// attack group
struct attack_group : group{
    float retreat_prop;
    int recruiting_time;
    int counter=0;
    // 0 for recruit, 1 for attack
    int state = 0;

    attack_group(int c, Vector2f g, Vector2f rally, std::shared_ptr<MapLevel> m, float rp = .3, int r = 1, int rt = 1000);

    virtual bool update();
};

// defense group
struct defense_group : group{
    float spacing;
    std::vector<Vector2f> positions;

    defense_group(int c, Vector2f g, Vector2f rally, std::shared_ptr<MapLevel> m, float s, int r = 1);

    virtual Vector2f role_dest(int role);

    virtual bool update();
};