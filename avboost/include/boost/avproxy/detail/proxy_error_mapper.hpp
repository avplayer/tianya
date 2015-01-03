#pragma once

#include <boost/system/system_error.hpp>

namespace avproxy{

namespace proxy{

namespace detail{
	template<class Handler>
	void proxy_error_mapper(const boost::system::error_code & _ec, Handler handler)
	{
		boost::system::error_code ec = _ec;
		if (ec == boost::asio::error::connection_refused)
		{
			ec = avproxy::error::make_error_code(avproxy::error::proxy_connection_refused);
		}
		handler(ec);
	}
}

}// proxy

}// avproxy
