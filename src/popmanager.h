#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>

#include "grid.h"
#include "network.h"

class PopMan
{
    public:
        vector<Network> population;
        int* id_count;

        PopMan(int* idc)
        {
            id_count = idc;
        }

        void add ()
        {
            for (int i = 0; i < 1; i++)
            {
                population.push_back(Network(id_count));
            }
        }

        void mutate()
        {
            for (auto &network : population)
            {
                int mut_num = GetRandomValue(0, 8);
                switch (mut_num)
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        network.change_weight();
                        break;

                    case 5:
                        network.reset_weight();
                        break;

                    case 6:
                    case 7:
                        network.add_link();
                        break;

                    case 8:
                        network.add_neuron();
                        break;

                    // case 9:
                    //     network.toggle_link();
                    //     break;
                    
                }
            }
        }

        void test()
        {
            for (auto network : population)
            {
                Grid grid = Grid(id_count);
                grid.brain = network;
                while (grid.running)
                {
                    grid.update();
                }
            }

            sort(population.begin(), population.end(), [](Network a, Network b) {
                return a.fitness > b.fitness;
            });
        }

        void kill()
        {
            while (population.size() > 50)
            {
                population.pop_back();
            }
        }
        
        void repopulate()
        {
            for (auto network : population)
            {
                population.push_back(network);
            }
        }

};