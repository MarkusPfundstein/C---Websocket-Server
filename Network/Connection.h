#ifndef __NETWORK_CONNECTION_H__
#define __NETWORK_CONNECTION_H__

#include <memory>
#include <string>

namespace network {
namespace detail {
	struct Connection_impl;
	struct WebSocketStreamHeader;
} // namespace detail

	class Socket;
	class Connection
	{
		public:
			enum WebSocketError {
				NoError = 0,
				InvalidSize = 1,
				InvalidType = 2,
				Abort = 2, 
				NotReady = 3,
				MessageTooBig = 4
			};

			explicit Connection(int sock);
			~Connection();

			network::Socket& Socket() const;
			void Disconnect();
		
			const std::string& Stream() const;

			WebSocketError Authenticate();
			WebSocketError Receive();
		  	WebSocketError Send(const std::string& stream) const;

		private:
			WebSocketError Read();
			WebSocketError Read(int max);
			WebSocketError Read(int max, int min);
			
			WebSocketError ReadHTMLPacket(std::string *stream);
			WebSocketError ReadStream(detail::WebSocketStreamHeader *header);
			bool DecodeRawStreamData(const detail::WebSocketStreamHeader& header);

			WebSocketError Write(const std::string& str) const;
			WebSocketError Write(const unsigned char* bits, long long len) const;

			void ResetBuffer();			
			WebSocketError ReadHeader(detail::WebSocketStreamHeader *header);

			std::unique_ptr<detail::Connection_impl> impl_;
			
			Connection(const Connection& rhs) = delete;
			Connection(Connection&& rhs) = delete;
			Connection& operator=(const Connection& rhs) = delete;
			Connection& operator=(Connection&& rhs) = delete;
	};

} // namespace network

#endif
