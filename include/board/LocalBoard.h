#ifndef LOCAL_BOARD_H
#define LOCAL_BOARD_H

#include <sys/types.h>
#include <vector>
#include <string>
#include <Board.h>

class LocalBoard : Board
{
    public:
        /**
         * Create a board with a given size, where the upper left corner is 0,0
         *
         * @param width is the horizontal size of the board
         * @param height is the vertical size of the board
         */
        LocalBoard(int width, int height) : Board(width, height) {}

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
        virtual enum life_status_t getPos(int x, int y) = 0;

        /**
         * Sets a element to a life status. If position or status is invalid,
         * nothing is done.
         *
         * @param x is the horizontal position of the element
         * @param y is the vertical position of the element
         * @param status is the life status to be set
         */
        virtual void setPos(int x, int y, enum life_status_t status) = 0;
        /**
         * Exports this board to output file.
         *
         * @return true, if successful
         * @return false, else
         */
        virtual bool exportAll(std::string destFileName) = 0;

        /**
         * Imports to this board from output file.
         *
         * @return true
         * @return false
         */
        virtual bool importAll(std::string sourceFileName) = 0;

        /**
         * Performs one step on this board.
         */
        virtual void step() = 0;

        virtual int getWidth() = 0;

        virtual int getHeight() = 0;

    protected:
        /**
         * Sets an element to a life status. Invalid inputs will be discarded.
         *
         * @param x is the horizontal position of the element
         * @param y is the vertical position of the element
         * @param status is the life status to be set
         */
        virtual void setPosRaw(int x, int y, enum life_status_t status) = 0;

        /**
         * Gets the life status of an element. Invalid inputs will return invalid.
         *
         * @param x is the horizontal position of the element
         * @param y is the vertical position of the element
         */
        virtual enum life_status_t getPosRaw(int x, int y) = 0;

        // 1-Dimensional representation of the field (y * width + x, to access (x,y))
        std::vector<enum life_status_t> field;
};

#endif