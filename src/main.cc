#include "gui/DrawingWindow.h"

using namespace std;
using namespace GUI;

int main(int argc, char *argv[])
{
	// example code for using the DrawingWindow class

	DrawingWindow window(800, 600, "Game of Life");
	// DrawingWindow window(800, 600, "Game of Life", true, true);
	window.setForegroundColor(RGBColor(255, 0, 0));
	window.drawRectangle(100, 100, 75, 150);
	window.setForegroundColor(LIME);
	window.drawLine(0, 0, 100, 100);

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
