#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <map>
#include <algorithm> 
#include <random>

#include "neuron.h"
#include "link.h"

using namespace std;

enum Mutations 
{
    ADD_LINK,
    ADD_NEURON,
    TOGGLE_LINK,
    EDIT_WEIGHT,
    RESET_WEIGHT,
    NONE,
};

enum Inputs
{
    // how far the closest wall is in each direction
    WALL_N,
    WALL_S,
    WALL_W,
    WALL_E,
    WALL_NE,
    WALL_SE,
    WALL_SW,
    WALL_NW,
    // how far the closest body part is in each direction
    TAIL_N,
    TAIL_S,
    TAIL_W,
    TAIL_E,
    TAIL_NE,
    TAIL_SE,
    TAIL_SW,
    TAIL_NW,

    // distance in each direction to the fruit
    FRUIT_N,
    FRUIT_S,
    FRUIT_W,
    FRUIT_E,

    //direction snake is travelling
    DIR_N,
    DIR_S,
    DIR_W,
    DIR_E,
    // size of the snake
    SIZE,

    //distance to fruit
    FRUIT_D
};

class Network
{
    public:
        map<int, Neuron> neurons;
        vector<Link> links;

        int inputs_ids[26];
        int output_ids[4];
        int* id_count;

        float fitness = 0;
        
    
        Network(int* idc)
        {
            id_count = idc;
            for (int i = 0; i < 26; i++)
            {
                neurons[i] = Neuron();
                inputs_ids[i] = i;
            }
            for (int i = 26; i < 30; i++)
            {
                neurons[i] = Neuron();
                output_ids[i - 26] = i;
            }
            
            neurons[40] = Neuron();
            neurons[41] = Neuron();
            neurons[42] = Neuron();
            neurons[43] = Neuron();
            neurons[44] = Neuron();
            neurons[45] = Neuron();
        };

        void add_link(map<tuple<Mutations, int, int>, int>& mut_hist)
        {
            //find from and to make sure no repeats or opposites of links that already exist and no loops
            vector<int> ids;
            for (auto [key, val] : neurons)
            {
                if (count(begin(output_ids), end(output_ids), key) == 0) // no outputs for now
                {
                    ids.push_back(key);
                }
            }

            int from_id = ids[GetRandomValue(0, ids.size() - 1)];

            ids.clear();
            for (auto [key, val] : neurons)
            {
                if (count(begin(inputs_ids), end(inputs_ids), key) == 0) // no inputs for now
                {
                    ids.push_back(key);
                }
            }

            vector<int> filtered_ids;

            for (auto id : ids)
            {
                bool allowed = true;
                for (auto link : links)
                {
                    if ((from_id == link.from_id && id == link.to_id) || (from_id == link.to_id && id == link.from_id))
                    {
                        allowed = false;
                    }
                }

                if (allowed)
                {
                    filtered_ids.push_back(id);
                }
            }

            if (filtered_ids.size() == 0)
            {
                return;
            }


            int to_id = filtered_ids[GetRandomValue(0, filtered_ids.size() - 1)];

            bool cyclic = false;
            check_cycle(from_id, to_id, &cyclic);

            if (cyclic)
            {
                //cout << "CYCLIC" <<endl;
                return;
            }

            tuple<Mutations, int, int> t = make_tuple(Mutations::ADD_LINK, from_id, to_id);

            Link link = Link();
            link.from_id = from_id;
            link.to_id = to_id;
            link.weight = (float) GetRandomValue(-1000000, 1000000) / 1000000.0f;
            
            if (mut_hist.count(t))
            {
                link.id = mut_hist[t];
            }
            else
            {
                mut_hist[t] = *id_count;
                link.id = *id_count;
                (*id_count)++;
            }

            links.push_back(link);
        }

        void check_cycle(int original, int current, bool* outer_var)
        {
            if (*outer_var == true)
            {
                return;
            }
            if (current == original)
            {
                *outer_var = true;
            }

            for (auto link : links)
            {
                if (link.from_id == current)
                {
                    check_cycle(original, link.to_id, outer_var);
                }
            }
        }

        void add_neuron(map<tuple<Mutations, int, int>, int>& mut_hist)
        {
            //neuron can only be created by splitting an enabled link
            vector<int> enabled_link_indices;
            for (int i = 0; i < links.size(); i++)
            {
                if (!links[i].disabled)
                {
                    enabled_link_indices.push_back(i);
                }
            }

            if (enabled_link_indices.size() == 0)
            {
                return;
            }
            
            int index = enabled_link_indices[GetRandomValue(0, enabled_link_indices.size() - 1)];
            links[index].disabled = true; // disable old link

            tuple<Mutations, int, int> t = make_tuple(Mutations::ADD_NEURON, links[index].from_id, links[index].to_id);
            
            if (mut_hist.count(t))
            {
                int neuron_id = mut_hist[t];//new neuron between
                neurons[neuron_id] = Neuron();

                Link link1 = Link(); //new link from old from to new neuron
                link1.from_id = links[index].from_id;
                link1.to_id = neuron_id;
                link1.id = neuron_id + 1;
                link1.weight = 1.0;
                links.push_back(link1);

                Link link2 = Link(); //new link from new neuron to old to
                link2.from_id = neuron_id;
                link2.to_id = links[index].to_id;
                link2.weight = links[index].weight;
                link2.id = neuron_id + 2;
                links.push_back(link2);
            }
            else
            {
                int neuron_id = *id_count; //new neuron between
                neurons[neuron_id] = Neuron();
                (*id_count) ++;

                mut_hist[t] = neuron_id;

                
                Link link1 = Link(); //new link from old from to new neuron
                link1.from_id = links[index].from_id;
                link1.to_id = neuron_id;
                link1.weight = 1.0;
                link1.id = *id_count;
                (*id_count) ++;
                links.push_back(link1);
                
                

                Link link2 = Link(); //new link from new neuron to old to
                link2.from_id = neuron_id;
                link2.to_id = links[index].to_id;
                link2.weight = links[index].weight;
                link2.id = *id_count;
                (*id_count) ++;
                links.push_back(link2);
            }
        }

        void change_weight()
        {
            if (links.size() == 0)
            {
                return;
            }

            random_device rd{};
            mt19937 gen{rd()};

            std::normal_distribution<float> d{0.0, 0.2}; //TODO make sigma start at 0.4 and go to 0.1 or even less later
            links[GetRandomValue(0, links.size() - 1)].weight += d(gen);
        }

        void reset_weight()
        {
            if (links.size() == 0)
            {
                return;
            }

            links[GetRandomValue(0, links.size() - 1)].weight =  (float) GetRandomValue(-1000000, 1000000) / 1000000.0f;
        }

        void toggle_link()
        {
            if (links.size() == 0)
            {
                return;
            }

            Link link = links[GetRandomValue(0, links.size() - 1)];
            link.disabled = !link.disabled;
        }


        void calc_output() // do a topological sort
        {
            //TODO MAKE IT SO IT ONLY LOOKS FOR ONES THAT LINK TO THE END
            vector<int> stack; // end order in which to visit
            map<int, vector<int>> neighbours; // key is id of neuron, value is neurons you can go to from there
            map<int, bool> visited; // whether a neuron of id int has been visited
            for (auto [id, _] : neurons)
            {
                visited[id] = false;
                neighbours[id] = {};
            }

            for (auto link : links)
            {
                neighbours[link.from_id].push_back(link.to_id);
            }

            vector<pair<int, int>> pairs;
            for (auto [key, val] : neighbours)
            {
                pairs.push_back({key, val.size()});
            }

            sort(pairs.begin(), pairs.end(), [](auto& a, auto& b) { 
                return a.second < b.second; 
            }); 


            vector<int> to_visit; // locations in order of least neighbours to most

            for (auto [first, _] : pairs)
            {
                to_visit.push_back(first);
            }
            

            for (auto id : to_visit)
            {
                if (visited[id])
                {
                    continue;
                }
                dfs(stack, neighbours, visited, id);
            }

            //compute activation of each neuron based on previous neurons. Order is calculated stack.
            for (auto id : stack)
            {
                //cout << id << endl;

                if (count(begin(inputs_ids), end(inputs_ids), id) == 1)
                {
                    continue;
                }

                float sum = 0.0;
                for (auto link : links)
                {
                    if (link.to_id == id && !link.disabled)
                    {
                        sum += neurons[link.from_id].activation * link.weight;
                    }
                }
                neurons[id].activation = sigmoid(sum);
            }
        }

        void dfs(vector<int>& stack, map<int, vector<int>>& neighbours, map<int, bool>& visited, int current)
        {
            int j =  neighbours.size();
            for (auto neighbour_id : neighbours[current])
            {
                if (visited[neighbour_id])
                {
                    continue;
                }
                dfs(stack, neighbours, visited, neighbour_id);
            }
            stack.push_back(current);
            visited[current] = true;
        }
        
        float sigmoid(float val)
        {
            return 1.0f / (1+ exp(-val));
        }

        void print_links()
        {
            for (auto link : links)
            {
                cout << link.from_id << "->" << link.to_id << " " << link.weight << endl;
            }
        }

        Vector2 get_desire()
        {
            float highest_val = -100000.0f;
            int highest_i = -1;
            for (int i = 0; i < 4; i++)
            {
                int id = output_ids[i];
                if (neurons[id].activation > highest_val)
                {
                    highest_val = neurons[id].activation;
                    highest_i = i;
                }
            }
            if (neurons[output_ids[0]].activation == neurons[output_ids[1]].activation && neurons[output_ids[1]].activation == neurons[output_ids[2]].activation && neurons[output_ids[2]].activation == neurons[output_ids[3]].activation)
            {
                return Vector2{0.0, 0.0};
            }
            else if (highest_i == 0)
            {
                return Vector2 {0.0, -1.0};
            }
            else if (highest_i == 1)
            {
                return Vector2 {0.0, 1.0};
            }
            else if (highest_i == 2)
            {
                return Vector2 {-1.0, 0.0};
            }
            else if (highest_i == 3)
            {
                return Vector2 {1.0, 0.0};
            }

            return Vector2{0.0, 0.0};
        }

        void mutate(map<tuple<Mutations, int, int>, int>& mut_hist)
        {
            map<Mutations, int> chances;
            //chances[Mutations::ADD_LINK] = 100;
            chances[Mutations::ADD_LINK] = 5;
            chances[Mutations::EDIT_WEIGHT] = 72;
            chances[Mutations::RESET_WEIGHT] = 8;
            chances[Mutations::TOGGLE_LINK] = 1;
            chances[Mutations::NONE] = 11;

            int dice = GetRandomValue(1, 100);

            for (auto [k, v] : chances)
            {
                dice -= v;
                if (dice <= 0)
                {
                    switch (k)
                    {
                        case Mutations::ADD_NEURON:
                            add_neuron(mut_hist);
                            break;
                        case Mutations::ADD_LINK:
                            add_link(mut_hist);
                            break;
                        case Mutations::EDIT_WEIGHT:
                            change_weight();
                            break;
                        case Mutations::RESET_WEIGHT:
                            reset_weight();
                            break;
                        case Mutations::TOGGLE_LINK:
                            toggle_link();
                            break;
                        case Mutations::NONE:
                            break;
                    }
                }
            }

        }
};