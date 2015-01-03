#pragma once

#include <cstdlib>
#include <boost/asio/io_service.hpp>

#include "detail/proxy_chain.hpp"
#include "proxies.hpp"

namespace avproxy{

namespace detail{

static boost::asio::ip::tcp::resolver::query queryfromstr(std::string str)
{
	std::size_t pos;
	pos = str.find("http://");
	if ( pos != std::string::npos)
	{
		// 应该是 http_proxy=http://host[:port]
		str = str.substr(pos+7);
		pos = str.find(':');
		if ( pos != std::string::npos)
		{
			// 应该是 http_proxy=http://host
			// 默认 80 端口即可.
			return boost::asio::ip::tcp::resolver::query(str, "80");
		}
		return queryfromstr(str);		
	}

	pos = str.find(':');
	std::string host = str.substr(0, pos);
	std::string port = str.substr(pos+1);
	return boost::asio::ip::tcp::resolver::query(host, port);
}

}
// automanticaly build proxychain
// accourding to env variables http_proxy and socks5_proxy
// to use socks5 proxy, set socks5_proxy="host:port"
template<class Socket>
proxy_chain autoproxychain(Socket & socket,const typename Socket::protocol_type::resolver::query & _query)
{
	proxy_chain _proxychain(socket.get_io_service());
	if (std::getenv("socks5_proxy"))
	{ // add socks5_proxy
		_proxychain.add(proxy::tcp(socket, detail::queryfromstr(std::getenv("socks5_proxy"))));
		_proxychain.add(proxy::socks5(socket, _query));
	}else if (std::getenv("http_proxy")){
		_proxychain.add(proxy::tcp(socket, detail::queryfromstr(std::getenv("http_proxy"))));
		_proxychain.add(proxy::http(socket, _query));
	}else{
		_proxychain.add(proxy::tcp(socket, _query));
	}
	return _proxychain;
}

}
