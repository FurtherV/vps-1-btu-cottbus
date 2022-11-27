#include "board/LocalBoard.h"
#include "gui/BoardDrawingWindow.h"
#include "misc/Log.h"
#include <boost/program_options.hpp>
#include <chrono>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

using namespace std;
using namespace GUI;

int write_benchmark_file(string file_path, long time) {
    ofstream benchmark_file(file_path);
    benchmark_file << time;
    benchmark_file.close();
    return 1;
}

int main(int argc, char *argv[]) {
    // Configure logger before usage.
    LOGCFG = {};
    LOGCFG.headers = true;
#ifdef DEBUG_MODE
    LOGCFG.level = DEBUG;
#else
    LOGCFG.level = ERROR;
#endif
    // End of configuration.

    // parse command line args
    namespace po = boost::program_options;

    // define available arguments
    po::options_description desc("Usage", 1024, 512);
    desc.add_options()                                                                                         //
        ("help,", "Print help message")                                                                        //
        ("input,i", po::value<string>()->default_value(""), "Input file\nMust be in the correct .rle format")  //
        ("output,o", po::value<string>()->default_value(""), "Output file\nExisting files will be overwriten") //
        ("steps,r", po::value<int>()->default_value(1), "Simulation steps")                                    //
        ("width,w", po::value<int>()->default_value(100), "Width of the board\nNot compatible with -i")        //
        ("height,h", po::value<int>()->default_value(100), "Height of the board\nNot compatible with -i")      //
        ("profile,", po::value<string>(), "Output file for profiler\nNot compatible with -g")                  //
        ("gui,g", "Enable GUI");                                                                               //

    // read arguments
    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error &e) {
        LOG(ERROR) << e.what();
        cerr << desc << endl;
        return 1;
    }

    // Print help text and exit on --help
    if (vm.count("help")) {
        cerr << "Game of Life Server" << endl;
        cerr << desc << endl;
        return 0;
    }

    // Store parameters and validate them
    int board_width = vm["width"].as<int>();
    if (board_width <= 0) {
        LOG(ERROR) << "'width' argument must be greater than 0";
        return 1;
    }

    int board_height = vm["height"].as<int>();
    if (board_height <= 0) {
        LOG(ERROR) << "'height' argument must be greater than 0";
        return 1;
    }

    int simulation_steps = vm["steps"].as<int>();
    if (simulation_steps < 0) {
        LOG(ERROR) << "'steps' argument must be greater than 0";
        return 1;
    }

    string input_path = "RANDOM";
    if (!vm["input"].defaulted()) {
        input_path = vm["input"].as<string>();
        LOG(DEBUG) << "Importing board from " << input_path;
    }

    LocalBoard *board = new LocalBoard(board_width, board_height);
    bool importResult = board->importAll(input_path);
    if (!importResult) {
        LOG(ERROR) << "Could not import board from file '" << input_path << "'.";
        LOG(ERROR) << "File might be missing, its content might be malformed or permissions to access it are missing.";
        return 1;
    }

    if (vm.count("gui")) {
        BoardDrawingWindow window(board, 800, 800);

        cout << "Simulation Step: 0" << endl;
        cout << "Press enter to continue..." << endl;
        cin.get();
        for (int i = 0; i < simulation_steps; i++) {
            board->step();
            cout << "Simulation Step: " << i + 1 << endl;
            cout << "Press enter to continue..." << endl;
            cin.get();
        }
    } else {
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < simulation_steps; i++) {
            board->step();
        }
        auto end = chrono::high_resolution_clock::now();
        if (vm.count("profile")) {
            long time = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            write_benchmark_file(vm["profile"].as<string>(), time);
        }
    }

    if (!vm["output"].defaulted()) {
        LOG(DEBUG) << "Exporting board to " << vm["output"].as<string>();
        board->exportAll(vm["output"].as<string>());
    }

    cout << "Done." << endl;
    delete board;
    return 0;
}
