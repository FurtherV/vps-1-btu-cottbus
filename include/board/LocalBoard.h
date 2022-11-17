#ifndef LOCALBOARD_H
#define LOCALBOARD_H

#include <algorithm>
#include <string>
#include <sys/types.h>
#include <vector>

#include "board/Board.h"
#include "misc/Log.h"

class LocalBoard : public Board {
  public:
    /**
     * Create a board with a given size, where the upper left corner is 0,0
     *
     * @param width is the horizontal size of the board
     * @param height is the vertical size of the board
     */
    LocalBoard(int width, int height);

    /**
     * Because this is a pure virtual class, we should declare
     * the destructor also as virtual.
     */
    virtual ~LocalBoard();

    /**
     * Sets a element to a life status. If position or status is invalid,
     * nothing is done.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @param status is the life status to be set
     */
    void setPos(int x, int y, enum life_status_t status) override;

    /**
     * Get the liveliness of a specified element.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @return dead or alive if element exists, else invalid
     */
    life_status_t getPos(int x, int y) override;

    /**
     * Exports this board to output file.
     *
     * @return true, if successful, else otherwise.
     */
    bool exportAll(std::string destFileName) override;

    /**
     * Imports to this board from output file.
     *
     * @return true, if successful, else otherwise.
     */
    bool importAll(std::string sourceFileName) override;

    /**
     * Performs one step on this board.
     */
    void step() override;

    /**
     * @brief Get the board width.
     * @return board width.
     */
    int getWidth() override;

    /**
     * @brief Get the board height
     * @return board height.
     */
    int getHeight() override;

    /**
     * @brief Clears the board by killing all alive cells.
     */
    void clear() override;

  protected:
    /**
     * Sets an element to a life status. Invalid inputs will be discarded.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @param status is the life status to be set
     */
    void setPosRaw(int x, int y, enum life_status_t status);

    /**
     * Gets the life status of an element. Invalid inputs will return invalid.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     */
    enum life_status_t getPosRaw(int x, int y);

    // 1-Dimensional representation of the field (y * width + x, to access (x,y))
    std::vector<enum life_status_t> field;
};

#endif