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

    int id_count = 23;

    /*map <tuple<Mutations, int, int>, int> mut_hist;
    Network net = Network(&id_count);
    net.add_link(mut_hist);
    net.add_neuron(mut_hist);
    net.print_links();*/

    PopMan pmanager = PopMan(&id_count);
    pmanager.add();

    

    for (int i = 0; i < 3; i++)
    {
        pmanager.mut_hist.clear();
        pmanager.mutate();
    }

    pmanager.test();

    for (int i = 0; i < 138; i++)
    {
        pmanager.mut_hist.clear();
        pmanager.reproduce_and_crossover();

        cout << "\n" << "Generation" << i + 1 << "\n";
        cout << "population size: " << pmanager.population.size() << "\n";
        
        pmanager.test();
        cout << "highest_fitness: " << pmanager.population[0].fitness << "\n";
        cout << "links: " << pmanager.population[0].links.size() << "\n";
        cout << "neurons: " << pmanager.population[0].neurons.size() << "\n";
        int c = 0;
        for (auto &a : pmanager.population)
        {
            if (a.links.size() == 0 )
            {
                c += 1;
            }
        }
        cout << "min_n_links: " << c << "\n";
        
        
        /*if (pmanager.population[0].fitness > 1000)
        {
            break;
        }*/
    }
    cout << pmanager.population.size() << "\n";

    Grid grid = Grid(&id_count);
    grid.brain = pmanager.population[0];
    pmanager.population[0].print_links();

    // for (auto &network : pmanager.population)
    // {
    //     cout << network.neurons.size() << "neurons" << endl;
    //     if (network.links.size() > 0)
    //     {
    //         network.print_links();
    //     }
    // }

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(darkGreen);
        if (grid.running)
        {
            grid.update();
            //grid.brain.print_links();
            //cout << grid.brain.neurons[grid.brain.output_ids[0]].activation << " " << grid.brain.neurons[grid.brain.output_ids[1]].activation << " " << grid.brain.neurons[grid.brain.output_ids[2]].activation << " " << grid.brain.neurons[grid.brain.output_ids[3]].activation << endl;
            //cout << grid.brain.neurons[grid.brain.inputs_ids[Inputs::FRUIT_S]].activation;
            grid.draw();
        }
        else
        {
            grid = Grid(&id_count);
            grid.brain = pmanager.population[0];
        }
        
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
