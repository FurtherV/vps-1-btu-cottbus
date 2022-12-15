#include "net/TCPNetwork.h"
#include "misc/Log.h"
#include <exception>
#include <sys/socket.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

TCPNetwork::TCPNetwork(short port, int queue_size) {
    // create server socket and bind it to the address
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw std::system_error(errno, std::generic_category(), "Could not create server socket");
    }

    // allow socket to reuse IP / Port of another socket which is closed but still in TIME_WAIT state

    int optval = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    IPAddress ipaddress = IPAddress(port);

    // try for 60 tries to bind the socket to the address and port, if the port is still in used, retry after 1 second.
    // On other errors, and if this takes too long, throw an exception
    int retries = 0;
    int max_retries = 60;
    while (retries < max_retries) {
        if (bind(socket_fd, (const sockaddr *)&ipaddress, sizeof(ipaddress)) == 0) {
            break;
        } else {
            if (errno == EADDRINUSE || errno == 98) {
                retries++;
                sleep(1);
            }
            continue;
        }
        throw std::system_error(errno, std::generic_category(), "Could not bind server socket");
    }

    if (listen(socket_fd, queue_size) != 0) {
        throw std::system_error(errno, std::generic_category(),
                                "Could not make server socket listen for incoming connections");
    }
}

TCPNetwork::TCPNetwork() {
    // client does not use socket_fd but connections in case there are multiple servers
}

TCPNetwork::~TCPNetwork() {
    // close all open sockets
    close(socket_fd);
    for (auto elem : connections) {
        close(elem.second);
    }
}

ssize_t TCPNetwork::request(const Server &server, void *req, size_t reqlen, void *res, size_t reslen, int timeout) {
    // clear buffer
    bzero(res, reslen);

    // check if socket exists, if not create connection with server
    if (socket_fd == -1) {
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            throw std::system_error(errno, std::generic_category(), "Could not create socket");
        }

        socklen_t server_address_len = sizeof(server);
        while (connect(socket_fd, (const sockaddr *)&server, server_address_len) != 0) {
            if (errno == ECONNREFUSED) {
                LOG(INFO) << "Server does not exist, waiting...";
                sleep(timeout);
            } else {
                throw std::system_error(errno, std::generic_category(), "Could not connect to the server");
            }
        }
    }

    // send request
    ssize_t send_bytes = send(socket_fd, req, reqlen, 0);

    if (send_bytes < 0) {
        throw std::system_error(errno, std::generic_category(), "Could not send request to server");
    }

    // receive reply (wait until socket has one though)
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(socket_fd, &rset);

    if (select(socket_fd + 1, &rset, NULL, NULL, NULL) < 0) {
        throw std::system_error(errno, std::generic_category(), "Could not monitor socket descriptors");
    }

    if (FD_ISSET(socket_fd, &rset)) {
        ssize_t received_bytes = recv(socket_fd, res, reslen, 0);
        if (received_bytes < 0) {
            throw std::system_error(errno, std::generic_category(), "Could not receive reply from server");
        }
        return received_bytes;
    }

    // our connection must be the one set in the FD set, otherwise it was malformed.
    throw std::logic_error("Select returned an impossible network state, possibly malformed fd_set");
}

ssize_t TCPNetwork::receive(Client &client, void *req, size_t reqlen) {
    // clear buffer
    bzero(req, reqlen);

    int max_fd = socket_fd;
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(socket_fd, &rset);
    for (auto elem : connections) {
        FD_SET(elem.second, &rset);
        max_fd = max(max_fd, elem.second);
    }

    if (select(max_fd + 1, &rset, NULL, NULL, NULL) < 0) {
        throw std::system_error(errno, std::generic_category(), "Could not monitor socket descriptors");
    }

    if (FD_ISSET(socket_fd, &rset)) {
        // accept new connection on listen socket and store it
        socklen_t size_of_client = sizeof(client);
        int connection_fd = accept(socket_fd, (sockaddr *)&client, &size_of_client);
        if (connection_fd < 0) {
            throw std::system_error(errno, std::generic_category(), "Could not accept connection from client");
        }
        connections[client] = connection_fd;

        ssize_t received_bytes = recv(connection_fd, req, reqlen, 0);

        if (received_bytes < 0) {
            throw std::system_error(errno, std::generic_category(), "Could not receive request from client");
        }
        return received_bytes;

    } else {
        // retrieve first connection that send us something
        int connection_fd = -1;
        for (auto elem : connections) {
            if (FD_ISSET(elem.second, &rset)) {
                connection_fd = elem.second;
                client.setAddr(elem.first.getAddr());
                client.setPort(elem.first.getPort());
                break;
            }
        }

        ssize_t received_bytes = recv(connection_fd, req, reqlen, 0);

        if (received_bytes < 0) {
            throw std::system_error(errno, std::generic_category(), "Could not receive request from client");
        }
        return received_bytes;
    }

    // must be on of our connections to clients or the listening socket, otherwise some socket magically created itself
    throw std::logic_error("Select returned an impossible network state, possibly malformed fd_set or magic socket");
}

ssize_t TCPNetwork::reply(const Client &client, void *res, size_t reslen) {
    auto search = connections.find(client);
    if (search == connections.end()) {
        throw std::invalid_argument("Given client is not known");
    }

    int connection_fd = search->second;
    ssize_t send_bytes = send(connection_fd, res, reslen, 0);

    return send_bytes;
}
