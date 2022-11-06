#ifndef BoardDrawer_h
#define BoardDrawer_h

#include <cmath>

#include "misc/Log.h"
#include "gui/DrawingWindow.h"
#include "board/Board.h"

class BoardDrawer
{
public:
    /**
     * Creates a board drawer which draws a board on a drawing window.
     *
     * @param drawingWindow The window on which the board will be drawn.
     * @param board The board that will be drawn.
     * @param forceSquareCells if true, drawn cells will be forced to have same width as height irrespective of window to board sizes.
     */
    BoardDrawer(GUI::DrawingWindow *drawingWindow, Board *board, bool forceSquareCells = true);

    void draw();
    void calculateSizes();

protected:
    GUI::DrawingWindow *drawingWindow;
    Board *board;
    size_t boardWidth;
    size_t boardHeight;
    size_t cellWidthPixel;
    size_t cellHeightPixel;
    bool forceSquareCells;
};

#endif