#include "popmanager.h"
#include "grid.h"
#include "network.h"

PopMan::PopMan(int* idc)
{
    id_count = idc;
}

void PopMan::add()
{
    for (int i = 0; i < population_size; i++)
    {
        population.push_back(Network(id_count));
    }
}

void PopMan::mutate()
{
    mut_hist.clear();
    //key is (type of mutation, from id, to id) val is innovation id
    //make it so it only does things that are possible
    for (auto& network : population)
    {
        network.mutate(mut_hist);
    }
}


void PopMan::reproduce_and_crossover()
{
    //shuffle order in which genomes are considered
    random_device rd{};
    auto rng = default_random_engine{ rd() };
    shuffle(begin(population), end(population), rng);

    vector<vector<Network>> species_list; // the first of each species vector is the species representative

    //put each genome into species groups
    for (auto network : population)
    {
        bool need_new_species = true;
        shuffle(begin(species_list), end(species_list), rng);
        for (auto& species : species_list)
        {
            int n_excess = 0;
            int n_disjoint = 0;
            float avgwdif = 0.0;

            float coefE = 1.0;
            float coefD = 1.0;
            float coefW = 0.4;

            float delta_threshold = 2.0;

            pair<int, int> network_range = make_pair(INT_MAX, -1); // (lowest, highest)
            for (auto& link : network.links)
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


            pair<int, int> sr_range = make_pair(INT_MAX, -1); // speciec rep (lowest, highest)
            for (auto& link : species[0].links)
            {
                if (link.id < sr_range.first)
                {
                    sr_range.first = link.id;
                }
                if (link.id > sr_range.second)
                {
                    sr_range.second = link.id;
                }
            }


            pair<int, int> match_range = make_pair(max(network_range.first, sr_range.first), min(network_range.second, sr_range.second));

            for (auto& link : network.links)
            {
                if (!has_link(species[0].links, link.id)) // if not in the other genomes link ids, must be disjoint or excess
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

            for (auto& link : species[0].links)
            {
                if (!has_link(network.links, link.id)) // if not in the other genomes link ids, must be disjoint or excess
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
            for (auto& link : species[0].links)
            {
                if (has_link(network.links, link.id)) // if in the other genomes links, is matching
                {
                    n_matching_links += 1;
                    float network_link_weight = network.links[get_index(network.links, link.id)].weight;
                    sum_w_dif += abs(link.weight - network_link_weight);
                }
            }

            avgwdif = (n_matching_links > 0) ? sum_w_dif / (float)n_matching_links : 0.0; //make sure if no matching, weight difference doenst come into the equation for delta

            int N = max(network.links.size(), species[0].links.size());

            if (N < 20) // if both genomes less than 20 genes, n = 1
            {
                N = 1;
            }

            float delta = (coefE * (float)n_excess) / (float)N + (coefD * (float)n_disjoint) / (float)N + coefW * avgwdif;
            /*if (delta != 0.0 && delta != 1.0)
            {
                cout << "delta: "<< delta <<  "\n";
            }*/

            //cout << " " << n_excess << " " << n_disjoint << " " << sum_w_dif << " " << delta << "\n";
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
            vector<Network> vec = { network };
            species_list.push_back(vec);
        }
    }
    cout << "species size!:" << species_list.size() << "\n";
    cout << "species[0] size!:" << species_list[0].size() << "\n";


    //CROSSOVER
    shuffle(begin(species_list), end(species_list), rng);

    float total_adjusted_fitness = 0.0; //sum of the adjusted fitnesses of the entire population

    //adjust fitness through fitness sharing, where networks in species with more genomes are penalised more
    for (auto& species : species_list)
    {
        for (auto& network : species)
        {
            network.fitness /= (float)species.size();
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

        sort(species.begin(), species.end(), [](Network a, Network b) {
            return a.fitness > b.fitness;
            });

        float species_adjusted_fitnesses = 0.0;
        for (auto& network : species)
        {
            species_adjusted_fitnesses += network.fitness;
            //cout << network.fitness;
        }

        //make number of offspring from a species proportional to how good it is
        int n_offspring = (int)round(species_adjusted_fitnesses / total_adjusted_fitness * population_size);
        //cout << "size" << species.size() << " n_offspring: " << n_offspring;
        for (int i = 0; i < n_offspring; i++)
        {
            if (population.size() >= population_size)
            {
                break;
            }

            int id1 = GetRandomValue(0, species.size() - 1);
            int id2 = GetRandomValue(0, species.size() - 1);;

            if (i == 0 && species.size() >= 5)
            {
                //cout << "DDDD: " << species[0].fitness * species.size() << "\n";
                population.push_back(species[0]);
            }
            else if (id1 == id2 || GetRandomValue(1, 5) == 5) // if the same one is chosen to be crossedover with mutate instead into the next generation or just by random chance
            {
                species[id1].mutate(mut_hist);
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
                        new_link.id = link.id;
                        new_link.weight = (GetRandomValue(1, 2) == 1) ? link.weight : species[id2].links[get_index(species[id2].links, link.id)].weight; //inherit weight randomly from 1 of the parents
                        if (link.disabled || species[id2].links[get_index(species[id2].links, link.id)].disabled)
                        {
                            if (GetRandomValue(1, 4) <= 3) // 3/4 chance if one of the genes is disabled, the offspring will be
                            {
                                new_link.disabled = true;
                            }
                        }
                        offspring.links.push_back(new_link);
                    }
                    else
                    {
                        //always inherits all disjoint and excess genes from the fitter parent. If they're the same,each has a 50% chance of being in the offspring.
                        if (species[id1].fitness >= species[id2].fitness)
                        {
                            offspring.links.push_back(link);
                        }
                    }
                }

                // do disjoint and excess in the second genome
                for (auto link : species[id2].links)
                {
                    if (!has_link(species[id1].links, link.id))
                    {
                        if (species[id2].fitness >= species[id1].fitness)
                        {
                            offspring.links.push_back(link);
                        }
                    }
                }

                for (auto& link : offspring.links)
                {
                    if (offspring.neurons.count(link.from_id) == 0)
                    {
                        offspring.neurons[link.from_id] = Neuron();
                    }
                    if (offspring.neurons.count(link.to_id) == 0)
                    {
                        offspring.neurons[link.to_id] = Neuron();
                    }
                }

                population.push_back(offspring);
                //cout << species[id1].links.size() << " " << species[id2].links.size() << " " << offspring.links.size() << "\n";
            }
        }
    }
}

//Simulate the snake many times and take an average for it's final fitness
void PopMan::test()
{
    for (auto& network : population)
    {
        int runs = 15;
        float total = 0.0;
        for (int i = 0; i < runs; i++)
        {
            Grid grid = Grid(id_count);
            grid.brain = network;
            while (grid.running)
            {
                grid.update();
            }
            total += grid.brain.fitness;
        }

        network.fitness = total / (float)runs;
        /*if (total / (float)runs > 500.0)
        {
            cout << "JACKPOT!!!"  << (total / (float)runs);
        }*/
    }

    sort(population.begin(), population.end(), [](Network a, Network b) {
        return a.fitness > b.fitness;
        });
}

int PopMan::get_index(vector<Link>& links, int id)
{
    for (int i = 0; i < links.size(); i++)
    {
        if (links[i].id == id)
        {
            return i;
        }
    }
    return -1;
}

bool PopMan::has_link(vector<Link>& links, int id)
{
    for (int i = 0; i < links.size(); i++)
    {
        if (links[i].id == id)
        {
            return true;
        }
    }
    return false;
}