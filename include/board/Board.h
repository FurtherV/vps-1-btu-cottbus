#ifndef BOARD_H
#define BOARD_H

#include <atomic>
#include <string>
#include <sys/types.h>

enum life_status_t { invalid, dead, alive };

/**
 * A Board is the matrix for the life-simulation.
 *
 * Subclasses must implement the Board(), readPos() and setPos() functions
 * in order to be instanciated.
 */

class Board {
  public:
    /**
     * Create a board with a given size, where the upper left corner is 0,0
     *
     * @param width is the horizontal size of the board
     * @param height is the vertical size of the board
     */
    Board(int width, int height) : width(width), height(height) { currentStep = 0; }

    /**
     * Because this is a pure virtual class, we should declare
     * the destructor also as virtual.
     */
    virtual ~Board() {}

    /**
     * Sets an element to a life status. Input will be modulo width or height.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @param status is the life status to be set
     */
    virtual void setPos(int x, int y, life_status_t status) = 0;

    /**
     * Gets the life status of an element. Input will be modulo width or height.
     *
     * @param x is the horizontal position of the element
     * @param y is the vertical position of the element
     * @return dead or alive if element exists, else invalid
     */
    virtual life_status_t getPos(int x, int y) = 0;

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

    /**
     * @brief Get the current board iteration number.
     * @return iteration number (current step)
     */
    size_t getCurrentStep() { return currentStep; }

    /**
     * @brief Sets the current board iteration number.
     * @param currentStep iteration number (current step)
     */
    void setCurrentStep(size_t currentStep) { this->currentStep = currentStep; }

    /**
     * @brief Clears the board by killing all alive cells.
     */
    virtual void clear() = 0;

  protected:
    int width;
    int height;
    std::atomic_size_t currentStep;
};

#endif
