#include "../MessageQueue.h"
#include "../Message.h"
#include <string>
#include <thread>
#include <iostream>

using namespace network;

bool g_is_running;
MessageQueue g_queue;

class BoolMessage
	: public Message
{
	public:
		explicit BoolMessage(bool s)
			: success_(s) {}

		inline bool success() const
		{
			return success_;
		}

	private:
		bool success_;
};

class StreamMessage 
	: public Message
{
	public:
		explicit StreamMessage(std::string stream)
			: stream_(std::move(stream)) {}
	
		const std::string& stream() const 
		{
			return stream_;
		}
	private:
		std::string stream_;
};

void QueueCallback(MessagePtr message)
{
	std::cout << "Callback Thread: " << std::this_thread::get_id() << std::endl;
	StreamMessage* msg = dynamic_cast<StreamMessage*>(message);
	if (msg) {
		std::cout << "StreamMessage: "; 
		std::cout << msg->stream() << std::endl;
	}
	BoolMessage* bool_msg = dynamic_cast<BoolMessage*>(message);
	if (bool_msg) {
		std::cout << "BoolMessage: " << bool_msg->success() << std::endl;
	} 
}

void QueueRunloop()
{
	g_queue.Run();
}

void HandleInput()
{
	std::cout << "Input Thread: " << std::this_thread::get_id() << std::endl;
	std::string input_buffer;
	while (1) {
		std::cout << "Type something: " << std::endl;
		std::getline(std::cin, input_buffer);
		if (input_buffer == "q") break;
		g_queue.PostMessage(new StreamMessage(input_buffer));
	}

	g_is_running = false;
	g_queue.Stop();
}

void DisturbQueue()
{
	std::cout << "Disturb Thread: " << std::this_thread::get_id() << std::endl;
	long long counter = 0;
	while (g_is_running) {
		if (++counter > 5000000) {
			g_queue.PostMessage(new BoolMessage(rand()%2));
			counter = 0;
		}
	}
}

int main(int argc, char **argv)
{
	std::cout << "Main Thread: " << std::this_thread::get_id() << std::endl;

	//g_queue.SetCallback(&QueueCallback);
	g_queue.RunAsync(&QueueCallback);

	std::thread io_thread(HandleInput);
	//std::thread disturber(DisturbQueue);
	//std::thread queue_thread(QueueRunloop);

	g_is_running = true;
	while (g_is_running) {
		//g_queue.Update();
	}

	io_thread.join();
	//disturber.join();
	//queue_thread.join();

	std::cout << "bye bye" << std::endl;

	return 0;
}
