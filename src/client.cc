#include "board/LocalBoard.h"
#include "client/LifeClient.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/TCPNetwork.h"

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

    IPNetwork *net = (IPNetwork *)new TCPNetwork();
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
