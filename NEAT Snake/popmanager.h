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
        map<tuple<Mutations, int, int>, int> mut_hist; // resets every generation TODO make unordered
        int population_size = 150;
        int* id_count;

        PopMan(int* idc);
        void add ();
        void mutate();
        void reproduce_and_crossover();
        void test();
        int get_index(vector<Link>& links, int id);
        bool has_link(vector<Link>& links, int id);
};

