#ifndef BoardDrawingWindow_h
#define BoardDrawingWindow_h

#include <chrono>

#include "gui/DrawingWindow.h"
#include "board/Board.h"

namespace GUI
{
    class BoardDrawingWindow : DrawingWindow
    {
    public:
        BoardDrawingWindow(Board *board, unsigned int width, unsigned int height);
        virtual ~BoardDrawingWindow();
        virtual void run();

    private:
        void drawBoard();

        Board *board;
        int originX = 0;
        int originY = 0;
        int pixelsPerCell = 5;     // Pixels
        int minPixelsPerCell = 5;  // Pixels
        int maxPixelsPerCell = 20; // Pixels
        int borderPixels = 1;      // Pixels
        int updateFrequency = 5;   // Hz
        size_t lastUpdateStep = -1;
        bool redraw = true;
        std::chrono::time_point<std::chrono::system_clock> lastUpdateTime = std::chrono::system_clock::now();
    };
};

#endif // BoardDrawingWindow_h