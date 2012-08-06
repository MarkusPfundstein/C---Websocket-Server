#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include "Socket.h"
#include "Detail/Reactor_impl.h"
#include "ConnectionManager.h"
#include "Reactor.h"
#include "Connection.h"
#include "MessageQueue.h"
#include "Message.h"

namespace network {

	using namespace detail;

	Reactor::Reactor()
		: impl_(new Reactor_impl)
	{
		for (int i = 0; i < kCountMessageQueues; ++i) {
			MessageQueue *queue = new MessageQueue;
			queue->SetCallback(std::bind(&Reactor::ExecuteMessage,
						     this,
						     std::placeholders::_1));
			queue->RunAsync();
			impl_->queues_.push_back(std::move(std::unique_ptr<MessageQueue>(queue)));
		}
	}

	Reactor::~Reactor()
	{
		Stop();
	}

	void Reactor::Run(int port, CallbackType on_acc, CallbackType on_rec, CallbackType on_abort)
	{
		impl_->on_accept_callback_ = on_acc;
		impl_->on_receive_callback_ = on_rec;
		impl_->on_abort_callback_ = on_abort;
		InitListener(port);
		RebuildSet();
		Runloop();
	}

	void Reactor::Runloop()
	{
		timeval timeout;
		int sockets_ready = 0;

		fd_set use_set;
		impl_->is_running_ = true;
		while (impl_->is_running_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			use_set = impl_->fds_listen_;
			auto busy_it = impl_->busy_fds_.begin();
			impl_->busy_fds_lock_.lock();
			for (; busy_it != impl_->busy_fds_.end(); ++busy_it) {
				FD_CLR(*busy_it, &use_set);
			}
			impl_->busy_fds_lock_.unlock();
			timeout.tv_sec = 0;
			timeout.tv_usec = 5;
			sockets_ready = select(impl_->high_sock_ + 1, &use_set, nullptr, nullptr, &timeout);
			if (sockets_ready == -1) {
				perror("Reactor::Runloop() - select");
				impl_->is_running_ = false;
			}
			else if (sockets_ready > 0) {
				if (FD_ISSET(impl_->sock_listen_->Native(), &use_set)) {
					std::cout << "New connection" << std::endl;
					AcceptConnection();
				}

				const std::set<std::shared_ptr<Connection>> &connections = ConnectionManager::Singleton().AllConnections();
				auto it = connections.begin();
				for (;it != connections.end(); ) {
					std::cout << "connection check" << std::endl;
					std::shared_ptr<Connection> con = *it;
					it++;
					if (FD_ISSET(con->Socket().Native(), &use_set)) {
						std::cout << "FD ISSET " << std::endl;
						HandleConnection(con);
						impl_->busy_fds_lock_.lock();
						impl_->busy_fds_.push_back(con->Socket().Native());
						impl_->busy_fds_lock_.unlock();
					}
				}
			}
		}
	}

	void Reactor::ExecuteMessage(Message* raw_message)
	{
		Connection::WebSocketError error = Connection::WebSocketError::NoError;
		ReactorMessage *message = dynamic_cast<ReactorMessage*>(raw_message);
		std::shared_ptr<Connection> connection = message->Connection();
		
		std::cout << "START RECEIVE" << std::endl;
		error = connection->Receive();
		std::cout << "Reactor::HandleConnection::WebSocketError: " << error << std::endl;
		if (error == Connection::WebSocketError::NoError) {
			if (impl_->on_receive_callback_) {
				impl_->on_receive_callback_(*connection);
			}
		}
		else if (error == Connection::WebSocketError::Abort) {
			HandleConnectionAbort(connection);
			return;
		} 
		else if (error == Connection::WebSocketError::MessageTooBig) {
			connection->Send("Message too big. I will not handle it fucker");
			HandleConnectionAbort(connection);
		}
		
		impl_->busy_fds_lock_.lock();
		impl_->busy_fds_.remove(connection->Socket().Native());
		impl_->busy_fds_lock_.unlock();
	}

	void Reactor::HandleConnection(const std::shared_ptr<Connection>& connection)
	{
		if (!connection) return;

		std::cout << "HANDLE CONNECTION! !!!!! " << std::endl;
		ReactorMessage *message = new ReactorMessage(connection);
		impl_->queues_[impl_->index_current_queue_]->PostMessage(message);
		std::cout << "PUSH ON QUEUE :  " << impl_->index_current_queue_ << std::endl;

		impl_->index_current_queue_++;
		if (impl_->index_current_queue_ > (int)impl_->queues_.size() -1) {
			impl_->index_current_queue_ = 0;
		}
	}

	void Reactor::HandleConnectionAbort(const std::shared_ptr<Connection>& connection)
	{
		std::cout << "HandleConnectionAbort()" << std::endl;
		if (impl_->on_abort_callback_) {
			impl_->on_abort_callback_(*connection);
		}


		impl_->busy_fds_lock_.lock();
		impl_->busy_fds_.remove(connection->Socket().Native());

		std::cout << "still busy fds: " << std::endl;
		for (auto it = impl_->busy_fds_.begin(); it != impl_->busy_fds_.end(); ++it) {
			std::cout << *it << std::endl;
		}
		impl_->busy_fds_lock_.unlock();

		ConnectionManager::Singleton().RemoveConnection(connection);
		RebuildSet();
	}

	void Reactor::AcceptConnection()
	{
		sockaddr_in conn_addr;
		int addr_len = sizeof(conn_addr);
		int new_sock = accept(impl_->sock_listen_->Native(), (sockaddr*)&conn_addr, (socklen_t*)&addr_len);
		if (new_sock == -1) {
			perror("Reactor::AcceptConnection() - accept");
			errno = 0;
			return;
		}

		std::cout << "New High Sock: " << impl_->high_sock_ << "\n";
		std::cout << "Connection Socket: " << new_sock << std::endl;

		Connection* connection = new Connection(new_sock);
		connection->Socket().SetNonBlock();
		if (connection->Authenticate() == Connection::WebSocketError::NoError) {
			ConnectionManager::Singleton().AddConnection(connection);
			if (impl_->on_accept_callback_) {
				impl_->on_accept_callback_(*connection);
			}
			FD_SET(new_sock, &impl_->fds_listen_);
			if (new_sock > impl_->high_sock_) {
				impl_->high_sock_ = new_sock;
			}
		}
		else {
			connection->Disconnect();
			delete connection;
		}
	}

	void Reactor::Stop()
	{
		if (impl_->sock_listen_->Native() > 0) {
			impl_->sock_listen_->Close();
		}

		impl_->is_running_ = false;
	}

	void Reactor::RebuildSet()
	{
		FD_ZERO(&impl_->fds_listen_);
		FD_SET(impl_->sock_listen_->Native(), &impl_->fds_listen_);

		const std::set<std::shared_ptr<Connection>>& connections = ConnectionManager::Singleton().AllConnections();
		auto it = connections.begin();
		for (; it != connections.end(); ++it) {
			int sock = (*it)->Socket().Native();
			if (sock > 0) {
				FD_SET(sock, &impl_->fds_listen_);
				if (sock > impl_->high_sock_) {
					impl_->high_sock_ = sock;
				}
			}
		}
	}

	bool Reactor::InitListener(int port)
	{
		int sock_id = -1;
		if ((sock_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("Reactor::InitListener() - socket()");
			return false;
		}

		impl_->sock_listen_->Open(sock_id);

		sockaddr_in listen_addr;
		memset(&listen_addr, 0, sizeof(sockaddr_in));
		listen_addr.sin_family = AF_INET;
		listen_addr.sin_port = htons(port);
		listen_addr.sin_addr.s_addr = INADDR_ANY;

		int reuse_addr = 1;
		if (setsockopt(impl_->sock_listen_->Native(), SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) {
			perror("Reactor::InitListener() - setsockopt()");
			close(impl_->sock_listen_->Native());
			return false;
		}

		impl_->sock_listen_->SetNonBlock();

		if (bind(impl_->sock_listen_->Native(), (sockaddr*)&listen_addr, sizeof(sockaddr_in)) == -1) {
			perror("Reactor::InitListener() - bind()");
			close(impl_->sock_listen_->Native());
			return false;
		}

		if (listen(impl_->sock_listen_->Native(), 1024) == -1) {
			perror("Reactor::InitListener() - listen()");
			close(impl_->sock_listen_->Native());
			return false;
		}

		impl_->high_sock_ = impl_->sock_listen_->Native();

		std::cout << "Reactor running on port: " << port << std::endl;

		return true;
	}
}
