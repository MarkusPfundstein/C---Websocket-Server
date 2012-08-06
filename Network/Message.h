#ifndef __NETWORK_MESSAGE_H__
#define __NETWORK_MESSAGE_H__

namespace network {

	struct Message
	{
		Message() = default;
		Message(const Message& rhs) = default;
		Message(Message&& rhs) = default;
		virtual Message& operator=(const Message& rhs) = default;
		virtual Message& operator=(Message&& rhs) = default;
		virtual ~Message() = default;
	};

} // namespace network

#endif
