#include "board/Board.h"
#include "board/BoardServer.h"
#include "board/LocalBoard.h"
#include "gui/BoardDrawingWindow.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/TCPNetwork.h"
#include "net/UDPNetwork.h"
#include <boost/program_options.hpp>

using namespace std;
using namespace GUI;

int main(int argc, char **argv) {
    // configure logger before usage.
    LOGCFG = {};
    LOGCFG.headers = true;
#ifdef DEBUG_MODE
    LOGCFG.level = DEBUG;
#else
    LOGCFG.level = ERROR;
#endif
    // end of logger configuration.

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
        ("clients,c", po::value<int>()->default_value(1), "Required connected clients")                        //
        ("network,n", po::value<int>()->default_value(0), "IP Network type\nTypes:\n  0) UDP\n  1) TCP")       //
        ("profile,", po::value<string>()->default_value(""), "Output file for profiler")                       //
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

    int client_count = vm["clients"].as<int>();
    if (client_count < 0) {
        LOG(ERROR) << "'clients' argument must be greater than 0";
        return 1;
    }

    IPNetwork *net;
    int network_type = vm["network"].as<int>();
    switch (network_type) {
    case 0: {
        net = (IPNetwork *)new UDPNetwork(7654);
        LOG(DEBUG) << "Using UDP";
        break;
    }
    case 1: {
        net = (IPNetwork *)new TCPNetwork(7654, client_count);
        LOG(DEBUG) << "Using TCP";
        break;
    }
    default: {
        LOG(ERROR) << "'network' must be a valid network type";
        return 1;
    }
    }

    string input_path = "RANDOM";
    if (!vm["input"].defaulted()) {
        input_path = vm["input"].as<string>();
        LOG(DEBUG) << "Importing board from " << input_path;
    }

    Board *board_read = new LocalBoard(board_width, board_height);
    Board *board_write = new LocalBoard(board_width, board_height);

    bool import_result = board_read->importAll(input_path);
    if (!import_result) {
        LOG(ERROR) << "Could not import board from file '" << input_path << "'.";
        LOG(ERROR) << "File might be missing, its content might be malformed or permissions to access it are missing.";
        return 1;
    }

    board_write->importAll(vm["input"].as<string>());
    board_write->clear();

    BoardDrawingWindow *window_read = nullptr;
    BoardDrawingWindow *window_write = nullptr;

    if (vm.count("gui")) {
        window_read = new BoardDrawingWindow(board_read, 800, 800);
        window_write = new BoardDrawingWindow(board_write, 800, 800);
    }

    BoardServer *board_server = new BoardServer(net, client_count, board_read, board_write, simulation_steps);
    board_server->start();
    delete board_server;

    if (!vm["output"].defaulted()) {
        LOG(DEBUG) << "Exporting board to " << vm["output"].as<string>();
        board_read->exportAll(vm["output"].as<string>());
    }

    delete window_read;
    delete window_write;
    delete board_server;
    delete net;
    return 0;
}
