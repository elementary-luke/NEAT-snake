#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <map>
#include <tuple>

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
            for (int i = 0; i < 150; i++)
            {
                population.push_back(Network(id_count));
            }
        }

        void mutate()
        {
            map<tuple<Mutations, int, int>, int> mut_hist; //key is (type of mutation, from id, to id) val is innovation id
            //make it so it only does things that are possible
            for (auto& network : population)
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
                        network.add_link(mut_hist);
                        break;

                    case 8:
                        network.add_neuron(mut_hist);
                        break;

                    case 9:
                        network.toggle_link();
                        break;
                    
                }
            }
        }

        
        void crossover()
        {
            //shuffle order in which genomes are considered
            random_device rd{}; 
            auto rng = default_random_engine { rd() };
            shuffle(begin(population), end(population), rng);

            //put each genome into species groups
            vector<vector<Network>> species_list; // the first of each species vector is the species representative
            for (auto network : population)
            {
                bool need_new_species = true;
                for (auto &species : species_list)
                {
                    int n_excess = 0;
                    int n_disjoint = 0;
                    float avgwdif = 0.0;
                    
                    float coefE = 1.0;
                    float coefD = 1.0;
                    float coefW = 0.4;

                    float delta_threshold = 3.0;

                    pair<int, int> network_range = make_pair(100000, -1); // (lowest, highest)
                    for (auto &link : network.links)
                    {
                        if (link.id < network_range.first)
                        {
                            network_range.first = link.id;
                        }
                        if (link.id > network_range.second)
                        {
                            network_range.second = link.id;
                        }
                    }

                    pair<int, int> sr_range = make_pair(100000, -1); // speciec rep (lowest, highest)
                    for (auto &link : species[0].links)
                    {
                        if (link.id < network_range.first)
                        {
                            network_range.first = link.id;
                        }
                        if (link.id > network_range.second)
                        {
                            network_range.second = link.id;
                        }
                    }
                    
                    pair<int, int> match_range = make_pair(max(network_range.first, sr_range.first), max(network_range.second, sr_range.second));

                    for (auto &link : network.links)
                    {
                        if (find(species[0].links.begin(), species[0].links.end(), link.id) == species[0].links.end()) // if not in the other genomes link ids, must be disjoint or excess
                        {
                            if (link.id >= match_range.first && link.id <= match_range.second) // if in the match range disjoint else excess
                            {
                                n_disjoint += 1;
                            }
                            else
                            {
                                n_excess += 1;
                            }
                        }
                    }

                    for (auto &link : species[0].links)
                    {
                        if (find(network.links.begin(), network.links.end(), link.id) == network.links.end()) // if not in the other genomes link ids, must be disjoint or excess
                        {
                            if (link.id >= match_range.first && link.id <= match_range.second) // if in the match range disjoint else excess
                            {
                                n_disjoint += 1;
                            }
                            else
                            {
                                n_excess += 1;
                            }
                        }
                    }
                    
                    // only need to do from the perspective of one vector as the other will have the same matching genes
                    int n_matching_links = 0;
                    float sum_w_dif = 0;
                    for (auto &link : species[0].links)
                    {
                        if (find(network.links.begin(), network.links.end(), link.id) != network.links.end()) // if in the other genomes links, is matching
                        {
                            n_matching_links += 1;
                            float network_link_weight = network.links[get_index(network.links, link.id)].weight;
                            sum_w_dif += abs(link.weight - network_link_weight);
                        }
                    }
                    avgwdif = sum_w_dif / n_matching_links;

                    int N = max(network.links.size(), species[0].links.size());

                    if (N < 20) // if both genomes less than 20 genes, n = 1
                    {
                        N = 1;
                    }

                    float delta = (coefE * (float)n_excess) / (float)N + (coefD * (float)n_disjoint) / (float)N + coefW * avgwdif;
                    

                    if (delta <= delta_threshold)
                    {
                        species.push_back(network);
                        need_new_species = false;
                        break;
                    }
                }

                //only runs if the network doesn't fit into any of the current species
                if (need_new_species)
                {
                    vector<Network> vec = {network};
                    species_list.push_back(vec);
                }
            }
        }


        void test()
        {
            for (auto &network : population)
            {
                Grid grid = Grid(id_count);
                grid.brain = network;
                while (grid.running) //make it so it cant run forever
                {
                    grid.update();
                }
            }

            sort(population.begin(), population.end(), [](Network a, Network b) {
                return a.fitness > b.fitness;
            });
        }
};

int get_index(vector<Link>& vec, int val)
{
    if (find(vec.begin(), vec.end(), val) != vec.end()) 
    {
        return find(vec.begin(), vec.end(), val) - vec.begin();
    }
    else {
        return -1;
    }
}