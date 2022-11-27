#include "board/LocalBoard.h"
#include "client/LifeClient.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/TCPNetwork.h"
#include "net/UDPNetwork.h"
#include <boost/program_options.hpp>

#include "misc/Log.h"

int main(int argc, char **argv) {
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
    desc.add_options()                                                                                //
        ("help,", "Print help message")                                                               //
        ("network,n", po::value<int>()->default_value(0), "IP Network type\nTypes:\n0) UDP\n1) TCP"); //

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

    IPNetwork *net;
    int network_type = vm["network"].as<int>();
    switch (network_type) {
    case 0: {
        net = (IPNetwork *)new UDPNetwork();
        LOG(DEBUG) << "Using UDP";
        break;
    }
    case 1: {
        net = (IPNetwork *)new TCPNetwork();
        LOG(DEBUG) << "Using TCP";
        break;
    }
    default: {
        LOG(ERROR) << network_type << " is not a valid network type";
        return 1;
    }
    }

    LifeClient *life_client = new LifeClient(net, "localhost", 7654);
    int status = life_client->start();
    if (status != 0) {
        LOG(ERROR) << "Could not start GoL LifeClient";
    } else {
        life_client->loop();
    }

    delete life_client;
    return 0;
}
