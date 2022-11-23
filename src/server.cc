#include <boost/program_options.hpp>

#include "board/Board.h"
#include "board/BoardServer.h"
#include "board/LocalBoard.h"
#include "gui/BoardDrawingWindow.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/TCPNetwork.h"

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
    desc.add_options()                                                                                    //
        ("help,", "Print help message")                                                                   //
        ("input,i", po::value<string>()->default_value(""), "Input file")                                 //
        ("output,o", po::value<string>()->default_value(""), "Output file")                               //
        ("steps,r", po::value<int>()->default_value(1), "Simulation steps")                               //
        ("width,w", po::value<int>()->default_value(100), "Width of the board. Not compatible with -i")   //
        ("height,h", po::value<int>()->default_value(100), "Height of the board. Not compatible with -i") //
        ("clients,c", po::value<int>()->default_value(1), "Required connected clients")                   //
        ("gui,g", "Enable GUI");                                                                          //

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

    // did someone say 'help'?
    if (vm.count("help")) {
        cerr << "Game of Life Server" << endl;
        cerr << desc << endl;
        return 0;
    }

    int board_width = vm["width"].as<int>();
    if (board_width <= 0) {
        LOG(ERROR) << "width parameter must be greater than 0";
        return 1;
    }

    int board_height = vm["height"].as<int>();
    if (board_height <= 0) {
        LOG(ERROR) << "'height' parameter must be greater than 0";
        return 1;
    }

    int simulation_steps = vm["steps"].as<int>();
    if (simulation_steps < 0) {
        LOG(ERROR) << "'steps' parameter must be equal to or greater than 0";
        return 1;
    }

    int client_count = vm["clients"].as<int>();
    if (client_count < 0) {
        LOG(ERROR) << "'clients' parameter must be equal to or greater than 0";
        return 1;
    }
    if (client_count == 0) {
        LOG(WARN) << "'clients' parameter was zero, simulation will be skipped";
    }

    IPNetwork *net = (IPNetwork *)new TCPNetwork(7654, client_count);
    Board *board_read = new LocalBoard(board_width, board_height);
    Board *board_write = new LocalBoard(board_width, board_height);
    if (!vm["input"].defaulted()) {
        LOG(DEBUG) << "Importing board from " << vm["input"].as<string>();
        board_read->importAll(vm["input"].as<string>());
        board_write->importAll(vm["input"].as<string>());
        board_write->clear();
    }

    if (vm.count("gui")) {
        BoardDrawingWindow *window_read = new BoardDrawingWindow(board_read, 800, 800);
        BoardDrawingWindow *window_write = new BoardDrawingWindow(board_write, 800, 800);
    }
    if (simulation_steps > 0 && client_count > 0) {
        BoardServer *board_server = new BoardServer(net, client_count, board_read, board_write, simulation_steps);
        board_server->start();
    }

    if (!vm["output"].defaulted()) {
        LOG(DEBUG) << "Exporting board to " << vm["output"].as<string>();
        board_read->exportAll(vm["output"].as<string>());
    }

    delete net;
    return 0;
}
