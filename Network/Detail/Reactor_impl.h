#ifndef __NETWORK_DETAIL_REACTOR_IMPL_H__
#define __NETWORK_DETAIL_REACTOR_IMPL_H__

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <memory>
#include <vector>
#include <functional>
#include <list>
#include <mutex>
#include "../Socket.h"
#include "../ReactorTypes.h"
#include "../Message.h"

namespace network {
	class Socket;
	class Connection;
	class MessageQueue;
namespace detail {

	class ReactorMessage
		: public Message
	{
		public:
			explicit ReactorMessage(const std::shared_ptr<network::Connection>& connection)
				: connection_(connection) {}

			std::shared_ptr<network::Connection> Connection()
			{
				return connection_;
			}

		private:
			std::shared_ptr<network::Connection> connection_;
	};

	static const int kCountMessageQueues = 5;

	struct Reactor_impl
	{
		Reactor_impl()
			: sock_listen_(new Socket),
			  index_current_queue_(0),
			  high_sock_(0),
			  is_running_(false) {}

		CallbackType on_accept_callback_;
		CallbackType on_receive_callback_;
		CallbackType on_abort_callback_;
		fd_set fds_listen_;
		std::unique_ptr<Socket> sock_listen_;
		std::vector<std::unique_ptr<MessageQueue>> queues_;
		int index_current_queue_;
		int high_sock_;
		bool is_running_;
		std::list<int> busy_fds_;
		std::mutex busy_fds_lock_;
	};

} // namespace detail
} // namespace network

#endif
