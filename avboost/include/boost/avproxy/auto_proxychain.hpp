#pragma once

#include <cstdlib>
#include <boost/asio/io_service.hpp>

#include "detail/proxy_chain.hpp"
#include "proxies.hpp"

#include "boost/xpressive/xpressive.hpp"

namespace avproxy{

// automanticaly build proxychain
// accourding to env variables http_proxy and socks5_proxy
// to use socks5 proxy, set socks5_proxy="host:port"
template<class Socket>
proxy_chain autoproxychain(Socket & socket, const typename Socket::protocol_type::resolver::query & _query, std::string proxy_settings_string = "")
{
	using query_t = typename Socket::protocol_type::resolver::query;

	proxy_chain _proxychain(socket.get_io_service());

	if (proxy_settings_string.empty())
	{
		if (std::getenv("socks5_proxy"))
			proxy_settings_string = std::getenv("socks5_proxy");
		if (std::getenv("http_proxy"))
			proxy_settings_string = std::getenv("http_proxy");
	}

	if (!proxy_settings_string.empty())
	{
		const boost::xpressive::sregex url_expr = (boost::xpressive::s1= (boost::xpressive::as_xpr("http://") | boost::xpressive::as_xpr("socks5://")) )
			>> (boost::xpressive::s2= +boost::xpressive::set[ boost::xpressive::range('a','z') | boost::xpressive::range('A', 'Z') | boost::xpressive::range('0', '9') | '.' ] )
			>>  -! ( ':' >>  (boost::xpressive::s3= +boost::xpressive::set[boost::xpressive::range('0', '9')]) )
			>> ! boost::xpressive::as_xpr('/');

		boost::xpressive::smatch what_url;

		if (boost::xpressive::regex_match(proxy_settings_string, what_url, url_expr))
		{
			std::string host = what_url[2].str();
			std::string optional_port = what_url[3].str();

			if ( what_url[1].str() == "http://")
			{
				// 应该是 http_proxy=http://host[:port]
				if (optional_port.empty())
					optional_port = "80";
				_proxychain.add(proxy::tcp(socket, query_t(host, optional_port)));
				_proxychain.add(proxy::http(socket, _query));
				return _proxychain;
			}

			if ( what_url[1].str() == "socks5://")
			{
				// 应该是 http_proxy=http://host[:port]
				if (optional_port.empty())
					optional_port = "1080";

				_proxychain.add(proxy::tcp(socket, query_t(host, optional_port)));
				_proxychain.add(proxy::socks5(socket, _query));
				return _proxychain;
			}
		}
	}

	_proxychain.add(proxy::tcp(socket, _query));
	return _proxychain;
}

}
