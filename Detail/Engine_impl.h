#ifndef __ENGINE_ENGINE_IMPL_H__
#define __ENGINE_ENGINE_IMPL_H__

#include <thread>
#include "../Network/MessageQueue.h"
#include "../Network/Reactor.h"
#include "../sql/Database.h"

namespace engine {
namespace detail {

	struct Engine_impl
	{
		Engine_impl() {}

		std::thread reactor_thread_;
		sql::Database database_;
		network::Reactor reactor_;
		network::MessageQueue runloop_queue_;
	};

} // namespace detail
} // namespace engine

#endif
