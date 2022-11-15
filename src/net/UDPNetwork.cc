#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "net/UDPNetwork.h"
#include "misc/Log.h"

UDPNetwork::UDPNetwork(short port)
{
	// open socket
	// bind port
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd == -1)
	{
		LOG(ERROR) << "Socket could not be created (" << socket_fd << ")";
		LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
		return;
	}

	IPAddress ipaddress = IPAddress(7654);
	int bind_result = bind(socket_fd, (const sockaddr *)&ipaddress, sizeof(ipaddress));
	if (bind_result == -1)
	{
		LOG(ERROR) << "Socket " << socket_fd << " could not be bound using bind()";
		LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
		return;
	}
}

UDPNetwork::UDPNetwork()
{
	// open socket
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd == -1)
	{
		LOG(ERROR) << "Socket could not be created (" << socket_fd << ")";
		LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
		return;
	}
}

ssize_t UDPNetwork::request(const Server &server, void *req, size_t reqlen, void *res, size_t reslen, int timeout)
{
	if (socket_fd == -1)
	{
		LOG(ERROR) << "No socket exists";
		return -1;
	}

	socklen_t server_address_len = sizeof(server);

	// clear buffer !!!
	bzero(res, reslen);
	// loop
	while (true)
	{
		// send
		ssize_t send_bytes = sendto(socket_fd, req, reqlen, 0, (const sockaddr *)&server, server_address_len);
		if (send_bytes == -1)
		{
			LOG(ERROR) << "Could not send a request to the server";
			LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
		}

		// use select for timeout, see manpages for further information for select
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(socket_fd, &rset);
		timeval select_timeout;
		select_timeout.tv_usec = 0;
		select_timeout.tv_sec = timeout;
		int nready = select(socket_fd + 1, &rset, NULL, NULL, &select_timeout);
		if (nready == -1)
		{
			LOG(ERROR) << "Could not monitor file descriptor with select";
			LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
		}

		if (FD_ISSET(socket_fd, &rset))
		{
			// if not timeout return data
			ssize_t received_bytes = recvfrom(socket_fd, res, reslen, 0, (sockaddr *)&server, &server_address_len);
			if (received_bytes == -1)
			{
				LOG(ERROR) << "Could not receive answer from server";
				LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
			}
			return received_bytes;
		}
		LOG(DEBUG) << "Request timeout exceeded, retry";
		// else loop again
	}

	return 0;
}

ssize_t UDPNetwork::receive(Client &client, void *req, size_t reqlen)
{
	if (socket_fd == -1)
	{
		LOG(ERROR) << "No socket exists";
		return -1;
	}

	socklen_t size_of_client = sizeof(client);

	// recvfrom, blocks until a message arrives
	ssize_t received_bytes = recvfrom(socket_fd, req, reqlen, 0, (sockaddr *)&client, &size_of_client);
	if (received_bytes == -1)
	{
		LOG(ERROR) << "Could not receive answer from client";
		LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
	}
	return received_bytes;
}

ssize_t UDPNetwork::reply(const Client &client, void *res, size_t reslen)
{
	if (socket_fd == -1)
	{
		LOG(ERROR) << "No socket exists";
		return -1;
	}

	// sendto
	ssize_t send_bytes = sendto(socket_fd, res, reslen, 0, (sockaddr *)&client, sizeof(client));
	if (send_bytes == -1)
	{
		LOG(ERROR) << "Could not send message to client";
		LOG(ERROR) << "Error: " << errno << " - " << strerror(errno);
	}
	return send_bytes;
}
