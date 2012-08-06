#ifndef __NETWORK_CONNECTIONMANAGER_H__
#define __NETWORK_CONNECTIONMANAGER_H__

#include <memory>
#include <set>

namespace network {
namespace detail {
	class ConnectionManager_impl;
} // namespace detail

	class Connection;
	class Socket;
	class ConnectionManager
	{
		public:
			static ConnectionManager& Singleton();

			void AddConnection(Connection* connection);
			void RemoveConnection(const std::shared_ptr<Connection>& connection);
			std::shared_ptr<Connection>&& FindConnection(const Socket& socket) const;
			const std::set<std::shared_ptr<Connection>>& AllConnections() const;
			void RemoveAllConnections();

		private:
			ConnectionManager();
			~ConnectionManager();

			std::unique_ptr<detail::ConnectionManager_impl> impl_;

			ConnectionManager(const ConnectionManager& rhs) = delete;
			ConnectionManager(ConnectionManager&& rhs) = delete;
			ConnectionManager& operator=(const ConnectionManager& rhs) = delete;
			ConnectionManager& operator=(ConnectionManager&& rhs) = delete;
	};

} // namespace network

#endif
