#ifndef __NETWORK_REACTOR_DETAIL_CONNECTIONMANAGER_H__
#define __NETWORK_REACTOR_DETAIL_CONNECTIONMANAGER_H__

#include <set>
#include <memory>

namespace network {
	class Connection;
namespace detail {

	struct ConnectionManager_impl
	{
		ConnectionManager_impl() {}

		std::set<std::shared_ptr<Connection>> connections_;
	};

} // namespace detail
} // namespace network

#endif
