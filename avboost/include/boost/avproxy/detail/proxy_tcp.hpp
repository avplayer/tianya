#pragma once

#include "proxy_chain.hpp"
#include "../proxy_connect.hpp"

namespace avproxy{
namespace proxy{
// 使用　TCP 发起连接. 也就是不使用代理啦.
class tcp : public  avproxy::detail::proxy_base {
public:
	typedef boost::asio::ip::tcp::resolver::query query;
	typedef boost::asio::ip::tcp::socket socket;

	tcp(socket &_socket,const query & _query)
		:socket_(_socket), query_(_query)
	{
	}
private:
	void _resolve(handler_type handler, proxy_chain subchain){
		//handler(boost::system::error_code());
		socket_.get_io_service().post(boost::asio::detail::bind_handler(handler, boost::system::error_code()));
	}

	void _handshake(handler_type handler, proxy_chain subchain ){
		avproxy::async_connect(socket_, query_, handler);
	}

	socket&		socket_;
	const query	query_;
};


} // namespace proxy
} // namespace avproxy