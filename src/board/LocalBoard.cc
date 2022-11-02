#include <sys/types.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "board/LocalBoard.h"

LocalBoard::LocalBoard(int width, int height) : Board(width, height)
{
}

LocalBoard::~LocalBoard()
{
}

void LocalBoard::setPos(int x, int y, enum life_status_t status)
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

enum life_status_t LocalBoard::getPos(int x, int y)
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
                    if (dx != 0 && dy != 0)
                    {
                        if (getPos(x + dx, y + dy) == life_status_t::alive)
                        {
                            neighbourCount++;
                        }
                    }
                }
            }
            if (status == life_status_t::dead && neighbourCount == 3)
            {
                newField.push_back(life_status_t::alive);
            }
            else if (status == life_status_t::alive && neighbourCount >= 2 && neighbourCount <= 3)
            {
                newField.push_back(life_status_t::alive);
            }
            else
            {
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

void LocalBoard::setPosRaw(int x, int y, enum life_status_t status)
{
    field[y * width + x] = status;
}

enum life_status_t LocalBoard::getPosRaw(int x, int y)
{
    return field[y * width + x];
}

bool LocalBoard::exportAll(std::string destFileName)
{
    std::ofstream OutBoardFile(destFileName);

    OutBoardFile << "x = " << this->width << ", y = " << this->height << std::endl;

    for (int y = 0; y < this->height; ++y)
    {
        for (int x = 0; x < this->width; ++x)
        {
            if (this->getPos(x, y) == life_status_t::alive)
                OutBoardFile << "o";
            else
                OutBoardFile << "b";
        }
        OutBoardFile << std::endl;
    }

    OutBoardFile.close();

    return true;
}

bool LocalBoard::importAll(std::string sourceFileName)
{
    if (sourceFileName == "RANDOM")
    {
        srand(time(nullptr));

        this->width = 1 + rand() % 64;
        this->height = 1 + rand() % 64;

        for (int y = 0; y < this->height; ++y)
        {
            for (int x = 0; x < this->width; ++x)
            {
                if (rand() % 2 < 1)
                    this->setPos(x, y, life_status_t::dead);
                else
                    this->setPos(x, y, life_status_t::alive);
            }
        }

        return true;
    }

    std::ifstream BoardFile(sourceFileName);

    int line_number = 0;
    std::string line;
    while (getline(BoardFile, line))
    {
        //        std::cout << line << std::endl;
        int line_length = (int)line.length();

        if (line_length < 1 or line[0] == '#')
        {
            --line_number;
            continue;
        }

        if (line_number == 0)
        {
            int x_size = -1, y_size = -1;

            for (int cursor = 0; cursor < line_length; ++cursor)
            {
                if (isdigit(line[cursor]) and x_size == -1)
                {
                    int read_start_index = cursor;
                    while (isdigit(line[++cursor]))
                        ;
                    x_size = std::stoi(line.substr(read_start_index, read_start_index - cursor));
                }
                else if (isdigit(line[cursor]))
                {
                    int read_start_index = cursor;
                    while (isdigit(line[++cursor]))
                        ;
                    y_size = std::stoi(line.substr(read_start_index, read_start_index - cursor));
                }
            }

            this->width = x_size;
            this->height = y_size;

            --line_number;
        }

        if (line_number > 1)
        {
            int written_cells = 0;
            for (int cursor = 0; cursor < line_length; ++cursor)
            {
                int number = -1;
                if (isdigit(line[cursor]))
                {
                    int read_start_index = cursor;
                    while (isdigit(line[++cursor]))
                        ;
                    number = std::stoi(line.substr(read_start_index, read_start_index - cursor));
                }

                if (line[cursor] == 'b')
                {
                    if (number == -1)
                    {
                        this->setPos(written_cells, line_number, life_status_t::dead);
                        ++written_cells;
                    }
                    else
                    {
                        for (int i = 0; i < number; ++i)
                        {
                            this->setPos(written_cells, line_number, life_status_t::dead);
                            ++written_cells;
                        }
                    }
                }
                else if (line[cursor] == 'o')
                {
                    if (number == -1)
                    {
                        this->setPos(written_cells, line_number, life_status_t::alive);
                        ++written_cells;
                    }
                    else
                    {
                        for (int i = 0; i < number; ++i)
                        {
                            this->setPos(written_cells, line_number, life_status_t::alive);
                            ++written_cells;
                        }
                    }
                }
            }

            if (written_cells < this->width)
            {
                int diff = this->width - written_cells;
                for (int i = 0; i < diff; ++i)
                {
                    this->setPos(written_cells, line_number, life_status_t::dead);
                    ++written_cells;
                }
            }
        }

        ++line_number;
    }

    BoardFile.close();
    return true;
}

int LocalBoard::getWidth()
{
    return width;
}

int LocalBoard::getHeight()
{
    return height;
}