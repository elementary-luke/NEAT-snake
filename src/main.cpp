#include <raylib.h>
#include <vector>
#include <iostream>

#include "ball.h"
#include "grid.h"

using namespace std;

int main()
{
    Color darkGreen = Color{20, 160, 133, 255};

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "NEAT snake");
    SetTargetFPS(10);

    Grid grid = Grid();
    //grid.set_grid(3, 4, Object::FRUIT);
    //grid.print_grid();



    while (!WindowShouldClose())
    {
        BeginDrawing();

        grid.update();
        ClearBackground(darkGreen);
        grid.draw();
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
