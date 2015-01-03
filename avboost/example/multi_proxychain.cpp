
/***
 * 
 * multi_proxychain.cpp
 *
 * avproxy 示例 - 使用 avproxy::proxychain 进行代理嵌套
 * 
 * 
 * testclient -> socks5 proxy <mysocks5proxy.me>
 *		-> http proxy <myhttpproxy.me>
 *		-> target host <www.google.com>
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

	avproxy::proxy_chain proxychain(io_service);
	proxychain.add(avproxy::proxy::tcp(msocket, boost::asio::ip::tcp::resolver::query("mysocks5proxy.me", "1080")));
	proxychain.add(avproxy::proxy::socks5(msocket, boost::asio::ip::tcp::resolver::query("myhttpproxy.me", "8080")));
	proxychain.add(avproxy::proxy::http(msocket, boost::asio::ip::tcp::resolver::query("www.google.com", "80")));

	avproxy::async_proxy_connect(
		proxychain, boost::bind(&connected, boost::asio::placeholders::error, boost::ref(msocket))
	);
	io_service.run();
}