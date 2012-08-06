#ifndef __NETWORK_DETAIL_MESSAGEQUEUE_IMPL_HPP__
#define __NETWORK_DETAIL_MESSAGEQUEUE_IMPL_HPP__

#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include "MessageQueueTypes.h"

namespace network {
namespace detail {

	struct MessageQueue_impl
	{
		MessageQueue_impl() : is_running_(false) {};
		explicit MessageQueue_impl(CallbackFunc callback) 
			: callback_(callback), is_running_(false) {}
		
		std::vector<std::unique_ptr<Message>> messages_;
		CallbackFunc callback_;
		bool is_running_;
		std::mutex mutex_;
		std::thread thread_;
	};

} // namespace detail
} // namespace network

#endif
