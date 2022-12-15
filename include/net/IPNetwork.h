#ifndef IPNETWORK_H
#define IPNETWORK_H

#include "net/IPAddress.h"

class IPNetwork {
  public:
    typedef IPAddress Server;
    typedef IPAddress Client;

    IPNetwork() {}
    virtual ~IPNetwork() {}

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
     * @return the length of the received message
     */
    virtual ssize_t request(const Server &server, void *req, size_t reqlen, void *res, size_t reslen,
                            int timeout = 1) = 0;

    /**
     * Wait the whole time for a message that arrives at a specified port.
     *
     * @param client is the information of the client that sent a request
     * @param req is the buffer that will be send through the network
     * @param reqlen is the length of the buffer
     * @return the length of the received message
     */
    virtual ssize_t receive(Client &client, void *req, size_t reqlen) = 0;

    /**
     * Send a message to the client from that we received a request.
     *
     * @param client is the information of the client that has been sent a request
     * @param res is the buffer that will be send through the network
     * @param reslen is the length of the buffer
     * @return the length of the message that was sent
     */
    virtual ssize_t reply(const Client &client, void *res, size_t reslen) = 0;
};

#endif
