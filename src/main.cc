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
 * @brief The Game of Life (GoL for short), with an optional graphical representation.
 *
 * @details This program supports the following command line options:
 * -i <FILE> 	=> Loads the given file as a board configuration.
 * -o <FILE> 	=> Saves the board configuration (after the simulation) as given file.
 * -r <NUMBER> 	=> Amount of simulation steps to be done.
 * -g 			=> If set, a GUI will be used.
 * -a			=> If set, Board will be drawn to match aspect ratio of window to board.
 * 				(IE: cells might not be squares).
 * -w <NUMBER>	=> Sets the windows width, only works when -g is set, default: 800
 * -h <NUMBER>	=> Sets the windows height, only works when -g is set, default: 800
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
	unsigned int windowWidth = 800;
	unsigned int windowHeight = 800;

	bool graphical = false;
	bool aspect = false;

	while ((option_val = getopt(argc, argv, ":i:o:r:w:h:ga")) != -1)
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
			if (steps < 0)
			{
				cout << "[ERROR] GoL does not support negative step amounts." << endl;
				return 1;
			}
			break;
		case 'g':
			graphical = true;
			break;
		case 'a':
			aspect = true;
			break;
		case 'w':
			if (optarg != NULL)
			{
				int newWindowWidth = atoi(optarg);
				if (newWindowWidth <= 0)
				{
					cout << "[ERROR] GoL GUI does not support zero or negative window sizes." << endl;
					return 1;
				}
				windowWidth = (unsigned int)newWindowWidth;
			}
			break;
		case 'h':
			if (optarg != NULL)
			{
				int newWindowHeight = atoi(optarg);
				if (newWindowHeight <= 0)
				{
					cout << "[ERROR] GoL GUI does not support zero or negative window sizes." << endl;
					return 1;
				}
				windowHeight = (unsigned int)newWindowHeight;
			}
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
		DrawingWindow window(windowWidth, windowHeight, "Game of Life");
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
