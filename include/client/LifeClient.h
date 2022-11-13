#ifndef LIFECLIENT_H
#define LIFECLIENT_H

#include "net/IPAddress.h"
#include "net/IPNetwork.h"

/*
* This class represents a LifeClient which connects to a boardserver, and then
* calculates the status of determined Boardelements. For communication a network class is used.
*/
class LifeClient
{
public:
	/*
	* Constructor gets a pointer to a IPNetwork object for communication,
	* additionally the contact data for the server.
	*
	* @param net is the network object for communication
	* @param servername is the name of the server for example "ep", not an ip address in ascii-representation
	* @param port is the portnumber the boardserver listens
	*/
	LifeClient(IPNetwork* net, char* servername, short port){};

	~LifeClient(){};


private:
	int x1,x2,y1,y2; 	//outlines the part of the board which should be calculated by the client
	IPAddress server;
	IPNetwork* net;

	/**
	* This function contacts the server and gets the outlines of the boarpart which should be worked on.
	*
	* @return a -1 if logon failed, else 0
	*/
	int startUp();

	/**
	* Loops until server signals end of the simulation, and calls makeStep() for the next simulation cycle
	*/
	void loop();

	/**
	* Calculate status for the next timestep on the given boardpart
	*/
	void makeStep();
};

#endif
