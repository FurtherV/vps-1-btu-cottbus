#include "board/Board.h"
#include "board/LocalBoard.h"
#include "client/TACOClient.h"
#include "misc/Stopwatch.h"
#include "misc/debug.h"
#include "taco/tacoMain.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <vector>

using namespace std;
namespace po = boost::program_options;

struct client_area {
    int start_x, start_y, end_x, end_y;
};

void taco_calculate_area(int client_id, int client_count, int board_width, int board_height, int &start_x, int &start_y,
                         int &end_x, int &end_y) {
    // rows are evenly distributed among clients.
    // If it can not be evenly distributed, early clients get a row more than later clients.
    // Example: 100 rows, 7 clients, 0 = 15, 1 = 15, 2 to 6 = 14

    int clients = client_count;
    int rows = board_height;

    int rows_per_client = rows / clients;
    int rows_per_client_remainder = rows % clients;

    int already_used_rows = client_id * rows_per_client + min(client_id, rows_per_client_remainder);
    int rows_used_by_this_client = rows_per_client + (client_id < rows_per_client_remainder ? 1 : 0);

    start_x = 0;
    start_y = already_used_rows;
    end_x = board_width;
    end_y = start_y + rows_used_by_this_client;
}

int parse_command_line_args(int argc, char **argv, po::variables_map &vm) {
    // define available arguments
    po::options_description desc("Usage", 1024, 512);
    desc.add_options()                                                                                    //
        ("help,", "Print help message")                                                                   //
        ("input,i", po::value<string>(), "Input file\nMust be in the correct .rle format")                //
        ("output,o", po::value<string>(), "Output file\nExisting files will be overwriten")               //
        ("steps,r", po::value<int>()->default_value(1), "Simulation steps")                               //
        ("width,w", po::value<int>()->default_value(100), "Width of the board\nNot compatible with -i")   //
        ("height,h", po::value<int>()->default_value(100), "Height of the board\nNot compatible with -i") //
        ("profile,", po::value<string>(), "Output file for profiler");                                    //

    // read arguments and store in a map
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional({}).allow_unregistered().run(), vm);
        po::notify(vm);
    } catch (po::error &e) {
        cerr << e.what() << endl;
        cerr << desc << endl;
        return 1;
    }

    // Print help text and exit on --help
    if (vm.count("help")) {
        cerr << "Game of Life TACO" << endl;
        cerr << desc << endl;
        return 1;
    }

    return 0;
}

int validate_variables_map(po::variables_map vm) {
    int steps = vm["steps"].as<int>();
    if (steps <= 0) {
        cerr << "--steps must be greater than 0" << endl;
        return 1;
    }

    int width = vm["width"].as<int>();
    if (width <= 0) {
        cerr << "--width must be greater than 0" << endl;
        return 1;
    }

    int height = vm["height"].as<int>();
    if (height <= 0) {
        cerr << "--height must be greater than 0" << endl;
        return 1;
    }

    return 0;
}

void apply_with_barrier(vector<taco::ObjectPtr<TACOClient>> &taco_clients, vector<taco::Future<bool> *> &barrier,
                        int client_count,
                        taco::Method0<TACOClient, bool, bool (TACOClient::*)(), TACOClient &> functor) {
    barrier.clear();
    for (int i = 0; i < client_count; i++) {
        taco::Future<bool> *sync = new taco::Future<bool>();
        taco_clients[i]->apply(functor, *sync);
        barrier.push_back(sync);
    }

    for (size_t i = 0; i < barrier.size(); i++) {
        barrier[i]->wait();
        delete barrier[i];
    }
}

int main(int argc, char **argv) {
    // only node 0 continues afterwards
    taco::init(argc, argv);

    if (taco::thisNode() != 0) {
        return 0;
    }

    po::variables_map vm;
    if (parse_command_line_args(argc, argv, vm) > 0) {
        return 1;
    }

    if (validate_variables_map(vm) > 0) {
        return 1;
    }

    // create the shared board
    taco::ObjectPtr<LocalBoard> server_board =
        taco::allocate<LocalBoard>(0)(vm["width"].as<int>(), vm["height"].as<int>());
    if (vm.count("input")) {
        server_board->call(taco::m2f(&LocalBoard::importAll, vm["input"].as<string>()));
    } else {
        server_board->call(taco::m2f(&LocalBoard::importAll, "RANDOM"));
    }

    int board_width = server_board->invoke(taco::m2f(&LocalBoard::getWidth));
    int board_height = server_board->invoke(taco::m2f(&LocalBoard::getHeight));
    _DEBUG(cout << "Server board loaded with size " << board_width << " * " << board_height << endl;)

    int client_count = taco::nodes() - 1;
    _DEBUG(cout << "Using " << client_count << " clients for simulation" << endl;)

    if (client_count <= 0) {
        return 0;
    }

    // calculate the client areas
    vector<client_area> client_areas;
    for (int i = 0; i < min(client_count, board_height); i++) {
        client_area area = client_area();
        taco_calculate_area(i, client_count, board_width, board_height, area.start_x, area.start_y, area.end_x,
                            area.end_y);
        client_areas.push_back(area);
        _DEBUG(cout << "Client " << i << " has area from "
                    << "(" << area.start_x << "," << area.start_y << ") to (" << area.end_x << "," << area.end_y << ")"
                    << endl;)
    }

    // create the clients that use the shared board
    vector<taco::ObjectPtr<TACOClient>> taco_clients;
    for (int i = 0; i < min(client_count, board_height); i++) {
        client_area area = client_areas[i];
        taco::ObjectPtr<TACOClient> client =
            taco::allocate<TACOClient>(i + 1)(area.start_x, area.start_y, area.end_x, area.end_y, server_board);
        taco_clients.push_back(client);
        _DEBUG(cout << "TACOClient " << i << " was allocated" << endl;)
    }

    // simulate the gol using the client boards
    Stopwatch stopwatch;
    int step = 0;
    while (step < vm["steps"].as<int>()) {
        if (step == 0)
            stopwatch.start();
        vector<taco::Future<bool> *> barrier;
        apply_with_barrier(taco_clients, barrier, min(client_count, board_height), taco::m2f(&TACOClient::import_area));
        apply_with_barrier(taco_clients, barrier, min(client_count, board_height), taco::m2f(&TACOClient::step));
        apply_with_barrier(taco_clients, barrier, min(client_count, board_height), taco::m2f(&TACOClient::export_area));
        stopwatch.stop();

        _DEBUG(cout << "Step " << step << " done" << endl;)
        step++;
    }

    // export the profiler results, if needed
    if (vm.count("profile")) {
        stopwatch.to_file(vm["profile"].as<string>());
    }

    // export the resulting board
    if (vm.count("output")) {
        server_board->call(taco::m2f(&LocalBoard::exportAll, vm["output"].as<string>()));
    }
    return 0;
}
