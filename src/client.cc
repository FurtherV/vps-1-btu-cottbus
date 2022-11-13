#include "gui/DrawingWindow.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/UDPNetwork.h"
#include "client/LifeClient.h"

int main()
{
	UDPNetwork* net = new UDPNetwork();
	LifeClient* client = new LifeClient((IPNetwork *)net,"localhost",7654);

	//Following code is only a usage example, please use class LifeClient
	//for such kind of implementation
	IPAddress server("localhost",7654);
	char message[10];
	char req[]="hello server";
	net->request(server,(void *)req,13,message,10);

}

