#ifndef __NETWORK_REACTOR_H__
#define __NETWORK_REACTOR_H__

#include <memory>
#include "ReactorTypes.h"

namespace network {
namespace detail {
	class Reactor_impl;
} // namespace detail

	class Message;
	class Connection;
	class Reactor
	{
		public:
			Reactor();
			~Reactor();

			void Run(int port, CallbackType on_acc, CallbackType on_rec, CallbackType on_abort);
			void Stop();

		private:
			std::unique_ptr<detail::Reactor_impl> impl_;
			
			void Runloop();
			void RebuildSet();
			void AcceptConnection();
			void HandleConnection(const std::shared_ptr<Connection>& connection);
			void HandleConnectionAbort(const std::shared_ptr<Connection>& connection);
			bool InitListener(int port);
			void ExecuteMessage(Message* message);
			
			Reactor(const Reactor& rhs) = delete;
			Reactor(Reactor&& rhs) = delete;
			Reactor& operator=(const Reactor& rhs) = delete;
			Reactor& operator=(Reactor&& rhs) = delete;
	};

} // namespace detail

#endif
