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
        map<tuple<Mutations, int, int>, int> mut_hist; // resets every generation
        int population_size = 150;
        int* id_count;

        PopMan(int* idc)
        {
            id_count = idc;
        }

        void add ()
        {
            for (int i = 0; i < population_size; i++)
            {
                population.push_back(Network(id_count));
            }
        }

        void mutate()
        {
            mut_hist.clear();
            //key is (type of mutation, from id, to id) val is innovation id
            //make it so it only does things that are possible
            for (auto& network : population)
            {
                network.mutate(mut_hist);
            }
        }

        
        void reproduce_and_crossover()
        {
            //shuffle order in which genomes are considered
            random_device rd{}; 
            auto rng = default_random_engine { rd() };
            shuffle(begin(population), end(population), rng);

            vector<vector<Network>> species_list; // the first of each species vector is the species representative
            //put each genome into species groups
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
                        if (has_link(species[0].links, link.id)) // if not in the other genomes link ids, must be disjoint or excess
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
                        if (has_link(network.links, link.id)) // if not in the other genomes link ids, must be disjoint or excess
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
                        if (has_link(network.links, link.id)) // if in the other genomes links, is matching
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
            //CROSSOVER
            //TODO The champion of each species with more than five networks was copied into the next generation unchanged

            float total_adjusted_fitness = 0.0; //sum of the adjusted fitnesses of the entire population
            //adjust fitness through fitness sharing, where networks in species with more genomes are decreased more
            for (auto &species : species_list)
            {
                 for (auto &network : species)
                 {
                    network.fitness /= species.size();
                    total_adjusted_fitness += network.fitness;
                 }
            }

            population.clear();

            for (auto species : species_list)
            {
                if (population.size() >= population_size)
                {
                    break;
                }

                float species_adjusted_fitnesses = 0.0;
                for (auto &network : species)
                {
                    species_adjusted_fitnesses += network.fitness;
                    //cout << network.fitness;
                }

                //make number of offspring from a species proportional to how good it is
                int n_offspring = ceil(species_adjusted_fitnesses / total_adjusted_fitness);
                //cout << n_offspring;
                for (int i = 0; i < n_offspring; i++)
                {
                    if (population.size() >= population_size)
                    {
                        break;
                    }

                    int id1 = GetRandomValue(0, species.size() - 1);
                    int id2 = GetRandomValue(0, species.size() - 1);;
                    if (id1 == id2) // if the same one is chosen to be crossedover with mutate instead into the next generation
                    {
                        species[id1].mutate(mut_hist); //TODO put mutation function within the network class
                        population.push_back(species[id1]);
                    }
                    else
                    {
                        Network offspring = Network(id_count);

                        //do matching, and disjoint and excess in the first genome
                        for (auto link : species[id1].links)
                        {
                            if (has_link(species[id2].links, link.id)) //matching link
                            {
                                Link new_link = Link();
                                new_link.disabled = false;
                                new_link.from_id = link.from_id;
                                new_link.to_id = link.to_id;
                                if (link.disabled || species[id2].links[get_index(species[id2].links, link.id)].disabled)
                                {
                                    if (GetRandomValue(1, 4) <= 3) // 3/4 chance if one of the genes is disabled, the offspring will be
                                    {
                                        new_link.disabled = true ;
                                    }
                                }
                                offspring.links.push_back(new_link);
                            }
                            else 
                            {
                                //always inherits all disjoint and excess genes from the fitter parent. If they're the same,each has a 50% chance of being in the offspring.
                                 if (species[id1].fitness > species[id2].fitness || species[id1].fitness == species[id2].fitness && GetRandomValue(1, 2) == 1)
                                 {
                                    offspring.links.push_back(link);
                                 }
                            }
                        }

                        // do disjoint and excess in the second genome
                        for (auto link : species[id2].links)
                        {
                            if (has_link(species[id2].links, link.id))
                            {
                                if (species[id2].fitness > species[id1].fitness || species[id1].fitness == species[id2].fitness && GetRandomValue(1, 2) == 1)
                                 {
                                    offspring.links.push_back(link);
                                 }
                            }
                        }
                    }
                }
            }
        }


        void test()
        {
            for (auto &network : population)
            {
                Grid grid = Grid(id_count);
                grid.brain = network;
                while (grid.running) //TODO make it so it cant run forever
                {
                    grid.update();
                }
                network.fitness = grid.brain.fitness;
            }

            sort(population.begin(), population.end(), [](Network a, Network b) {
                return a.fitness > b.fitness;
            });
        }

        int get_index(vector<Link>& links, int id)
        {
            for (int i=0; i < links.size(); i++)
            {
                if (links[i].id == id)
                {
                    return i;
                }
            }
            return -1;
        }

        bool has_link(vector<Link>& links, int id)
        {
            for (int i=0; i < links.size(); i++)
            {
                if (links[i].id == id)
                {
                    return true;
                }
            }
            return false;
        }
};

