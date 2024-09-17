#include <raylib.h>
#include <vector>
#include <iostream>
#include <random>

#include "neuron.h"
#include "grid.h"
#include "network.h"


using namespace std;

int main()
{
    Color darkGreen = Color{20, 160, 133, 255};

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "NEAT snake");
    SetTargetFPS(10);

    int id_count = 31;

    Grid grid = Grid(&id_count);

    while (!WindowShouldClose())
    {
        grid.set_input();
        BeginDrawing();

        grid.update();
        ClearBackground(darkGreen);
        grid.draw();
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
