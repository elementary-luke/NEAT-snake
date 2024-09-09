#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>

using namespace std;


enum Object 
{
    HEAD,
    BODY,
    WALL,
    FRUIT,
    EMPTY,
};

class Grid
{
    private:
        Object data[20][20];
        vector<Vector2> segments = {Vector2{3, 3}};
        Vector2 dir = Vector2{1.0, 0.0};
        bool growing = false;

    public:
        Grid();
        void set_grid(int x, int y, Object val);
        void print_grid();
        void draw();
        void update();
        void spawn_fruit();
        // void Draw();
};

