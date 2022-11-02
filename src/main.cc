#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>

#include "gui/DrawingWindow.h"
#include "board/LocalBoard.h"

using namespace std;
using namespace GUI;

int main(int argc, char *argv[])
{
	int option_val = 0;

	string input_path = "RANDOM";
	string output_path;

	int steps = 1;

	bool graphical = false;

	while ((option_val = getopt(argc, argv, ":i:o:r:g")) != -1)
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
		default:
			break;
		}
	}

	LocalBoard board(0, 0);

	auto *localBoard = new LocalBoard(0, 0);

	localBoard->importAll(input_path);

	std::cout << "sad" << input_path << endl;
	return 0;
}
