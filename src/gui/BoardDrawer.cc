#include <cmath>
#include "gui/BoardDrawer.h"

BoardDrawer::BoardDrawer(GUI::DrawingWindow *drawingWindow, Board *board) : drawingWindow(drawingWindow), board(board)
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

    if (boardWidth > windowWidth)
    {
        std::printf("[WARNING] Board width is greater than window width, graphical output might be inaccurate!");
    }

    if (boardHeight > windowHeight)
    {
        std::printf("[WARNING] Board height is greater than window height, graphical output might be inaccurate!");
    }

    float cellWidthPixel = (float)windowWidth / (float)boardWidth;
    float cellHeightPixel = (float)windowHeight / (float)boardHeight;
    size_t usedCellSizePixel = (size_t)std::round(std::fmax((float)1, std::fmin(cellWidthPixel, cellHeightPixel)));
    cellSizePixel = usedCellSizePixel;
}

void BoardDrawer::draw()
{
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
                drawingWindow->drawFilledRectangle(row * cellSizePixel, column * cellSizePixel, cellSizePixel, cellSizePixel);
            }
        }
    }
}