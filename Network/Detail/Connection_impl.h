#ifndef __NETWORK_CONNECTION_IMPL_H__
#define __NETWORK_CONNECTION_IMPL_H__

#include <memory>
#include <array>
#include <algorithm>
#include <mutex>
#include "../Socket.h"

namespace network {
namespace detail {

	static const int kBufferSize = 1024;

	struct WebSocketStreamHeader {
		unsigned int header_size;
		int mask_offset;
		unsigned int payload_size;
		bool fin;
		bool masked;
		unsigned char opcode;
		unsigned char res[3];
	};
	
	enum WebSocketOpCode {
		ContinuationFrame = 0x0,
		TextFrame 	  = 0x1,
		BinaryFrame 	  = 0x2,
		ConnectionClose   = 0x8,
		Ping 		  = 0x9,
		Pong 		  = 0xA
	};

	enum WebSocketStreamSize {
		SmallStream = 125,
		MediumStream = 126,
		BigStream = 127
	};

	enum WebSocketHeaderSize {
		SmallHeader = 6,
		MediumHeader = 8,
		BigHeader = 14
	};

	enum WebSocketMaskOffset {
		SmallOffset = 2,
		MediumOffset = 4,
		BigOffset = 10
	};

	struct Connection_impl
	{
		Connection_impl(int sock) 
			: socket_(new Socket(sock)),
			  buffer_(),
			  buffer_len_(0)
		{ 
			std::fill(buffer_.begin(), buffer_.begin() + kBufferSize, 0);
		}

		std::unique_ptr<Socket> socket_;
		std::array<unsigned char, kBufferSize> buffer_;
		std::string stream_;
		int buffer_len_;
	};

} // namespace detail
} // namespace network

#endif
