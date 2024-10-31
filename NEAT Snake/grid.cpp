#include "grid.h"

using namespace std;

Grid::Grid(int* id_count)
{
    //initialise grid and make edges into wall
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            data[i][j] = (i == 0 || j == 0 || i == 20 - 1 || j == 20 - 1) ? Object::WALL : Object::EMPTY;
        }
    }

    spawn_fruit();

    //put head in a random position
    int x = GetRandomValue(2, 18);
    int y = GetRandomValue(2, 18);
    data[x][y] = Object::HEAD;
    segments.push_back(Vector2{ (float) x, (float) y });

    //snake faces a random direction
    int choice = GetRandomValue(1, 4);
    if (choice == 1)
    {
        dir = { 1.0, 0.0 };
    }
    else if (choice == 2)
    {
        dir = {-1.0, 0.0 };
    }
    else if (choice == 3)
    {
        dir = { 0.0, 1.0 };
    }
    else if (choice == 4)
    {
        dir = { 0.0, -1.0 };
    }
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

                default:
                    col = GRAY;
            }
            DrawRectangleRounded(Rectangle{x, y, 20.0f, 20.0f}, 0.2, 1, col);
            
        }
    }
}

//each step of the simulation
void Grid::update()
{
    set_input();
    brain.calc_output();
    Vector2 desire = brain.get_desire(); //Vector2{(float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A), (float)IsKeyDown(KEY_S) - (float)IsKeyDown(KEY_W)};

    //cout << desire.x << " " << desire.y << endl;

    //change the direction of the snake based on the press simulated
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

    //get the type of object that the snake will go into
    Object obj_infront = data[(int)segments[0].x + (int)dir.x][(int)segments[0].y + (int)dir.y];


    if (obj_infront == Object::EMPTY || obj_infront == Object::FRUIT)
    {
        data[(int)segments[0].x][(int)segments[0].y] = Object::EMPTY;
        data[(int)segments[0].x + (int)dir.x][(int)segments[0].y + (int)dir.y] = Object::HEAD;
        

        if (growing)
        {
            data[(int)segments[0].x][(int)segments[0].y] = Object::BODY;
            segments.insert(segments.begin() + 1, segments[0]);// put new body part where head was
            growing = false;
        }
        else if (segments.size() > 1)
        {
            data[(int)segments[0].x][(int)segments[0].y] = Object::BODY;
            data[(int)segments.back().x][(int)segments.back().y] = Object::EMPTY;
            segments.insert(segments.begin() + 1, segments[0]); // put new body part where head was
            segments.pop_back(); //remove last segment
        }
    
        segments[0] = Vector2Add(segments[0], dir); //move head forward
    }
    else
    {
        //snake bumps into itself or the wall
        //set the fitness based on the criteria and stop the simulation
        brain.fitness = (segments.size() - 1) * 1000 - 500.0;
        brain.fitness = max(brain.fitness, 0.0f);
        running = false;
    }

    if (obj_infront == Object::FRUIT)
    {
        steps_without_fruit = 0;
        growing = true;
        spawn_fruit();
    }
    else if (steps_without_fruit >= 100)
    {
        brain.fitness = 0.0;
        running = false;
    }
    steps_without_fruit += 1;
    steps_lived += 1;
}

//randomly place fruit somewhere in the grid
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

//set input layer of the network
void Grid::set_input()
{
    //look in 8 directions from the head
    int dirs [8][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}, {1, -1}, {1, 1}, {-1, 1}, {-1, -1}};

    //set the inputs for how far the wall is in each direction, making sure the value is normalised between 0 and 1
    for (int i = 0; i < 4; i++)
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

    //set the inputs for how far a body part is in each direction, making sure the value is normalised between 0 and 1.
    //If there isn't a body part in that direction, defaults to how far a wall is
    for (int i = 0; i < 4; i++)
    {
        int dist = 1;
        while (true)
        {
            if (data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::BODY || data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::WALL)
            {
                brain.neurons[brain.inputs_ids[Inputs::TAIL_N + i]].activation = (float) dist / 20.0;
                break;
            }
            dist++;
        }
    }

    //set distance to fruit
    float fdist = sqrt(pow(segments[0].x - fruit_pos.x, 2.0) + pow(segments[0].y - fruit_pos.y, 2.0));

    brain.neurons[brain.inputs_ids[Inputs::FRUIT_D]].activation = fdist / 30.0f;

    //boolean inputs for if there's a fruit in that direction
    for (int i = 0; i < 4; i++)
    {
        int dist = 1;
        while (true)
        {
            if (data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::FRUIT)
            {
                brain.neurons[brain.inputs_ids[Inputs::FRUIT_N + i]].activation = 1.0;
                break;
            }
            else if (data[(int)segments[0].x + dirs[i][0] * dist][(int)segments[0].y + dirs[i][1] * dist] == Object::WALL)
            {
                brain.neurons[brain.inputs_ids[Inputs::FRUIT_N + i]].activation = 0.0;
                break;
            }
            dist++;
        }
    }

    //input for how long the snake currently is
    brain.neurons[brain.inputs_ids[Inputs::SIZE]].activation = float(segments.size()) / 400.0f ;
    
    //boolean input for the direction the snake is currently travelling in
    brain.neurons[brain.inputs_ids[Inputs::DIR_N]].activation = (dir.y == -1) ? 1.0 : 0.0;
    brain.neurons[brain.inputs_ids[Inputs::DIR_S]].activation = (dir.y == 1) ? 1.0 : 0.0;
    brain.neurons[brain.inputs_ids[Inputs::DIR_E]].activation = (dir.x == 1) ? 1.0 : 0.0;
    brain.neurons[brain.inputs_ids[Inputs::DIR_W]].activation = (dir.x == -1) ? 1.0 : 0.0;

    //cout << brain.neurons[brain.inputs_ids[Inputs::DIR_N]].activation << endl;
    
}