#include "gui/DrawingWindow.h"
#include "board/LocalBoard.h"
#include "gui/BoardDrawer.h"

using namespace std;
using namespace GUI;

int main(int argc, char *argv[])
{
	// example code for using the DrawingWindow class

	DrawingWindow window(1000, 1000, "Game of Life");
	LocalBoard board(100, 100);
	BoardDrawer drawer(&window, &board);

	drawer.draw();

	// run until user presses q on console
	char input = ' ';
	while (input != 'q')
	{
		cout << "Press q and enter to quit" << endl;
		cin >> input;
		cout << "read: '" << input << "' from console" << endl;
	}

	return 0;
}
