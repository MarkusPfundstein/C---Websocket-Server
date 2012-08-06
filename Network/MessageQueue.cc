#include <chrono>
#include "MessageQueue.h"
#include "Detail/MessageQueue_impl.h"
#include "Message.h"

namespace network {

	using namespace detail;

	MessageQueue::MessageQueue()
		: impl_(new MessageQueue_impl)
	{
	}

	MessageQueue::MessageQueue(CallbackFunc callback)
		: impl_(new MessageQueue_impl(callback))
	{
	}

	MessageQueue::~MessageQueue()
	{
		if (impl_->is_running_) {
			Stop();
		}
		if (impl_->thread_.joinable()) {
			impl_->thread_.join();
		}
	}

	bool MessageQueue::RunAsync() 
	{
		if (!impl_->callback_ || impl_->is_running_) return false;

		std::thread process_thread(std::bind(&MessageQueue::Run, this));
		impl_->thread_ = std::move(process_thread);
		
		return true;
	}

	bool MessageQueue::RunAsync(CallbackFunc callback)
	{
		SetCallback(callback);
		return RunAsync();
	}

	void MessageQueue::Stop()
	{
		std::lock_guard<std::mutex> lock(impl_->mutex_);
		impl_->is_running_ = false;
	}

	void MessageQueue::SetCallback(CallbackFunc callback)
	{
		std::lock_guard<std::mutex> lock(impl_->mutex_);
		impl_->callback_ = callback;
	}

	void MessageQueue::PostMessage(Message* message)
	{
		std::lock_guard<std::mutex> lock(impl_->mutex_);
		impl_->messages_.push_back(std::unique_ptr<Message>(message));
	}
	
	void MessageQueue::Update()
	{
		std::lock_guard<std::mutex> lock(impl_->mutex_);
		auto it = impl_->messages_.begin();
		for (; it != impl_->messages_.end(); ++it) {
			impl_->callback_(it->get());
		}
		impl_->messages_.clear();
	}

	bool MessageQueue::Run()
	{
		if (!impl_->callback_ || impl_->is_running_) return false;

		impl_->is_running_ = true;
		while (impl_->is_running_) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			Update();
		}

		return true;
	}

} // namespace network
