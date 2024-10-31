#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>

#include "network.h"
#include "neuron.h"

using namespace std;


class Link
{
    public:
        int from_id;
        int to_id;
        int id;
        float weight = 1.0;
        bool disabled = false;
};

