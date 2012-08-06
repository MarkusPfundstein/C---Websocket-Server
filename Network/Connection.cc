#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <openssl/sha.h>
#include <string>
#include <thread>
#include <chrono>
#include <endian.h>
#include "Base64.h"
#include "Connection.h"
#include "Detail/Connection_impl.h"

namespace network {

	using namespace detail;

	Connection::Connection(int sock)
		: impl_(new Connection_impl(sock))
	{
	}

	Connection::~Connection()
	{
	}

	Connection::WebSocketError Connection::Send(const std::string& stream) const
	{
		long long data_len = stream.length();
		unsigned char *bits = new unsigned char[10 + data_len];
		memset(bits, 0, 10 + data_len);

		bits[0] = 129;
		int mask_offset = 0;

		if (data_len <= WebSocketStreamSize::SmallStream) {
			bits[1] = data_len;
			mask_offset = WebSocketMaskOffset::SmallOffset;
		}
		else if (data_len > WebSocketStreamSize::SmallStream && stream.length() <= 65535) {
			bits[1] = WebSocketStreamSize::MediumStream;
			bits[2] = (data_len >> 8) & 255;
			bits[3] = data_len & 255;
			mask_offset = WebSocketMaskOffset::MediumOffset;
		}
		else {
			bits[1] = WebSocketStreamSize::BigStream;
			bits[2] = (data_len >> 56) & 255;
			bits[3] = (data_len >> 48) & 255;
			bits[4] = (data_len >> 40) & 255;
			bits[5] = (data_len >> 32) & 255;
			bits[6] = (data_len >> 24) & 255;
			bits[7] = (data_len >> 16) & 255;
			bits[8] = (data_len >> 8) & 255;
			bits[9] = data_len & 255;
			mask_offset = WebSocketMaskOffset::BigOffset;
		}

		std::copy(stream.begin(), stream.end(), &bits[mask_offset]);

		WebSocketError err = Write(bits, mask_offset + data_len);
		
		delete [] bits;

		return err;
	}

	Connection::WebSocketError Connection::Authenticate()
	{
		std::string stream;
		WebSocketError error = ReadHTMLPacket(&stream);
		if (error != WebSocketError::NoError)
			return error;

		std::cout << "Connection::AuthWebSocket() : " << stream << std::endl;

		static const std::string kWebSocketKeyIdentifier("Sec-WebSocket-Key: ");
		static const int kWebSocketKeyLen = 24;
		static const std::string kWebSocketMagic("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

		std::size_t pos = stream.find(kWebSocketKeyIdentifier);
		if (pos == std::string::npos) {
			std::cerr << "No WebSocket Key" << std::endl;
			return WebSocketError::InvalidType;
		}

		std::string key;
		try {
			key = stream.substr(pos + kWebSocketKeyIdentifier.length(), kWebSocketKeyLen);
		} catch (std::out_of_range &err) {
			return WebSocketError::InvalidType;
		}

		key.append(kWebSocketMagic);
		std::cout << "key: " << key << std::endl;

		unsigned char* sha_str = SHA1(reinterpret_cast<const unsigned char*>(key.c_str()), key.length(), nullptr);
		if (strlen(reinterpret_cast<const char*>(sha_str)) != 20)
			return WebSocketError::InvalidType;

		std::string final = base64_encode(reinterpret_cast<const char*>(sha_str));

		std::ostringstream oss;
		oss << "HTTP/1.1 101 Switching Protocols\r\n";
		oss << "Upgrade: websocket\r\n";
		oss << "Connection: Upgrade\r\n";
		oss << "Sec-WebSocket-Accept: " << final << "\r\n";
		oss << "Sec-WebSocket-Protocol: chat\r\n";
		oss << "\r\n";

		std::cout << "*****SEND*****\n";
		std::cout << oss.str() << std::endl;

		Write(oss.str());

		return WebSocketError::NoError;
	}

	Connection::WebSocketError Connection::Receive()
	{
		WebSocketStreamHeader header;
		memset(&header, 0, sizeof(WebSocketStreamHeader));

		WebSocketError error = ReadStream(&header);
		if (error != WebSocketError::NoError) {
			std::cout << "Connection::DecodeWebSocket() - Error: " << error << std::endl;
			return error;
		}

		std::cout << "Payload size: " << header.payload_size << std::endl;
		std::cout << "Header len: " << header.header_size << std::endl;
		std::cout << "Real BufferLen: " << impl_->buffer_len_ << std::endl;

		if (!DecodeRawStreamData(header)) {
			std::cout << "Connection::DecodeWebSocket Error - DecodeRawStreamData" << std::endl;
			error = WebSocketError::InvalidType;
		}

		return WebSocketError::NoError;
	}

	bool Connection::DecodeRawStreamData(const WebSocketStreamHeader& header)
	{
		const unsigned char *final_buf = impl_->buffer_.data();
		if (impl_->buffer_len_ < (int)header.header_size + 1) {
			return false;
		}

		char masks[4];
		memcpy(masks, final_buf + header.mask_offset, 4);

		char* payload = new char[header.payload_size + 1];
		memcpy(payload, final_buf + header.mask_offset + 4, header.payload_size);
		for (unsigned long long i = 0; i < header.payload_size; ++i) {
			payload[i] =  (payload[i] ^ masks[i % 4]);
		}
		payload[header.payload_size] = '\0';
		impl_->stream_ = payload;

		delete [] payload;

		return true;
	}

	Connection::WebSocketError Connection::ReadStream(WebSocketStreamHeader *header) 
	{
		ResetBuffer();

		WebSocketError err = ReadHeader(header);
		if (err != WebSocketError::NoError) {
			return err;
		}
		
		Read(header->payload_size, header->payload_size);
			
		return err;
	}

	Connection::WebSocketError Connection::ReadHeader(WebSocketStreamHeader *header)
	{
		WebSocketError err = Read(6, 6);
		if (err != WebSocketError::NoError) {
			return err;
		}
		const unsigned char *buf = impl_->buffer_.data();

		header->fin = buf[0] & 0x80;
		std::cout << "	fin: " << header->fin << std::endl;

		header->masked = buf[1] & 0x80;
		std::cout << "	masked: " << header->masked << std::endl;
		unsigned char stream_size = buf[1] & 0x7F;

		header->opcode = buf[0] & 0x0F;
		std::cout << "   opcode: " << std::endl;
		printf("%01x\n", header->opcode);
		if (header->opcode == WebSocketOpCode::ContinuationFrame) {
			std::cout << "	continuation frame" << std::endl;
			return WebSocketError::InvalidType;
		}
		else if (header->opcode == WebSocketOpCode::TextFrame) {
			std::cout << "	text frame" << std::endl;
		}
		else if (header->opcode == WebSocketOpCode::BinaryFrame) {
			std::cout << "   binary frame" << std::endl;
			return WebSocketError::InvalidType;
		}
		else if (header->opcode == WebSocketOpCode::ConnectionClose) {
			std::cout << "   connection close" << std::endl;
			return WebSocketError::Abort;
		}
		else if (header->opcode == WebSocketOpCode::Ping) {
			std::cout << "   ping" << std::endl;
			return WebSocketError::InvalidType;
		}
		else if (header->opcode == WebSocketOpCode::Pong) {
			std::cout << "   pong" << std::endl;
			return WebSocketError::InvalidType;
		}
		else {
			printf("%01x\n", header->opcode);
			std::cout << "  opcode not handled by application" << std::endl;
			return WebSocketError::Abort;
		}

		if (stream_size <= WebSocketStreamSize::SmallStream) {
			std::cout << "	small stream" << std::endl;
			header->header_size = WebSocketHeaderSize::SmallHeader;
			header->payload_size = stream_size;
			header->mask_offset = WebSocketMaskOffset::SmallOffset;
		}
		else if (stream_size == WebSocketStreamSize::MediumStream) {
			if (Read(2, 2) == WebSocketError::Abort)
				return WebSocketError::Abort;
			std::cout << "	medium stream" << std::endl;
			header->header_size = WebSocketHeaderSize::MediumHeader;
			unsigned short s = 0;
			memcpy(&s, (const char*)&buf[2], 2);
			header->payload_size = ntohs(s);
			header->mask_offset = WebSocketMaskOffset::MediumOffset;
		}
		else if (stream_size == WebSocketStreamSize::BigStream) {
			if (Read(8, 8) == WebSocketError::Abort)
				return WebSocketError::Abort;

			std::cout << "	big stream" << std::endl;
			header->header_size = WebSocketHeaderSize::BigHeader;
			unsigned long long l = 0;
			memcpy(&l, (const char*)&buf[2], 8);
			
			header->payload_size = be64toh(l);
			header->mask_offset = WebSocketMaskOffset::BigOffset;
		}
		else {
			std::cout << "Couldnt decode stream size" << std::endl;
			return WebSocketError::Abort;
		}

		if (header->payload_size > kBufferSize - 56) {
			return WebSocketError::MessageTooBig;
		}

		int diff = header->header_size + header->payload_size - impl_->buffer_len_;
		std::cout << "		PayLoad Size:  " << header->payload_size << std::endl;
		std::cout << "  	Header Size:   " << header->header_size << std::endl;
		std::cout << "  	Total: 	       " << header->header_size + header->payload_size << std::endl;
		std::cout << "  	BufferSize:    " << impl_->buffer_len_ << std::endl;
		std::cout << "------------------------------------------------" << std::endl;
		std::cout << "  	Difference:    " << diff << std::endl;
		std::cout << "  	Socket:        " << impl_->socket_->Native() << std::endl;
	
		return err;
	}

	Connection::WebSocketError Connection::ReadHTMLPacket(std::string* stream) 
	{
		ResetBuffer();

		int read_bytes = 1024;
		WebSocketError err = Read(read_bytes);
		if (err != WebSocketError::NoError) {
			return err;
		}

		*stream = reinterpret_cast<const char*>(impl_->buffer_.data());
		std::size_t delim_pos = stream->find("\r\n\r\n");
		if (delim_pos != std::string::npos) {
			std::cout << "Connection::ReadHTMLPacket::Found end of header" << std::endl;
		} else {
			std::cout << "No Header found" << std::endl;
			err = WebSocketError::Abort;
		}

		return err;
	}

	void Connection::ResetBuffer()
	{
		std::fill(impl_->buffer_.begin(), impl_->buffer_.begin() + impl_->buffer_len_, 0);
		impl_->buffer_len_ = 0;
	}

	Connection::WebSocketError Connection::Read()
	{
		return Read(kBufferSize);
	}

	Connection::WebSocketError Connection::Read(int bytes)
	{
		return Read(bytes, 1);
	}

	Connection::WebSocketError Connection::Read(int bytes, int expected)
	{
		std::cout << " **** READ START **** " << std::endl;
		std::cout << "Connection::Read START: " << impl_->socket_->Native() << std::endl;
		std::cout << "will try to read: " << bytes << " bytes" << std::endl;
		std::cout << "starting buffer len: " << impl_->buffer_len_ << std::endl;

		int remaining_size = bytes;
		int bytes_read = -1;
		auto it = impl_->buffer_.begin();

		std::cout << "	START READ LOOP" << std::endl;
		//static const int kMaxTimeOut = 10;
		//int timeout = 0;
		while (expected > 0) {
			std::cout << "	Expected size: " << expected << " bigger 0" << std::endl;
			while ((remaining_size > 0) && ((bytes_read = read(impl_->socket_->Native(), &it[impl_->buffer_len_], remaining_size)) > 0)) {				
				if (bytes_read > 0) {
					impl_->buffer_len_ += bytes_read;
					remaining_size -= bytes_read;
					expected -= bytes_read;	
				} 
				if (expected < 0) { 
					expected = 0;
				}

				std::cout << "	**************Bytes read Innerloop: " << bytes_read << std::endl;
				std::cout << "	expected: " << expected << std::endl;
				std::cout << "  buffer_len_: " << impl_->buffer_len_ << std::endl;
				
			}	
			std::cout << "   ****************Bytes_read outer loop: " << bytes_read << std::endl;
			
			if (bytes_read == 0) {
				errno = 0;
				std::cerr << "Bytes read Kill connection" << std::endl;
				return WebSocketError::Abort;
			}
			if (expected > 0) { 
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
		}
		std::cout << "	FINISHED READ LOOP" << std::endl;

		std::cout << "Connection::Read() - BufferData" << std::endl;
		std::cout << "bytes read; " << bytes_read << std::endl;
		std::cout << "BufferLength: " << impl_->buffer_len_ << std::endl;

		std::cout << "**** READ FINISH *****" << std::endl;

		return WebSocketError::NoError;
	}

	Connection::WebSocketError Connection::Write(const std::string& stream) const
	{
		return Write(reinterpret_cast<const unsigned char*>(stream.c_str()), stream.length());
	}

	Connection::WebSocketError Connection::Write(const unsigned char* bits, long long len) const
	{
		std::cout << "	** WRITE ** " << std::endl;
		int bytes_send = 0;
		int remaining = len;
		int offset = 0;
		while ((remaining > 0) && (bytes_send = write(impl_->socket_->Native(), &bits[offset], remaining)) > 0) {
			//std::cout << "	remaining: " << remaining << std::endl;
			if (bytes_send > 0) {
				offset += bytes_send;
				remaining -= bytes_send;
			}
			else if (bytes_send == 0) {
				return WebSocketError::Abort;
			}
			if (remaining > 0) {
				std::cout << "sleep" << std::endl;
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}
			//perror("Connection::Write() - write()");
			errno = 0;
		}
		std::cout << "	bytes send: " << bytes_send << std::endl;
		std::cout << "   ** END OF WRITE ** " << std::endl;
		return WebSocketError::NoError;
	}
	
	void Connection::Disconnect()
	{
		impl_->socket_->Close();
	}

	network::Socket& Connection::Socket() const
	{
		return *(impl_->socket_);
	}

	const std::string& Connection::Stream() const
	{
		return impl_->stream_;
	}

} // namespace network
