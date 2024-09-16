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
    REMOVE_LINK,
    REMOVE_NEURON
};

class Network
{
    public:
        map<int, Neuron> neurons;
        vector<Link> links;

        int inputs_ids[26];
        int output_ids[4];
        int* id_count;
        
    
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
        };

        void add_link()
        {
            //frind from and to make sure no repeats or opposites of links that already exist and no loops

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
                cout << "CYCLIC" <<endl;
                return;
            }

            Link link = Link();
            link.id = *id_count;
            link.from_id = from_id;
            link.to_id = to_id;
            link.weight = (float) GetRandomValue(-1000000, 1000000) / 1000000.0f;
            (*id_count)++;
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

            vector<int> neighbours;
            for (auto link : links)
            {
                if (link.from_id == current)
                {
                    check_cycle(original, link.to_id, outer_var);
                }
            }
        }

        void add_neuron()
        {
            if (links.size() == 0)
            {
                return;
            }
            int index = GetRandomValue(0, links.size() - 1);
            links[index].disabled = true; // disable old link
            
            int neuron_id = *id_count; //new neuron between
            neurons[*id_count] = Neuron();
            (*id_count) ++;
            
            Link link1 = Link(); //new link from old from to new neuron
            link1.from_id = links[index].from_id;
            link1.to_id = neuron_id;
            link1.id = *id_count;
            link1.weight = 1.0;
            (*id_count) ++;
            links.push_back(link1);

            Link link2 = Link(); //new link from new neuron to old to
            link2.from_id = neuron_id;
            link2.to_id = links[index].to_id;
            link2.id = *id_count;
            link2.weight =  links[index].weight;
            (*id_count) ++;
            links.push_back(link2);
        }

        void change_weight()
        {
            if (links.size() == 0)
            {
                return;
            }

            random_device rd{};
            mt19937 gen{rd()};

            std::normal_distribution<float> d{0.0, 0.2}; //TODO make it start at 0.4 and go to 0.1 or even less later
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



};