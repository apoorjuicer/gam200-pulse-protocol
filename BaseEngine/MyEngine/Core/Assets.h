#ifndef ASSETS_H
#define ASSETS_H


// CHANGE THIS SO THAT IT IS ONLY COMPONENT, HERO AND MONSTERS ARE NOT COMPONENTS.
struct Hero {
    Vector2 pos;
    float width, height;
    float swordWidth, swordHeight;
    float swingProgress;
    bool isSwinging;
    int hp;
};

struct Monster {
    Vector2 pos;
    float width, height;
    bool isAlive;
};

struct AABB {
    Vector2 min;
    Vector2 max;
};


#endif