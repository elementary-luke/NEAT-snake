#include "grid.h"
#include <raylib.h>
#include <iostream>
#include <raylib.h>
#include <raymath.h>

using namespace std;

Grid::Grid()
{
    for (int i = 0; i < 20; i++)
    {
        for (int j = 0; j < 20; j++)
        {
            data[i][j] = (i == 0 || j == 0 || i == 20 - 1 || j == 20 - 1) ? Object::WALL : Object::EMPTY;
        }
    }
    data[3][3] = Object::HEAD;
    spawn_fruit();
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
    Vector2 desire = Vector2{(float)IsKeyDown(KEY_D) - (float)IsKeyDown(KEY_A), (float)IsKeyDown(KEY_S) - (float)IsKeyDown(KEY_W)};
    if (snake.get_dir().x == 0.0)
    {
        if (desire.x != 0.0)
        {
            snake.set_dir(Vector2{desire.x, 0.0});
        }
    }
    else if (desire.y != 0.0)
    {
        snake.set_dir(Vector2{0.0, desire.y});
    }

    Object obj_infront = data[(int)snake.get_head_pos().x + (int)snake.get_dir().x][(int)snake.get_head_pos().y + (int)snake.get_dir().y];

    if (obj_infront == Object::EMPTY || obj_infront == Object::FRUIT)
    {
        data[(int)snake.get_head_pos().x][(int)snake.get_head_pos().y] = Object::EMPTY;
            data[(int)snake.get_head_pos().x + (int)snake.get_dir().x][(int)snake.get_head_pos().y + (int)snake.get_dir().y] = Object::HEAD;
            
            if (snake.get_len() > 1)
            {
                data[(int)snake.get_head_pos().x][(int)snake.get_head_pos().y] = Object::BODY;
                data[(int)snake.get_tailest_pos().x][(int)snake.get_tailest_pos().y] = Object::EMPTY;
            }
            snake.insert_segment(1, snake.get_head_pos());


            if (snake.get_growing())
            {
                snake.set_growing(false);
            }
            else
            {
                snake.remove_tailest();
            }


            snake.move_head_forwards();
    }
    if (obj_infront == Object::FRUIT)
    {
        snake.set_growing(true);
        spawn_fruit();
    }
}

void Grid::spawn_fruit()
{
    while (true)
    {
        int x = GetRandomValue(0, 20);
        int y = GetRandomValue(0, 20);
        if (data[x][y] == Object::EMPTY)
        {
            data[x][y] = Object::FRUIT;
            return;
        }
    }
}