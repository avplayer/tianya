#pragma once

#include "proxy_chain.hpp"
#include "../proxy_connect.hpp"

namespace avproxy{
namespace proxy{
// 使用　TCP 发起连接. 也就是不使用代理啦.
class tcp
{
public:
	typedef boost::asio::ip::tcp::resolver::query query;
	typedef boost::asio::ip::tcp::socket socket;

	tcp(socket &_socket,const query & _query)
		:socket_(_socket), query_(_query)
	{
	}

	template<class handler_type>
	void async_connect(handler_type handler)
	{
		avproxy::async_connect(socket_, query_, handler);
	}

	socket&		socket_;
	const query	query_;
};


} // namespace proxy
} // namespace avproxy