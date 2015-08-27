
/***
 * 
 * autoproxy.cpp
 *
 * avproxy 示例 - 使用 avproxy::autoproxychain 自动从 环境变了构建 proxychain
 * 
 */

#include "avproxy.hpp"



static void connected(const boost::system::error_code & ec, boost::asio::ip::tcp::socket & msocket)
{
	if ( ec )
		std::cout <<  ec.message() <<  std::endl;
	else
		msocket.write_some(boost::asio::buffer(std::string("GET / HTTP/1.1\r\n\r\n")));
}

int main()
{
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket msocket(io_service);

	// 支持环境变量　socks5_proxy/ http_proxy 哦！
	// avproxy::autoproxychain 构建一个 proxychain 
	// avproxy::autoproxychain 支持环境变量　socks5_proxy/ http_proxy， 要更精确的控制，请自己构建
	// proxychain 做为参数.
	avproxy::async_proxy_connect(
		avproxy::autoproxychain(msocket, avproxy::proxy::tcp::query("www.google.com", "81")),
		boost::bind(&connected, boost::asio::placeholders::error, boost::ref(msocket))
	);
	io_service.run();
}