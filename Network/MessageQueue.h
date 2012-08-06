#ifndef __NETWORK_MESSAGEQUEUE_HPP__
#define __NETWORK_MESSAGEQUEUE_HPP__

#include <memory>
#include "Detail/MessageQueueTypes.h"

namespace network {
namespace detail {
	class MessageQueue_impl;
} // namespace detail

	typedef detail::MessagePtr MessagePtr;

	class MessageQueue
	{
		public:
			MessageQueue();
			explicit MessageQueue(detail::CallbackFunc callback);
			~MessageQueue();

			bool Run();
			bool RunAsync();
			bool RunAsync(detail::CallbackFunc callback);

			void Update();
			void PostMessage(Message* message);
			void SetCallback(detail::CallbackFunc callback);

			void Stop();
			
		private:
			std::unique_ptr<detail::MessageQueue_impl> impl_;

		private:
			MessageQueue(const MessageQueue& rhs) = delete;
			MessageQueue(MessageQueue&& rhs) = delete;
			MessageQueue& operator=(const MessageQueue& rhs ) = delete;
			MessageQueue& operator=(MessageQueue&& rhs) = delete;


	};

} // namespace network

#endif
