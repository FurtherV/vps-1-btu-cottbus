#include "gui/DrawingWindow.h"
#include "LocalBoard.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>

using namespace std;
using namespace GUI;

int main(int argc, char *argv[]) {
    int option_val = 0;

    string input_path = "RANDOM";
    string output_path;

    int steps = 1;

    bool graphical = false;

    while ((option_val = getopt(argc, argv, ":i:o:r:g")) != -1) {
        switch (option_val) {
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

    auto *localBoard = new LocalBoard(0, 0);

    localBoard->importAll(input_path);

    std::cout << "sad" << input_path << endl;



    // example code for using the DrawingWindow class

//    DrawingWindow window(800, 600, "Game of Life");
//    // DrawingWindow window(800, 600, "Game of Life", true, true);
//    window.setForegroundColor(RGBColor(255, 0, 0));
//    window.drawRectangle(100, 100, 75, 150);
//    window.setForegroundColor(LIME);
//    window.drawLine(0, 0, 100, 100);
//
//    // run until user presses q on console
//    char input = ' ';
//    while (input != 'q') {
//        cout << "Press q and enter to quit" << endl;
//        cin >> input;
//        cout << "read: '" << input << "' from console" << endl;
//    }

    return 0;
}
