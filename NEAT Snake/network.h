#pragma once

#include <raylib.h>
#include <iostream>
#include <raymath.h>
#include <vector>
#include <map>
#include <unordered_set>
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
    //WALL_NE,
    //WALL_SE,
    //WALL_SW,
    //WALL_NW,
    // how far the closest body part is in each direction
    TAIL_N,
    TAIL_S,
    TAIL_W,
    TAIL_E,
    //TAIL_NE,
    //TAIL_SE,
    //TAIL_SW,
    //TAIL_NW,

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
    map<int, Neuron> neurons; //TODO MAKE UNORDERED
    vector<Link> links;

    int inputs_ids[18];
    int output_ids[4];
    int* id_count;

    float fitness = 0;

    string neuron_to_name[22] = { "WALL_N", "WALL_S", "WALL_W", "WALL_E", "TAIL_N", "TAIL_S", "TAIL_W", "TAIL_E", "FRUIT_N", "FRUIT_S", "FRUIT_W", "FRUIT_E", "DIR_N", "DIR_S", "DIR_W", "DIR_E", "SIZE", "FRUIT_D", "MOVE_UP", "MOVE_DOWN", "MOVE_LEFT", "MOVE_RIGHT" };


    Network(int* idc);

    void add_link(map<tuple<Mutations, int, int>, int>& mut_hist);
    bool cyclical(int original, int current);
    void add_neuron(map<tuple<Mutations, int, int>, int>& mut_hist);
    void change_weight();
    void reset_weight();
    void toggle_link();
    void calc_output(); // do a topological sort
    void dfs(vector<int>& stack, map<int, vector<int>>& neighbours, map<int, bool>& visited, int current);
    float sigmoid(float val);
    void print_links();
    Vector2 get_desire();
    void mutate(map<tuple<Mutations, int, int>, int>& mut_hist);
};