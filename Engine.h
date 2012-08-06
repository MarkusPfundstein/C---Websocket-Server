#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <memory>
#include <thread>

namespace network {
	class Connection;
} // namespace network;

namespace engine {
namespace detail {
	class Engine_impl;
} // namespace detail

	class Engine
	{
		public:
			static Engine& Singleton();

			bool Run(const std::string& init_file);
			void Stop();

		private:
			std::unique_ptr<detail::Engine_impl> impl_;

			void OnAcceptCallback(const network::Connection& connection);
			void OnReceiveCallback(const network::Connection& connection);
			void OnAbortCallback(const network::Connection& connection);

		public:
			Engine();
			~Engine();
		
		private:
			Engine(const Engine& rhs) = delete;
			Engine(Engine&& rhs) = delete;
			Engine& operator=(const Engine& rhs) = delete;
			Engine& operator=(Engine&& rhs) = delete;
	};

} // namespace engine

#endif
