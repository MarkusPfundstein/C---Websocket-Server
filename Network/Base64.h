#ifndef __NETWORK_REACTOR_BASE64_H__
#define __NETWORK_REACTOR_BASE64_H__

#include <string>

namespace network {
	// * http://stackoverflow.com/a/5291537
	extern std::string base64_encode(const std::string& bindata);
	extern std::string base64_decode(const std::string& ascdata);

} // namespace network

#endif
