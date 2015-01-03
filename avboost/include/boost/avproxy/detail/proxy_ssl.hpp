#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include "proxy_chain.hpp"
#include "../proxy_connect.hpp"

namespace avproxy{
namespace proxy{

// SSL 连接过程. 支持透过代理发起　SSL 连接哦!
// TODO
class proxy_ssl : public  avproxy::detail::proxy_base{
public:
	typedef boost::asio::ip::tcp tcp;
	typedef tcp::resolver::query query;
	typedef tcp::socket socket;

	proxy_ssl(boost::asio::ssl::stream<tcp> &stream, query _query)
	{
		
	}

private:
	void _resolve(handler_type handler, proxy_chain subchain){
		// 递归调用　avconnect 传递到下一层.
		avproxy::async_proxy_connect(subchain, handler);
	}

	void _handshake(handler_type handler, proxy_chain subchain ){
		// 调用　ssl 的　async_handshake
		// TODO
	}

};

} // namespace proxy
} // namespace avproxy