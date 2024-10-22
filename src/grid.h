#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>

#include "network.h"


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
        vector<Vector2> segments = {};
        Vector2 dir = Vector2{1.0, 0.0};
        bool growing = false;
        int* id_count;
        Vector2 fruit_pos;
        

    public:
        Grid(int* id_count);
        bool running = true;
        Network brain = Network(id_count);
        int steps_without_fruit = 0;
        int steps_lived = 0;
        void set_grid(int x, int y, Object val);
        void print_grid();
        void draw();
        void update();
        void spawn_fruit();
        void set_input();
        // void Draw();
        
};

