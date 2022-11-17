#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

#include "board/LocalBoard.h"

LocalBoard::LocalBoard(int width, int height) : Board(width, height), field(width * height, life_status_t::dead) {
    if (width * height <= 0) {
        throw std::invalid_argument("width or height was negative or zero.");
    }
}

LocalBoard::~LocalBoard() { field.clear(); }

void LocalBoard::setPos(int x, int y, enum life_status_t status) {
    while (x < 0) {
        x += width;
    }
    while (y < 0) {
        y += height;
    }
    LocalBoard::setPosRaw(x % width, y % height, status);
}

enum life_status_t LocalBoard::getPos(int x, int y) {
    while (x < 0) {
        x += width;
    }
    while (y < 0) {
        y += height;
    }
    return LocalBoard::getPosRaw(x % width, y % height);
}

void LocalBoard::step() {
    std::vector<enum life_status_t> newField = std::vector<life_status_t>(width * height, life_status_t::dead);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int neighbourCount = 0;
            enum life_status_t status = getPos(x, y);
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx != 0 || dy != 0) {
                        if (getPos(x + dx, y + dy) == life_status_t::alive) {
                            neighbourCount++;
                        }
                    }
                }
            }
            if (status == life_status_t::dead && neighbourCount == 3) {
                newField[y * width + x] = life_status_t::alive;
                // newField.push_back(life_status_t::alive);
            } else if (status == life_status_t::alive && (neighbourCount == 2 || neighbourCount == 3)) {
                newField[y * width + x] = life_status_t::alive;
                // newField.push_back(life_status_t::alive);
            } else {
                newField[y * width + x] = life_status_t::dead;
                // newField.push_back(life_status_t::dead);
            }
        }
    }
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            setPosRaw(x, y, newField[y * width + x]);
        }
    }
    currentStep += 1;
}

void LocalBoard::setPosRaw(int x, int y, enum life_status_t status) { field[y * width + x] = status; }

enum life_status_t LocalBoard::getPosRaw(int x, int y) { return field[y * width + x]; }

bool LocalBoard::exportAll(std::string destFileName) {
    std::ofstream outBoardFile(destFileName);

    outBoardFile << "x = " << this->width << ", y = " << this->height << std::endl;

    for (int y = 0; y < this->height; ++y) {
        for (int x = 0; x < this->width; ++x) {
            if (this->getPos(x, y) == life_status_t::alive)
                outBoardFile << "o";
            else
                outBoardFile << "b";
        }
        outBoardFile << "$" << std::endl;
    }

    outBoardFile.close();

    return true;
}

bool LocalBoard::importAll(std::string sourceFileName) {
    if (sourceFileName == "RANDOM") {
        srand(time(nullptr));

        // this->width = 1 + rand() % 64;
        // this->height = 1 + rand() % 64;
        field.resize(width * height, life_status_t::dead);

        for (int y = 0; y < this->height; ++y) {
            for (int x = 0; x < this->width; ++x) {
                if (rand() % 2 < 1)
                    this->setPos(x, y, life_status_t::dead);
                else
                    this->setPos(x, y, life_status_t::alive);
            }
        }

        return true;
    }

    std::ifstream boardFile(sourceFileName);
    if (!boardFile.good())
        return false;

    int written_cells_this_line = 0;
    int line_number = -1;
    std::string line;
    while (getline(boardFile, line)) {
        int line_length = (int)line.length();

        if (line_length < 1 or line[0] == '#') {
            continue;
        }

        // first line, attempt to parse board sizes
        if (line_number == -1) {
            // remove all spaces in line to make parsing it easier
            std::string::iterator end_pos = std::remove(line.begin(), line.end(), ' ');
            line.erase(end_pos, line.end());
            int x_size = -1, y_size = -1;
            size_t pos = 0;
            std::string delimiter = ",";
            std::string token;

            // try catch, because we parse user input directly with std::stoi, which can throw exceptions if its not a
            // number
            try {
                std::vector<string> tokens;
                while ((pos = line.find(delimiter)) != std::string::npos) {
                    token = line.substr(0, pos);
                    tokens.push_back(token);
                    line.erase(0, pos + delimiter.length());
                }
                tokens.push_back(line);
                for (string token : tokens) {
                    if (token.rfind("x=") == 0) {
                        token.erase(0, 2);
                        x_size = std::stoi(token);
                    } else if (token.rfind("y=") == 0) {
                        token.erase(0, 2);
                        y_size = std::stoi(token);
                    }
                }
            } catch (...) {
                return false;
            }

            if (x_size <= 0 || y_size <= 0) {
                LOG(DEBUG) << "Read invalid board sizes (" << x_size << "," << y_size << ") from file '"
                           << sourceFileName << "'.";
                return false;
            }

            this->width = x_size;
            this->height = y_size;

            field.resize(width * height, life_status_t::dead);

            ++line_number;
            continue;
        }

        // all other lines, parse board content
        if (line_number >= 0) {
            for (int cursor = 0; cursor < line_length; ++cursor) {
                int number = -1;
                if (isdigit(line[cursor])) {
                    int read_start_index = cursor;
                    while (isdigit(line[++cursor]))
                        ;
                    number = std::stoi(line.substr(read_start_index, read_start_index - cursor));
                }

                if (line[cursor] == 'b') {
                    if (number == -1) {
                        this->setPos(written_cells_this_line, line_number, life_status_t::dead);
                        ++written_cells_this_line;
                    } else {
                        for (int i = 0; i < number; ++i) {
                            this->setPos(written_cells_this_line, line_number, life_status_t::dead);
                            ++written_cells_this_line;
                        }
                    }
                } else if (line[cursor] == 'o') {
                    if (number == -1) {
                        this->setPos(written_cells_this_line, line_number, life_status_t::alive);
                        ++written_cells_this_line;
                    } else {
                        for (int i = 0; i < number; ++i) {
                            this->setPos(written_cells_this_line, line_number, life_status_t::alive);
                            ++written_cells_this_line;
                        }
                    }
                }

                if (line[cursor] == '$') {
                    if (written_cells_this_line < this->width) {
                        int diff = this->width - written_cells_this_line;
                        for (int i = 0; i < diff; ++i) {
                            this->setPos(written_cells_this_line, line_number, life_status_t::dead);
                            ++written_cells_this_line;
                        }
                    }
                    written_cells_this_line = 0;

                    ++line_number;
                }
            }
        }
    }

    boardFile.close();
    return true;
}

int LocalBoard::getWidth() { return width; }

int LocalBoard::getHeight() { return height; }

void LocalBoard::clear() {
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            setPosRaw(x, y, life_status_t::dead);
        }
    }
}