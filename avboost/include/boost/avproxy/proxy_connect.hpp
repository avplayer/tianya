﻿
#pragma once

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif

#include <boost/asio.hpp>

#include "detail/proxy_chain.hpp"

namespace avproxy {

// convient helper class that made for direct TCP connection without proxy
// usage:
//    avproxy::async_connect(  socket ,  ip::tcp::resolve::query( "host" ,  "port" ) ,  handler );
// the hander should be this signature
//    void handle_connect(
//         const boost::system::error_code & ec
//    )
class async_connect : boost::asio::coroutine
{
public:
	typedef void result_type; // for boost::bind
public:
	template<class Socket, class Handler, class connect_condition>
	async_connect(Socket & socket,const typename Socket::protocol_type::resolver::query & _query, BOOST_ASIO_MOVE_ARG(Handler) handler, connect_condition _connect_condition)
	{
		//BOOST_ASIO_CONNECT_HANDLER_CHECK(Handler, handler) type_check;
		_async_connect(socket, _query, boost::function<void (const boost::system::error_code&)>(handler), _connect_condition);
	}

	template<class Socket, class Handler>
	async_connect(Socket & socket,const typename Socket::protocol_type::resolver::query & _query, BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		//BOOST_ASIO_CONNECT_HANDLER_CHECK(Handler, handler) type_check;
		_async_connect(socket, _query, boost::function<void (const boost::system::error_code&)>(handler), detail::avproxy_default_connect_condition());
	}

	template<class Handler, class Socket, class connect_condition>
	void operator()(const boost::system::error_code & ec, typename Socket::protocol_type::resolver::iterator begin, Socket & socket, boost::shared_ptr<typename Socket::protocol_type::resolver> resolver, Handler handler, connect_condition _connect_condition)
	{
 		//BOOST_ASIO_CONNECT_HANDLER_CHECK(Handler, handler) type_check;

		BOOST_ASIO_CORO_REENTER(this)
		{
			BOOST_ASIO_CORO_YIELD boost::asio::async_connect(socket,begin, _connect_condition, boost::bind(*this, _1, _2, boost::ref(socket), resolver, handler, _connect_condition));
			boost::asio::post(socket.get_executor(), boost::asio::detail::bind_handler(handler,ec));
		}
	}
private:
	template<class Socket, class Handler, class connect_condition>
	void _async_connect(Socket & socket,const typename Socket::protocol_type::resolver::query & _query, BOOST_ASIO_MOVE_ARG(Handler) handler, connect_condition _connect_condition)
	{
		//BOOST_ASIO_CONNECT_HANDLER_CHECK(Handler, handler) type_check;

		typedef typename Socket::protocol_type::resolver resolver_type;

		boost::shared_ptr<resolver_type>
			resolver(new resolver_type(socket.get_executor()));
		resolver->async_resolve(_query, boost::bind(*this, _1, _2, boost::ref(socket), resolver, handler, _connect_condition));
	}
};

// 带　proxy 执行连接.
template<typename Handler>
class async_proxy_connect_op : boost::asio::coroutine
{
public:
	typedef void result_type; // for boost::bind_handler
public:
	async_proxy_connect_op(const proxy_chain &proxy_chain, Handler &handler)
		: proxy_chain_(proxy_chain)
		, m_handler(handler)
	{
	}

	void operator()(boost::system::error_code ec = {})
	{
		BOOST_ASIO_CORO_REENTER(this)
		{
			// resolve
			BOOST_ASIO_CORO_YIELD proxy_chain_.back()->resolve(*this, proxy_chain_.clone_poped());
			if(! ec)
				BOOST_ASIO_CORO_YIELD proxy_chain_.back()->handshake(*this, proxy_chain_.clone_poped());
			m_handler(ec);
		}
	}
private:
	proxy_chain proxy_chain_;
	Handler m_handler;
};

struct initiate_do_async_proxy_connect
{
	template<typename Handler>
	void operator()(Handler&& handler, const proxy_chain *proxy_chain) const
	{
		async_proxy_connect_op<Handler>(*proxy_chain, handler)();
	}

};

template <typename RealHandler>
auto async_proxy_connect(const proxy_chain &proxy_chain, BOOST_ASIO_MOVE_ARG(RealHandler) handler)
{
	using namespace boost::asio;

	BOOST_ASIO_CONNECT_HANDLER_CHECK(RealHandler, handler) type_check;

	return boost::asio::async_initiate<RealHandler, void(boost::system::error_code)>(
		initiate_do_async_proxy_connect{}, handler, &proxy_chain);

}

}
