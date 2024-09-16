#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include "neuron.h"
#include "link.h"
#include <vector>
#include <map>
#include <algorithm> 

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

            neurons[45] = Neuron();
            neurons[46] = Neuron();
            neurons[47] = Neuron();
            neurons[48] = Neuron();
            neurons[49] = Neuron();
            Link l = Link();
            l.from_id = 45;
            l.to_id = 46;
            links.push_back(l);
            // l = Link();
            // l.from_id = 46;
            // l.to_id = 48;
            // links.push_back(l);
            l = Link();
            l.from_id = 48;
            l.to_id = 49;
            links.push_back(l);
            l.from_id = 48;
            l.to_id = 47;
            links.push_back(l);
        };

        void add_link()
        {
            //frind from and to make sure no repeats or opposites of links that already exist and no loops

            vector<int> ids;
            for (auto imap : neurons)
            {
                if (count(begin(output_ids), end(output_ids), imap.first) == 0) // no outputs for now
                {
                    ids.push_back(imap.first);
                }
            }

            int from_id = ids[GetRandomValue(0, ids.size() - 1)];

            ids.clear();
            for (auto imap : neurons)
            {
                if (count(begin(inputs_ids), end(inputs_ids), imap.first) == 0) // no inputs for now
                {
                    ids.push_back(imap.first);
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

            from_id = 47;
            to_id = 45;

            bool cyclic = false;
            check_cycle(from_id, to_id, &cyclic);

            if (cyclic)
            {
                cout << "AAAH";
                return;
            }

            Link link = Link();
            link.id = *id_count;
            link.from_id = from_id;
            link.to_id = to_id;
            (*id_count)++;
            links.push_back(link);
        }

        void check_cycle(int original, int current, bool* outer_var)
        {
            cout << current << original << endl;
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

        }
};