#include "net/UDPNetwork.h"

UDPNetwork::UDPNetwork(short port)
{
	//open socket
	//bind port
}

UDPNetwork::UDPNetwork()
{
	//open socket
}

ssize_t UDPNetwork::request(const Server& server, void* req, size_t reqlen, void* res, size_t reslen, int timeout)
{
	//clear buffer !!!
	//loop
		//send
		//use select for timeout, see manpages for further information for select
			//if not timeout return data
			//else loop again

	return 0;
}

ssize_t UDPNetwork::receive(Client& client, void* req, size_t reqlen){
	//recvfrom, blocks until a message arrives
	return 0;
}

ssize_t UDPNetwork::reply(const Client& client, void* res, size_t reslen){
	//sendto
	return 0;
}




 
