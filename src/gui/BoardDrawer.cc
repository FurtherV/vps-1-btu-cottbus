#include "gui/BoardDrawer.h"

BoardDrawer::BoardDrawer(GUI::DrawingWindow *drawingWindow, Board *board, bool forceSquareCells) : drawingWindow(drawingWindow),
                                                                                                   board(board),
                                                                                                   forceSquareCells(forceSquareCells)
{
    drawingWindow->setBackgroundColor(GUI::BLACK);
    drawingWindow->setForegroundColor(GUI::WHITE);
    drawingWindow->clear();

    calculateSizes();
}

void BoardDrawer::calculateSizes()
{
    boardWidth = board->getWidth();
    boardHeight = board->getHeight();
    size_t windowWidth = drawingWindow->getWidth();
    size_t windowHeight = drawingWindow->getHeight();

    // TODO: remove these and add a zoom + scroll implementation
    if (boardWidth > windowWidth)
    {
        LOG(WARN) << "Board width is greater than window width, graphical output might be inaccurate!";
    }

    if (boardHeight > windowHeight)
    {
        LOG(WARN) << "Board height is greater than window height, graphical output might be inaccurate!";
    }

    // calculate how many pixels in width and height each grid cell will be rendered as by dividing the window size by the board size and rounding.
    // if the window is smaller than the board, we still use 1 pixel, since we can only draw whole pixels.
    cellWidthPixel = (size_t)std::roundf(std::fmaxf(1, (float)windowWidth / (float)boardWidth));
    cellHeightPixel = (size_t)std::roundf(std::fmaxf(1, (float)windowHeight / (float)boardHeight));

    if (forceSquareCells)
    {
        size_t min = std::min(cellWidthPixel, cellHeightPixel);
        cellWidthPixel = min;
        cellHeightPixel = min;
    }

    LOG(DEBUG) << "Each cell will be drawn " << cellWidthPixel << "px wide and " << cellHeightPixel << "px high";
    if (forceSquareCells)
    {
        LOG(DEBUG) << "Square cells are forced.";
    }
    else
    {
        LOG(DEBUG) << "Square cells are not forced.";
    }
}

void BoardDrawer::draw()
{
    // check if window is closed
    if (drawingWindow->isDestroyed())
        return;

    drawingWindow->clear();
    size_t rows = boardHeight;
    size_t columns = boardWidth;

    for (size_t row = 0; row < rows; row++)
    {
        for (size_t column = 0; column < columns; column++)
        {
            life_status_t cellState = board->getPos(column, row);
            if (cellState == life_status_t::alive)
            {
                drawingWindow->drawFilledRectangle(column * cellWidthPixel, row * cellHeightPixel, cellWidthPixel, cellHeightPixel);
            }
        }
    }
}