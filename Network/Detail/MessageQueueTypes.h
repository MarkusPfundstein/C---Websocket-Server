#ifndef __STREAM_MESSAGEQUEUE_TYPES_HPP__
#define __STREAM_MESSAGEQUEUE_TYPES_HPP__

#include <functional>

namespace network {
	struct Message;
namespace detail {

	typedef Message* MessagePtr;
	typedef Message& MessageRef;
	typedef std::function<void (MessagePtr)> CallbackFunc;

} // namespace detail
} // namespace stream

#endif
