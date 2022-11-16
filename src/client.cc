#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/UDPNetwork.h"
#include "client/LifeClient.h"

#include "misc/Log.h"

int main()
{
	// Configure logger before usage.
	LOGCFG = {};
	LOGCFG.headers = true;
#ifdef DEBUG_MODE
	LOGCFG.level = DEBUG;
#else
	LOGCFG.level = ERROR;
#endif
	// End of configuration.

	UDPNetwork *net = new UDPNetwork();
	// LifeClient *client = new LifeClient((IPNetwork *)net, (char *)"localhost", 7654);

	// Following code is only a usage example, please use class LifeClient
	// for such kind of implementation
	IPAddress server("localhost", 7654);
	char message[10];
	char req[] = "hello server";
	LOG(DEBUG) << "Sending '" << req << "' to server";
	net->request(server, (void *)req, 13, message, 10);
	LOG(DEBUG) << "Server answered: " << message;
	cin.get();
}
