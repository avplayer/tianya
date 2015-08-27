#pragma once

#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include "./error.hpp"
#include "proxy_chain.hpp"
#include "../proxy_connect.hpp"

#include "proxy_error_mapper.hpp"

namespace avproxy{

namespace proxy{

// 使用　socks5代理 发起连接.
// TODO, 添加认证支持.
class socks5 : public  avproxy::detail::proxy_base , boost::asio::coroutine{
public:
	typedef boost::asio::ip::tcp::resolver::query query;
	typedef boost::asio::ip::tcp::socket socket;
	typedef void result_type;

	socks5(socket &_socket,const query & _query)
		:socket_(_socket), query_(_query)
	{

	}
private:
	void _resolve(handler_type handler, proxy_chain subchain){
		// 执行下层　proxy 的链接，通常是　tcp
		// TODO: 将这里的错误从　connection-refuse 改成　　proxy_connectio_refuse
		handler_type _handler = boost::bind(detail::proxy_error_mapper<handler_type>, _1, handler);

  		avproxy::async_proxy_connect(subchain, _handler);
	}

	void _handshake(handler_type handler, proxy_chain subchain ){
		// 执行　socks5 的　handshake, 要异步哦.
		socket_.get_io_service().post(boost::asio::detail::bind_handler(*this, boost::system::error_code(), 0, handler));
	}
public:
	void operator()(boost::system::error_code ec,  std::size_t length, handler_type handler)
	{
		static const char socks5_command[3] = {5, 1, 0};
		const char * buffer = NULL;
		if (m_buffer)
			buffer = boost::asio::buffer_cast<const char*>(m_buffer->data());
		BOOST_ASIO_CORO_REENTER(this)
		{
			do{
				// 写入　0x05, 0x00
				// TODO,  支持认证.
				BOOST_ASIO_CORO_YIELD boost::asio::async_write(socket_, boost::asio::buffer(socks5_command), boost::bind(*this, _1, _2, handler));
				if (ec) break;
				m_buffer.reset(new boost::asio::streambuf);
				BOOST_ASIO_CORO_YIELD boost::asio::async_read(socket_, m_buffer->prepare(2), boost::bind(*this, _1, _2, handler));
				if (ec) break;
				// 解析是不是　0x05, 0x00
				m_buffer->commit(length);
				if (!(buffer[0] == 5 && buffer[1] == 0))
				{
					ec = error::make_error_code(error::proxy_not_authorized);
					break;
				}
				//　执行　connect 命令.
				BOOST_ASIO_CORO_YIELD boost::asio::async_write(socket_, boost::asio::buffer(buildsocks5connectcmd()), boost::bind(*this, _1, _2, handler));
				if (ec) break;
				m_buffer.reset(new boost::asio::streambuf);
				// 读取 socks5 server 返回值.
				BOOST_ASIO_CORO_YIELD boost::asio::async_read(socket_, m_buffer->prepare(4), boost::bind(*this, _1, _2, handler));
				if (ec) break;
				m_buffer->commit(length);

				// 解析返回值.
				if (!(length==4 && buffer[0] == 5 && buffer[1] == 0 && buffer[2] == 0 ))
				{
					ec = error::make_error_code(error::proxy_not_authorized);
					break;
				}

				// ATYPE==ipv4
				if (boost::asio::buffer_cast<const char*>(m_buffer->data())[3] == 1){
					BOOST_ASIO_CORO_YIELD boost::asio::async_read(socket_, m_buffer->prepare(6), boost::bind(*this, _1, _2, handler));
					m_buffer->commit(length);
				}else if (boost::asio::buffer_cast<const char*>(m_buffer->data())[3]==4){
					BOOST_ASIO_CORO_YIELD boost::asio::async_read(socket_, m_buffer->prepare(18), boost::bind(*this, _1, _2, handler));
					m_buffer->commit(length);
				}else if (boost::asio::buffer_cast<const char*>(m_buffer->data())[3]==3){
					ec = error::make_error_code(error::proxy_unknow_error);
				}else{
					// 出错.
					ec = error::make_error_code(error::proxy_unknow_error);
				}
				if (ec) break;
			}while (false);
 			socket_.get_io_service().post(boost::asio::detail::bind_handler(handler, ec));
		}
	}

	void _async_write_some(){
		
	}

private:
	//  0x05 0x01 0x00 0x03 [length]ipaddr [port]
	std::vector<char> buildsocks5connectcmd()
	{
		std::vector<char>	cmd(4 + 1 + query_.host_name().length() + 2 );
 		unsigned short port = boost::lexical_cast<int>(query_.service_name());
		cmd[0] = 5;
		cmd[1] = 1;
		cmd[2] = 0;
		cmd[3] = 3;
		cmd[4] = query_.host_name().length();
		std::memcpy(&cmd[5], query_.host_name().c_str(), query_.host_name().length());
 		cmd[5+query_.host_name().length()] = port >> 8;
 		cmd[5+query_.host_name().length() + 1 ] = port & 0xFF;
		return cmd;//boost::asio::buffer(cmd, cmd.size());
	}

private:
	boost::shared_ptr<boost::asio::streambuf>	m_buffer;
	socket&		socket_;
	const query	query_;
};


} // namespace proxy
} // namespace avproxy

