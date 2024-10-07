#include "grid.h"
#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

using namespace std;

Grid::Grid(int* id_count)
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            data[i][j] = (i == 0 || j == 0 || i == 20 - 1 || j == 20 - 1) ? Object::WALL : Object::EMPTY;
        }
    }
    spawn_fruit();
    data[3][3] = Object::HEAD;
}

void Grid::set_grid(int x, int y, Object val)
{
    data[x][y] = val;
}

void Grid::print_grid()
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            cout << data[j][i] << " ";
        }
        cout << endl;
    }
}

void Grid::draw()
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            float x = i * 22.0f;
            float y = j * 22.0f;

            Color col;
            switch (data[i][j])
            {
                case Object::WALL:
                    col = GRAY;
                    break;

                case Object::FRUIT:
                    col = RED;
                    break;
                
                case Object::EMPTY:
                    col = WHITE;
                    break;

                case Object::HEAD:
                    col = GREEN;
                    break;
                
                case Object::BODY:
                    col = BLUE;
                    break;
            }
            DrawRectangleRounded(Rectangle{x, y, 20.0f, 20.0f}, 0.2, 1, col);
            
        }
    }
}

void Grid::update()
{
    set_input();
    brain.calc_output();
    Vector2 desire = brain.get_desire();//Vector2{(float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A), (float)IsKeyDown(KEY_S) - (float)IsKeyDown(KEY_W)};

    //cout << desire.x << " " << desire.y << endl;

    
    if (dir.x == 0.0)
    {
        if (desire.x != 0.0)
        {
            dir = Vector2{desire.x, 0.0};
        }
    }
    else if (desire.y != 0.0)
    {
        dir = Vector2{0.0, desire.y};
    }

    Object obj_infront = data[(int)segments[0].x + (int)dir.x][(int)segments[0].y + (int)dir.y];


    if (obj_infront == Object::EMPTY || obj_infront == Object::FRUIT)
    {
        data[(int)segments[0].x][(int)segments[0].y] = Object::EMPTY;
        data[(int)segments[0].x + (int)dir.x][(int)segments[0].y + (int)dir.y] = Object::HEAD;
        

        if (growing)
        {
            data[(int)segments[0].x][(int)segments[0].y] = Object::BODY;
            segments.insert(segments.begin() + 1, segments[0]);// put body where head was
            growing = false;
        }
        else if (segments.size() > 1)
        {
            data[(int)segments[0].x][(int)segments[0].y] = Object::BODY;
            data[(int)segments.back().x][(int)segments.back().y] = Object::EMPTY;
            segments.insert(segments.begin() + 1, segments[0]); // put body where head was
            segments.pop_back(); //remove last segment
        }
    
        segments[0] = Vector2Add(segments[0], dir); //move head forward
    }
    else
    {
        brain.fitness = segments.size();
        running = false;
    }

    if (obj_infront == Object::FRUIT)
    {
        growing = true;
        spawn_fruit();
    }
}

void Grid::spawn_fruit()
{
    //TODO check if the whole grid is filled
    while (true)
    {
        int x = GetRandomValue(0, 20);
        int y = GetRandomValue(0, 20);
        if (data[x][y] == Object::EMPTY)
        {
            data[x][y] = Object::FRUIT;
            fruit_pos = Vector2{(float) x, (float) y};
            return;
        }
    }
}

void Grid::set_input()
{
    int dirs [8][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}, {1, -1}, {1, 1}, {-1, 1}, {-1, -1}};

    for (int i = 0; i < 8; i++)
    {
        int dist = 0;
        while (true)
        {
            if (data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::WALL)
            {
                brain.neurons[brain.inputs_ids[i]].activation = (float) dist / 20.0;
                break;
            }
        dist++;
        }
    }

    for (int i = 0; i < 8; i++)
    {
        int dist = 1;
        while (true)
        {
            if (data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::BODY || data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::WALL)
            {
                brain.neurons[brain.inputs_ids[i + 8]].activation = (float) dist / 20.0;
                break;
            }
        dist++;
        }
    }

    float fdist = sqrt(pow(segments[0].x - fruit_pos.x, 2.0) + pow(segments[0].y - fruit_pos.y, 2.0));

    brain.neurons[brain.inputs_ids[Inputs::FRUIT_D]].activation = fdist;

    //rel would be the normalised vector so (1.0, 0) when fruit is directly east
    float rel_x = (fruit_pos.x - segments[0].x) / fdist; 
    float rel_y = (fruit_pos.y - segments[0].y) / fdist;

    // cout << brain.neurons[brain.inputs_ids[Inputs::WALL_W]].activation << endl;

    if (rel_y <= 0.0)
    {
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_N]].activation = -rel_y;
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_S]].activation = 0.0f;
    }
    else 
    {
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_S]].activation = rel_y;
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_N]].activation = 0.0f;
    }
    
    if (rel_x <= 0.0)
    {
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_W]].activation = -rel_x;
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_E]].activation = 0.0f;
    }
    else
    {
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_E]].activation = rel_x;
        brain.neurons[brain.inputs_ids[Inputs::FRUIT_W]].activation = 0.0f;
    }

    brain.neurons[brain.inputs_ids[Inputs::SIZE]].activation = segments.size();

    
}