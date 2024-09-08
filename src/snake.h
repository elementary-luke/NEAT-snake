#pragma once
#include <vector>
#include <tuple>
#include <raylib.h>
#include <raymath.h>

using namespace std;


class Snake
{
    private:
        vector<Vector2> segments = {Vector2{3, 3}};
        Vector2 dir = Vector2{1.0, 0.0};
        bool growing = false;
        //brain

    public:
        Vector2 get_dir() {return dir;}
        void set_dir(Vector2 val) {dir = val;}
        Vector2 get_head_pos() {return segments[0];}
        Vector2 get_tailest_pos() {return segments[segments.size() - 1];}
        void set_segment(int index, Vector2 val) {segments[index] = val;}
        int get_len() {return segments.size();}
        void remove_tailest() {segments.pop_back();}
        void add_segment(Vector2 val) {segments.push_back(val);}
        void insert_segment(int index, Vector2 val) {segments.insert(segments.begin() + index, val);}
        void set_growing(bool val) {growing = val;}
        bool get_growing() {return growing;}
        void move_head_forwards()
        {
            segments[0] = Vector2Add(segments[0], dir);
        }
};