#pragma once

#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <vector>

using namespace std;


class Link
{
    public:
        int from_id;
        int to_id;
        int id;
        float weight;
};

