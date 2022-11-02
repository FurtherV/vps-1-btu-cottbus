#ifndef LOCAL_BOARD_H
#define LOCAL_BOARD_H

#include <sys/types.h>
#include <vector>
#include <string>
#include <Board.h>

class LocalBoard : Board {
public:
    /**
     * Create a board with a given size, where the upper left corner is 0,0
     *
     * @param width is the horizontal size of the board
     * @param height is the vertical size of the board
     */
    LocalBoard(size_t width, size_t height) : Board(width, height) {}

    /**
     * Because this is a pure virtual class, we should declare
     * the destructor also as virtual.
     */
    virtual ~LocalBoard() {}

    /**
     * Get the liveliness of a specified element.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @return dead or alive if element exists, else invalid
     */
    virtual enum life_status_t getPos(size_t x, size_t y);

    /**
     * Sets a element to a life status. If position or status is invalid,
     * nothing is done.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @param status is the life status to be set
     */
    virtual void setPos(size_t x, size_t y, enum life_status_t status);

    /**
     * Exports this board to output file.
     *
     * @return true, if successful
     * @return false, else
     */
    virtual bool exportAll(std::string destFileName);

    /**
     * Imports to this board from output file.
     *
     * @return true
     * @return false
     */
    virtual bool importAll(std::string sourceFileName);

    /**
     * Performs one step on this board.
     */
    virtual void step();

protected:
    /**
     * Sets an element to a life status. Invalid inputs will be discarded.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @param status is the life status to be set
     */
    virtual void setPosRaw(size_t x, size_t y, enum life_status_t status);

    /**
     * Gets the life status of an element. Invalid inputs will return invalid.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     */
    virtual enum life_status_t getPosRaw(size_t x, size_t y);

    // 1-Dimensional representation of the field (y * width + x, to access (x,y))
    std::vector<enum life_status_t> field;
};

#endif