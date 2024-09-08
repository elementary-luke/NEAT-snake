#pragma once
#include "snake.h"


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
        Snake snake = Snake();

    public:
        Grid();
        void set_grid(int x, int y, Object val);
        void print_grid();
        void draw();
        void update();
        void spawn_fruit();
        // void Draw();
};

