#include <raylib.h>
#include <vector>
#include <iostream>
#include <random>

#include "neuron.h"
#include "grid.h"
#include "network.h"
#include "popmanager.h"


using namespace std;

int main()
{
    Color darkGreen = Color{20, 160, 133, 255};

    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "NEAT snake");
    SetTargetFPS(10);

    int id_count = 31;

    PopMan pmanager = PopMan(&id_count);
    pmanager.add();

    for (int i = 0; i < 1; i++)
    {
        pmanager.mutate();
        pmanager.test();
        pmanager.reproduce_and_crossover();
    }
    pmanager.test(); // to sort them
    cout << pmanager.population.size();

    Grid grid = Grid(&id_count);
    grid.brain = pmanager.population[0];

    for (auto &network : pmanager.population)
    {
        cout << network.neurons.size() << "neurons" << endl;
        if (network.links.size() > 0)
        {
            network.print_links();
        }
    }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(darkGreen);
        grid.update();
        grid.draw();
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
