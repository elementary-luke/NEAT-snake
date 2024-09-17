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

    Grid grid = Grid();

    int id_count = 31;
    Network net = Network(&id_count);

    net.add_link();
    net.add_neuron();
    

    for (auto link : net.links)
    {
        cout << link.from_id << "->" << link.to_id << " " << link.weight << endl;
    }

    net.top_sort();
    



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
