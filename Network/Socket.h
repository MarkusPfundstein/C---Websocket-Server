#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include <memory>

namespace network {

	class Socket
	{
		public:
			Socket();
			explicit Socket(int sock);
			virtual ~Socket();

			int Native() const;
			bool IsOpen() const;
			bool SetNonBlock();

			void Close();

		private:
			friend class Reactor;
			void Open(int id);

			int id_;
	};

} // namespace network

#endif
