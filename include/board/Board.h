#ifndef BOARD_H
#define BOARD_H

#include <sys/types.h>

enum life_status_t
{
	invalid,
	dead,
	alive
};

/**
 * A Board is the matrix for the life-simulation.
 *
 * Subclasses must implement the Board(),readPos() and setPos() functions
 * in order to be instanciated.
 */

class Board
{
public:
	/**
	 * Create a board with a given size, where the upper left corner is 0,0
	 *
	 * @param width is the horizontal size of the board
	 * @param height is the vertical size of the board
	 */
	Board(int width, int height)
		: width(width), height(height) {}

	/**
	 * Because this is a pure virtual class, we should declare
	 * the destructor also as virtual.
	 */
	virtual ~Board() {}

	/**
	 * Get the liveliness of a specified element.
	 *
	 * @param x is the horizontal position of the element
	 * @param y is the vertical position of the element
	 * @return dead or alive if element exists, else invalid
	 */
	virtual enum life_status_t readPos(size_t x, size_t y) = 0;

	/**
	 * Sets an element to a life status. Input will be modulo width or height.
	 *
	 * @param x is the horizontal position of the element
	 * @param y is the vertical position of the element
	 * @param status is the life status to be set
	 */
	virtual void setPos(size_t x, size_t y, enum life_status_t status) = 0;

	/**
	 * Gets the life status of an element. Input will be modulo width or height.
	 *
	 * @param x is the horizontal position of the element
	 * @param y is the vertical position of the element
	 * @return dead or alive if element exists, else invalid
	 */
	virtual life_status_t getPos(size_t x, size_t y) = 0;

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

private:
	/**
	 * Sets an element to a life status. Invalid inputs will be discarded.
	 *
	 * @param x is the horizontal position of the element
	 * @param y is the vertical position of the element
	 * @param status is the life status to be set
	 */
	virtual void setPos(int x, int y, enum life_status_t status) = 0;

	/**
	 * @brief Get the board width.
	 * @return board width.
	 */
	virtual int getWidth() = 0;

	/**
	 * @brief Get the board height
	 * @return board height.
	 */
	virtual int getHeight() = 0;

protected:
	int width;
	int height;
};

#endif
