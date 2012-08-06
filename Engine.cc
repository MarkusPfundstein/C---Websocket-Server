#include <iostream>
#include <sstream>
#include "Engine.h"
#include "Detail/Engine_impl.h"
#include "Network/Connection.h"
#include <libjson/libjson.h>

namespace engine {
	
	using namespace detail;
	using namespace network;
	using namespace sql;

	Engine::Engine()
		: impl_(new Engine_impl)
	{
	}

	Engine::~Engine()
	{
	}

	void Engine::OnAcceptCallback(const Connection& connection)
	{
		connection.Send("Hallo");
	}

	void Engine::OnReceiveCallback(const Connection& connection)
	{
		//std::cout << std::this_thread::get_id() << std::endl;
		std::string stream = connection.Stream();

		if (stream.length() > 0) {
			std::ostringstream oss;
			if (std::isdigit(stream[0])) {
				oss << "SELECT * FROM cards WHERE id = ANY (SELECT card_id FROM rel_user_cards WHERE user_id = '" << stream[0] << "');";
			} else if (stream == "all") {
				oss << "SELECT * FROM cards" << std::endl;
			} else {
				connection.Send("Unknown command");
				return;
			}
			Collection collection = impl_->database_.Query(oss.str());
			
			JSONNODE *node = json_new(JSON_NODE);
			JSONNODE *cards = json_new(JSON_ARRAY);
			json_set_name(cards, "all_cards");
			
			std::cout << " Query finished" << std::endl;
			auto it = collection.begin();
			for (; it != collection.end(); ++it) {
				sql::Dictionary dict = *it;
				auto dict_it = dict.begin();
				JSONNODE *card_info = json_new(JSON_NODE);
				for (; dict_it != dict.end(); ++dict_it) {
					json_push_back(card_info, json_new_a(
								JSON_TEXT(dict_it->first.c_str()),
								JSON_TEXT(dict_it->second.c_str())));
				}
				json_push_back(cards, card_info);
			}
			json_push_back(node, cards);

			json_char* json_string = json_write_formatted(node);
			connection.Send(json_string);
			json_free(json_string);

			json_delete(node);
		} else {
			connection.Send("Unknown request");
		}
	}

	void Engine::OnAbortCallback(const Connection& connection)
	{
	}

	bool Engine::Run(const std::string& init_file)
	{
		int port = 56665; // get from init_file
		std::string username = "m4st3r";
		std::string password = "illuminati2012";
		std::string database = "prototype2012";
		std::string host = "ec2-46-137-146-117.eu-west-1.compute.amazonaws.com";

		if (!impl_->database_.Startup(username, password, database, host)) {
			return false;
		}

		std::function<void (const Connection&)> on_accept = std::bind(&Engine::OnAcceptCallback,
				this,
				std::placeholders::_1);
		std::function<void (const Connection&)> on_receive = std::bind(&Engine::OnReceiveCallback,
				this,
				std::placeholders::_1);
		std::function<void (const Connection&)> on_abort = std::bind(&Engine::OnAbortCallback,
				this,
				std::placeholders::_1);

		std::thread reactor_thread(std::bind(&Reactor::Run,
						     std::ref(impl_->reactor_),
						     port,
						     on_accept,
						     on_receive,
						     on_abort));

		impl_->reactor_thread_ = std::move(reactor_thread);

		//impl_->reactor_.Run(port, on_accept, on_receive, on_abort);

		return true;
	}

	void Engine::Stop()
	{
		std::cout << "Engine::Stop()" << std::endl;
		impl_->reactor_.Stop();
		if (impl_->reactor_thread_.joinable()) {
			std::cout << "Engine::Stop() :: join reactor thread" << std::endl;
			impl_->reactor_thread_.join();
		}

		impl_->runloop_queue_.Stop();
	}

	Engine& Engine::Singleton() 
	{
		static Engine singleton;
		return singleton;
	}

}
