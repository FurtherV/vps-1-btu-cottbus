#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>

#include "gui/DrawingWindow.h"
#include "board/LocalBoard.h"
#include "gui/BoardDrawer.h"

using namespace std;
using namespace GUI;

/**
 * @brief The game of life, with an optional graphical representation.
 *
 * @details This program supports the following command line options:
 * -i <FILE> 	=> Loads the given file as a board configuration.
 * -o <FILE> 	=> Saves the board configuration (after the simulation) as given file.
 * -r <NUMBER> 	=> Amount of simulation steps to be done.
 * -g 			=> If set, a GUI will be used.
 * -a			=> If set, Board will be drawn to match aspect ratio of window to board.
 * 				(IE: cells might not be squares).
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
	int option_val = 0;

	string input_path = "RANDOM";
	string output_path = "";

	int steps = 0;

	bool graphical = false;
	bool aspect = false;

	while ((option_val = getopt(argc, argv, ":i:o:r:ga")) != -1)
	{
		switch (option_val)
		{
		case 'i':
			if (optarg != NULL)
				input_path = optarg;
			break;
		case 'o':
			if (optarg != NULL)
				output_path = optarg;
			break;
		case 'r':
			if (optarg != NULL)
				steps = atoi(optarg);
			break;
		case 'g':
			graphical = true;
			break;
		case 'a':
			aspect = true;
			break;
		default:
			break;
		}
	}

	if (input_path == "RANDOM")
	{
		cout << "[INFO] No board configuration specified, GoL will use random configuration." << endl;
	}

	if (output_path == "")
	{
		cout << "[INFO] No output file specified, GoL will not write anything to disk." << endl;
	}

	LocalBoard board(3, 3);
	board.importAll(input_path);

	if (graphical)
	{
		DrawingWindow window(800, 800, "Game of Life");
		BoardDrawer drawer(&window, &board, !aspect);
		drawer.draw();

		cout << "Simulation Step: 0" << endl;
		cout << "Press enter to continue..." << endl;
		cin.get();
		for (int i = 0; i < steps; i++)
		{
			board.step();
			drawer.draw();
			cout << "Simulation Step: " << i + 1 << endl;
			cout << "Press enter to continue..." << endl;
			cin.get();
		}
	}
	else
	{
		for (int i = 0; i < steps; i++)
		{
			board.step();
		}
	}

	if (output_path != "")
	{
		board.exportAll(output_path);
	}
	cout << "Done." << endl;
	return 0;
}
