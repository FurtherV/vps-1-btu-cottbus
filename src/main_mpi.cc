#include "board/BoardServerMPI.h"
#include "board/LocalBoard.h"
#include "client/LifeClientMPI.h"
#include "misc/Log.h"
#include "misc/Stopwatch.h"
#include <algorithm>
#include <boost/program_options.hpp>
#include <cctype>
#include <exception>
#include <mpi.h>
#include <string>

/**
 * @brief Initial method called by the operating system upon executing this program.
 * @param argc amount of arguments
 * @param argv pointer to array of arguments as c-strings
 * @return 0 on success, >1 on error
 */
int main(int argc, char **argv) {
    // parse command line args
    namespace po = boost::program_options;

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
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional({}).allow_unregistered().run(), vm);
        po::notify(vm);
    } catch (po::error &e) {
        LOG(ERROR) << e.what();
        cerr << desc << endl;
    }

    // Print help text and exit on --help
    if (vm.count("help")) {
        cerr << "Game of Life MPI" << endl;
        cerr << desc << endl;
        return 0;
    }

    // store arguments as variables
    std::string input_path = "RANDOM";
    if (vm.count("input")) {
        input_path = vm["input"].as<std::string>();
    }
    std::string output_path = "";
    if (vm.count("output")) {
        output_path = vm["output"].as<std::string>();
    }
    int simulation_steps = vm["steps"].as<int>();
    int board_width = vm["width"].as<int>();
    int board_height = vm["height"].as<int>();
    string profiler_output = "";
    if (vm.count("profile")) {
        profiler_output = vm["profile"].as<std::string>();
    }

    // validate arguments
    if (simulation_steps <= 0) {
        LOG(ERROR) << "'steps' must be greater than 0, was '" << simulation_steps << "'";
        return 1;
    }
    if (board_width <= 0) {
        LOG(ERROR) << "'width' must be greater than 0, was '" << board_width << "'";
        return 1;
    }
    if (board_height <= 0) {
        LOG(ERROR) << "'height' must be greater than 0, was '" << board_height << "'";
        return 1;
    }

    MPI_Init(&argc, &argv);

    try {
        // main code here
        int server_rank = 0;

        int mpi_nodes = 0;
        MPI_Comm_size(MPI_COMM_WORLD, &mpi_nodes);

        if (mpi_nodes <= 1) {
            throw std::runtime_error("This application requires at least two slots!");
        }

        int my_rank = 0;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        if (my_rank == server_rank) {
            // is server
            LocalBoard *board_read = new LocalBoard(board_width, board_height);
            board_read->importAll(input_path);

            LocalBoard *board_write = new LocalBoard(board_width, board_height);
            board_write->clear();

            Stopwatch stopwatch;

            BoardServerMPI server = BoardServerMPI(board_read, board_write, simulation_steps);
            server.start(&stopwatch);

            if (output_path.length() > 0) {
                board_read->exportAll(output_path);
            }

            if (profiler_output.length() > 0) {
                stopwatch.to_file(profiler_output);
            }

            delete board_read;
            delete board_write;
        } else {
            // is client
            LifeClientMPI client = LifeClientMPI(server_rank);
            client.start();
        }
    } catch (const std::exception &e) {
        // error handling here
        LOG(ERROR) << e.what();
        MPI_Finalize();
        return 1;
    }

    MPI_Finalize();
    return 0;
}
