#ifndef BOARDSERVER_H
#define BOARDSERVER_H

#include "board/Board.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"

class BoardServer
{
public:
	/**
	 * Constructor gets an IPNetwork over which communication will take 
	 * place.
	 *
	 * @param net is a pointer to a network object for communication
	 * @param clientcount is the number of clients, which will register
	 * @param board_a is just a Board
	 * @param board_b is just another Board
	 * @param timesteps is the amount simulationcycles, 0 means infinity timesteps
	 */
	BoardServer(IPNetwork* net,int clientcount, Board* board_a, Board* board_b, int timesteps=0)
	{
	};

	/**
	 * Frees all memory, which was allocated with new inside this class.
	 */
	~BoardServer(){};

	void start();

private:
	int timestep;
	int timesteps;
	IPNetwork* net;
	Board* board_a;
	Board* board_b; 	//one board for timestep n for reading, and one for timestep n+1 for writing
	//List<IPAddress *> addresses;    //any kind of list of client addresses

	/**
	 * This function is called, when a client connects to the server for the first time.
	 * It stores the address of the client and determines the part of the board the client
	 * has to work on.
	 */
	void logon();

	/**
	 * This function is the global barrier for interstepsynchronization. All clients must call
	 * this function, before any of them gets a reply from the server. The reply can tell a client to
	 * calculate the next timestep or inform him of the end of simulation.
	 *
	 * @param ip is the address of the calling client
	 * @param timestep is the finished timestep which the client is signalling
	 */
	void barrier(IPAddress* ip,int timestep);

	/**
	 * This function tells a client, that it can continue to work or that the end of the simulation is reached.
	 *
	 * @param ip is the address of the client which should be notified
	 */
	void notify(IPAddress* ip);

	/**
	 * The more general notify, which sends the same notification to all clients.
	 */
	void notifyAll();

};

#endif
