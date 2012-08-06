#include "ConnectionManager.h"
#include "Detail/ConnectionManager_impl.h"
#include "Connection.h"
#include "Socket.h"
#include <algorithm>

namespace network {

	using namespace detail;

	ConnectionManager::ConnectionManager()
		: impl_(new ConnectionManager_impl)
	{
	}

	ConnectionManager::~ConnectionManager()
	{
		RemoveAllConnections();
	}

	std::shared_ptr<Connection>&& ConnectionManager::FindConnection(const Socket& socket) const
	{
		std::shared_ptr<Connection> find_me;

		return std::move(find_me);
	}

	const std::set<std::shared_ptr<Connection>>& ConnectionManager::AllConnections() const
	{
		return impl_->connections_;
	}

	void ConnectionManager::AddConnection(Connection* connection)
	{
		impl_->connections_.insert(std::shared_ptr<Connection>(connection));
	}

	void ConnectionManager::RemoveConnection(const std::shared_ptr<Connection>& connection)
	{
		connection->Disconnect();
		impl_->connections_.erase(connection);
	}

	void ConnectionManager::RemoveAllConnections()
	{
		std::for_each(impl_->connections_.begin(), impl_->connections_.end(), [](std::shared_ptr<Connection> con) {
					con->Disconnect();
				});
	}

	ConnectionManager& ConnectionManager::Singleton()
	{
		static ConnectionManager manager;
		return manager;
	}

} // namespace network
