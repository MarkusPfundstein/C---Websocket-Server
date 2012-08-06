#include "Socket.h"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>

namespace network {

	Socket::Socket()
		: id_(-1)
	{
	}

	Socket::Socket(int sock) 
		: Socket()
	{
		Open(sock);
	}

	Socket::~Socket()
	{
		if (IsOpen()) {
			Close();
		}
	}

	bool Socket::IsOpen() const
	{
		return id_ > 0;
	}

	void Socket::Close() 
	{
		std::cout << "Close Socket: " << id_ << std::endl;
		shutdown(id_, SHUT_WR);
		shutdown(id_, SHUT_RD);
		close(id_);
		id_ = -1;
	}

	void Socket::Open(int id)
	{
		if (IsOpen() && id_ != id) {
			Close();
		}

		id_ = id;
		std::cout << "Open Socket: " << id_ << std::endl;
	}

	bool Socket::SetNonBlock()
	{
		int sock_opt = fcntl(id_, F_GETFL);
		if (sock_opt < 0) {
			return false;
		}
		sock_opt |= O_NONBLOCK;
		if (fcntl(id_, F_SETFL, sock_opt) < 0) {
			return false;
		}

		return true;
	}

	int Socket::Native() const
	{
		return id_;
	}

} // namespace network
