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

namespace detail{

template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
class async_safe_read_until_op : boost::asio::coroutine {
private:
	AsyncReadStream &m_stream;
	boost::asio::basic_streambuf<Allocator> &	m_buf;
	const std::string	m_delim;
	ReadHandler			m_handler;
public:
	typedef				void result_type;

	async_safe_read_until_op(AsyncReadStream& s, 
		boost::asio::basic_streambuf<Allocator>& b, const std::string& delim,
		ReadHandler handler)
		:m_buf(b), m_stream(s), m_delim(delim), m_handler(handler)
	{
		m_stream.get_io_service().post(boost::bind(*this, boost::system::error_code(), 0));
	}
	void operator()(const boost::system::error_code & ec, std::size_t length)
	{
		BOOST_ASIO_CORO_REENTER(this)
		{
			// 一次读取一个字节.
			do{
				BOOST_ASIO_CORO_YIELD boost::asio::async_read(m_stream, m_buf.prepare(1), *this);
				if ( ec) break;
				m_buf.commit(length);			
			}while(!check_delim());
			m_stream.get_io_service().post(
				boost::asio::detail::bind_handler((m_handler), ec, m_buf.size())
			);
		}
	}
private:
	bool check_delim()
	{
		// 检查 m_buf 里是不是有 delim
		std::string response(boost::asio::buffer_cast<const char*>(m_buf.data()), m_buf.size());
		return response.find(m_delim) != std::string::npos;
	}
};

/**
 * async_safe_read_until 安全的读取到 delim 为止，绝对不多读一个字节。
 * 内部实现是每次读取一个字节，所以为了效率考虑请不要使用这个接口哦.
 */
template <typename AsyncReadStream, typename Allocator, typename ReadHandler>
void async_safe_read_until(AsyncReadStream& s, 
		boost::asio::basic_streambuf<Allocator>& b, const std::string& delim, ReadHandler handler)
{
	// If you get an error on the following line it means that your handler does
	// not meet the documented type requirements for a ReadHandler.

	//BOOST_ASIO_READ_HANDLER_CHECK(ReadHandler, handler) type_check;

	async_safe_read_until_op<AsyncReadStream, Allocator, ReadHandler>(
		s, b, delim, handler
	);
}

}

// 使用　http代理 发起连接.
// TODO, 添加认证支持.
class http : public  avproxy::detail::proxy_base , boost::asio::coroutine{
public:
	typedef boost::asio::ip::tcp::resolver::query query;
	typedef boost::asio::ip::tcp::socket socket;
	typedef void result_type;

	// _query 目的地址
	http(socket &_socket,const query & _query)
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

	// 执行　HTTP 的　handshake, 要异步哦.
	void _handshake(handler_type handler, proxy_chain subchain ){

		socket_.get_io_service().post(boost::asio::detail::bind_handler(*this, boost::system::error_code(), 0, handler));
	}
public:
	void operator()(boost::system::error_code ec,  std::size_t length, handler_type handler)
	{
		const char * buffer = NULL;
		if (m_buffer)
			buffer = boost::asio::buffer_cast<const char*>(m_buffer->data());
		BOOST_ASIO_CORO_REENTER(this)
		{
			do{
				// 写入　CONNECT XXX:XXX
				// TODO,  支持认证.
				BOOST_ASIO_CORO_YIELD boost::asio::async_write(socket_, boost::asio::buffer(
								std::string("CONNECT ") + query_.host_name() + ":" + query_.service_name() + "HTTP/1.1\r\n\r\n"
							), boost::bind(*this, _1, _2, handler));
				if (ec) break;
				m_buffer.reset(new boost::asio::streambuf);
				detail::async_safe_read_until(socket_, *m_buffer, "\r\n\r\n" , boost::bind(*this, _1, _2, handler));
				if (ec) break;
				// 解析是不是　HTTP 200 OK
				if (check_status()==200)
				{
					ec = boost::system::error_code();
				}else {
					ec = error::make_error_code(error::proxy_unknow_error);
				}
			}while (false);
 			socket_.get_io_service().post(boost::asio::detail::bind_handler(handler, ec));
		}
	}

	void _async_write_some(){
		
	}

private:
	unsigned check_status ()
	{
		std::string	ver;
		unsigned status;

		std::istream response(m_buffer.get());
		response >> ver >> status;
		return status;
	}

private:
	boost::shared_ptr<boost::asio::streambuf>	m_buffer;
	socket&		socket_;
	const query	query_;
};


} // namespace proxy
} // namespace avproxy