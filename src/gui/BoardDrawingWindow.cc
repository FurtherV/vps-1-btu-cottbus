#include "gui/BoardDrawingWindow.h"
#include <unistd.h>
#include <algorithm>

#include "misc/Log.h"

using namespace GUI;
using namespace std;

BoardDrawingWindow::BoardDrawingWindow(Board *board, unsigned int width, unsigned int height) : DrawingWindow(width, height, "Game of Life"), board(board)
{
    setForegroundColor(WHITE);
    setBackgroundColor(BLACK);
}

BoardDrawingWindow::~BoardDrawingWindow() = default;

void BoardDrawingWindow::drawBoard()
{
    clear();

    // calculate how many cells fit horizontally and vertically
    int cellsHorizontally = getWidth() / pixelsPerCell;
    int cellsVertically = getHeight() / pixelsPerCell;

    for (int row = originY; row < std::min(board->getHeight(), originY + cellsVertically); row++)
    {
        for (int column = originX; column < std::min(board->getWidth(), originX + cellsHorizontally); column++)
        {
            life_status_t state = board->getPos(column, row);
            if (state == life_status_t::alive)
            {
                drawFilledRectangle((column - originX) * (pixelsPerCell + borderPixels), (row - originY) * (pixelsPerCell + borderPixels), pixelsPerCell, pixelsPerCell);
            }
        }
    }
}

void BoardDrawingWindow::run()
{
    if (destroyed)
        return; // check if window is closed

    // event data structure for reading events from the event queue
    XEvent event;

    // define which input sources should be used for events
    XLockDisplay(display);
    XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask);

    // setup WM_DELETE_WINDOW protocol to intercept the destruction of the window
    // by the window manager (e.g. when users decides to close the window)
    Atom wmDeleteWindowAtom = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(display, window, &wmDeleteWindowAtom, 1); // takes array of atoms and count
    XUnlockDisplay(display);

    // endless loop which processes events
    while (!destroyed)
    {
        Thread::testCancel(); // creates a cancellation point
        usleep(100);

        // read an event
        XLockDisplay(display);
        bool cont = !XPending(display);
        XUnlockDisplay(display);

        auto currentTime = std::chrono::system_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);

        if (!redraw && board->getCurrentStep() != lastUpdateStep)
        {
            redraw = true;
            lastUpdateStep = board->getCurrentStep();
        }

        if (elapsedTime.count() > (1000 / updateFrequency) && redraw)
        {
            drawBoard();
            lastUpdateTime = std::chrono::system_clock::now();
            redraw = false;
        }

        if (cont) // pre-check if there are events to prevent blocking
        {
            //			cerr << "c";
            continue;
        }

        XLockDisplay(display);
        //		cerr << "p";
        XNextEvent(display, &event); // blocking if no events are pending
        XUnlockDisplay(display);

        // if(!XCheckWindowEvent(display, window, ExposureMask, &event)) // non blocking
        //	continue;

        // evaluate the event type
        switch (event.type)
        {
        case Expose:
            _DEBUG(cout << "DrawingWindow::run(), Info: Expose message received. " << endl);
            onExpose(event.xexpose.x, event.xexpose.y, event.xexpose.width, event.xexpose.height);
            break;
        case ClientMessage:
            _DEBUG(cout << "DrawingWindow::run(), Info: Delete Window request received. " << endl);
            if (destroyable) // if not destroyable => ignore close button
                destroyed = true;
            break;
        case KeyPress:
            LOG(DEBUG) << "Keycode: " << event.xkey.keycode;
            switch (event.xkey.keycode)
            {
            case 98:
                // UP
                if (originY > 0)
                {
                    originY -= 1;
                    redraw = true;
                }
                break;
            case 104:
                // DOWN
                if (originY < board->getHeight() - 3)
                {
                    originY += 1;
                    redraw = true;
                }
                break;
            case 102:
                // RIGHT
                if (originX < board->getWidth() - 3)
                {
                    originX += 1;
                    redraw = true;
                }
                break;
            case 100:
                // LEFT
                if (originX > 0)
                {
                    originX -= 1;
                    redraw = true;
                }
                break;
            case 99:
                // ZOOM IN
                if (pixelsPerCell < maxPixelsPerCell)
                {
                    pixelsPerCell += 1;
                    redraw = true;
                }
                break;
            case 105:
                // ZOOM OUT
                if (pixelsPerCell > minPixelsPerCell)
                {
                    pixelsPerCell -= 1;
                    redraw = true;
                }
                break;
            case 29:
                // Reset Zoom
                pixelsPerCell = minPixelsPerCell;
                redraw = true;
                break;
            }
            break;
        default:
            _DEBUG(cout << "DrawingWindow::run(), Warning: Unhandeled event.type: " << event.type << endl;)
            break;
        }
    }

    // this point is reached when the window is destroyed/closed, the window thread ends here
    // program will continue at the blocking call of Thread::join()
    destroyWindow();
}