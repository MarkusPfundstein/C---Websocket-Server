#ifndef __NETWORK_REACTOR_TYPES_H__
#define __NETWORK_REACTOR_TYPES_H__

#include <functional>

namespace network {

	class Connection;

	typedef std::function<void (const Connection&)> CallbackType;

} // namespace network

#endif
