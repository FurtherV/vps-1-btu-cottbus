#include <sys/types.h>
#include <vector>
#include <string>
#include <LocalBoard.h>

void LocalBoard::setPos(size_t x, size_t y, enum life_status_t status)
{
    while (x < 0)
    {
        x += width;
    }
    while (y < 0)
    {
        y += height;
    }
    LocalBoard::setPosRaw(x % width, y % height, status);
}
enum life_status_t LocalBoard::getPos(size_t x, size_t y)
{
    while (x < 0)
    {
        x += width;
    }
    while (y < 0)
    {
        y += height;
    }
    return LocalBoard::getPosRaw(x % width, y % height);
}
void LocalBoard::step()
{
    std::vector<enum life_status_t> newField;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            int neighbourCount = 0;
            enum life_status_t status = getPos(x, y);
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    if(dx != 0 && dy != 0){
                        if (getPos(x + dx, y + dy) == life_status_t::alive)
                        {
                            neighbourCount++;
                        }
                    }
                }
            }
            if(status == life_status_t::dead && neighbourCount == 3)
            {
                newField.push_back(life_status_t::alive);
            }
            else if(status == life_status_t::alive && neighbourCount >= 2 && neighbourCount <= 3)
            {
                newField.push_back(life_status_t::alive);
            }else{
                newField.push_back(life_status_t::dead);
            }
        }
    }
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            setPosRaw(x, y, newField[y * width + x]);
        }
    }
}

void LocalBoard::setPosRaw(size_t x, size_t y, enum life_status_t status)
{
    field[y * width + x] = status;
}
enum life_status_t LocalBoard::getPosRaw(size_t x, size_t y)
{
    return field[y * width + x];
}
