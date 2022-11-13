#include "gui/DrawingWindow.h"
#include "net/IPAddress.h"
#include "net/IPNetwork.h"
#include "net/UDPNetwork.h"
#include "board/Board.h"
#include "board/LocalBoard.h"
#include "board/BoardServer.h"

int main()
{
	UDPNetwork* net = new UDPNetwork(7654);
	LocalBoard* board_a = new LocalBoard(100,100);
	LocalBoard* board_b = new LocalBoard(100,100);
	BoardServer* boardserver = new BoardServer((IPNetwork *)net,4,(Board *)board_a,(Board *)board_b);

	//Following code is only a usage example, please use class BoardServer
	//for such kind of implementation
	IPAddress client;
	char message[20];
	net->receive(client,message,20);
	char rep[]="hello";
	net->reply(client,rep,10);
}

