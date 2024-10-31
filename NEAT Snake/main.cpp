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

    int id_count = 23; //globabl counter for innovation number. Used by both links and nodes

    PopMan pmanager = PopMan(&id_count);

    //add initial population
    pmanager.add();

    
    //mutate the inital population a couple times
    for (int i = 0; i < 3; i++)
    {
        pmanager.mut_hist.clear();
        pmanager.mutate();
    }

    //sort the original population by fitness
    pmanager.test();

    for (int i = 0; i < 40; i++)
    {
        pmanager.mut_hist.clear(); //the same innovation can only happen if the same mutation occurs in the same generation
        pmanager.reproduce_and_crossover();

        cout << "\n" << "Generation" << i + 1 << "\n";
        cout << "population size: " << pmanager.population.size() << "\n";
        
        pmanager.test();
        cout << "best_fitness: " << pmanager.population[0].fitness << "\n";
        cout << "best_nlinks: " << pmanager.population[0].links.size() << "\n";
        cout << "best_neurons: " << pmanager.population[0].neurons.size() << "\n";

        int nzero_links = 0;
        int total_nlinks = 0;
        int total_nneurons = 0;
        for (auto &net : pmanager.population)
        {
            total_nlinks += net.links.size();
            total_nneurons += net.neurons.size();
            if (net.links.size() == 0 )
            {
                nzero_links += 1;
            }
        }
        cout << "number_with_0_links:" << nzero_links << "\n";
        cout << "average_nlinks:" << total_nlinks / pmanager.population.size() << "\n";
        cout << "average_neurons:" << total_nlinks / pmanager.population.size() << "\n";
    }

    cout << pmanager.population.size() << "\n";

    Grid grid = Grid(&id_count);
    grid.brain = pmanager.population[0];
    pmanager.population[0].print_links();

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(darkGreen);

        if (grid.running)
        {
            grid.update();
            grid.draw();
            //grid.brain.print_links();
            //cout << grid.brain.neurons[grid.brain.output_ids[0]].activation << " " << grid.brain.neurons[grid.brain.output_ids[1]].activation << " " << grid.brain.neurons[grid.brain.output_ids[2]].activation << " " << grid.brain.neurons[grid.brain.output_ids[3]].activation << endl;
            //cout << grid.brain.neurons[grid.brain.inputs_ids[Inputs::FRUIT_S]].activation;
        }
        else // if the snake dies, reset the simulation with the same neural network
        {
            grid = Grid(&id_count);
            grid.brain = pmanager.population[0];
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
