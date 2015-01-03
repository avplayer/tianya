#pragma once

#include <boost/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

namespace avproxy{
namespace error{

enum proxy_errors{
	proxy_connection_refused = 5000, 
	proxy_refused = 5000, 
	proxy_not_authorized,
	proxy_unknow_error, 
};

namespace detail{
	class avproxy_category;
}

template<class error_category>
const boost::system::error_category& error_category_single()
{
	static error_category error_category_instance;
	return reinterpret_cast<const boost::system::error_category&>(error_category_instance);
}

inline const boost::system::error_category& get_avproxy_category()
{
	return error_category_single<detail::avproxy_category>();
}

inline boost::system::error_code make_error_code(proxy_errors e)
{
	return boost::system::error_code(
      static_cast<int>(e), get_avproxy_category());
}


namespace detail {

class avproxy_category : public boost::system::error_category {
public:
	const char* name() const BOOST_NOEXCEPT {
		return "avproxy.proxy";
	}

	std::string message ( int value ) const {
		if ( value == proxy_connection_refused )
			return "remote proxy refused the connection";

		if ( value == proxy_not_authorized )
			return "authorizing to proxy failed";

		return "avproxy.proxy error";
	}
};

} // namespace detail

} // namespace error
} // namespace avproxy

namespace boost {
namespace system {

template<> struct is_error_code_enum<avproxy::error::proxy_errors>
{
  static const bool value = true;
};

}
}
