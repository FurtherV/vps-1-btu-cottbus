#ifndef UDPNETWORK_H
#define UDPNETWORK_H

#include "net/IPNetwork.h"

/**
 * The UdpNetwork Class will be used to communicate through the network.
 *
 * every message to a server or to a client shall be send with the help of this class
 */
class UDPNetwork : IPNetwork {
  public:
    /**
     * Create a network object with a specified port. This is necessary for the server so that it can
     * offer a service.
     *
     * @param port is the port that the server will be bound to
     */
    UDPNetwork(short port);

    /**
     * Create a network object. A client does not need to specify a port.
     */
    UDPNetwork();

    /**
     * Destructor which closes the socket, if it is open.
     */
    ~UDPNetwork();

    /**
     * Every time a client wants to communicate to the server it has to do a request. It sends its message
     * with the command that shall be executed and waits for the answer of the server.
     *
     * @param server to whom the connection should be done
     * @param req is the buffer that will be send through the network
     * @param reqlen is the length of the buffer
     * @param res is the buffer with the answer of the server
     * @param reslen is the length of the buffer "res"
     * @param timeout is the time (in seconds) that we wait for an answer
     * @return On success, the length of the received message is returned. On error, -1 is returned.
     */
    ssize_t request(const Server &server, void *req, size_t reqlen, void *res, size_t reslen, int timeout = 1);

    /**
     * Wait the whole time for a message that arrives at a specified port.
     *
     * @param client is the information of the client that sent a request
     * @param req is the buffer that will be send through the network
     * @param reqlen is the length of the buffer
     * @return On success, the length of the received message is returned. On error, -1 is returned.
     */
    ssize_t receive(Client &client, void *req, size_t reqlen);

    /**
     * Send a message to the client from that we received a request.
     *
     * @param client is the information of the client that has been sent a request
     * @param res is the buffer that will be send through the network
     * @param reslen is the length of the buffer
     * @return On success, the length of the send message is returned. On error, -1 is returned.
     */
    ssize_t reply(const Client &client, void *res, size_t reslen);

  private:
    int socket_fd = -1;
};

#endif
