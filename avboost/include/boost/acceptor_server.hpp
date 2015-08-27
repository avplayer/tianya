
#pragma once

#ifdef __llvm__
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif

#ifndef AV_ACCEPTOR_SERVER_H
#define AV_ACCEPTOR_SERVER_H

#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio.hpp>

namespace boost{
namespace detail{

template<typename Protocol, typename ProtocolProcesser>
class acceptor_server_op :  boost::asio::coroutine
{
	boost::asio::io_service & io_service;
	boost::shared_ptr<boost::asio::basic_socket_acceptor<Protocol> > m_acceptor_socket;
	boost::shared_ptr<boost::asio::basic_stream_socket<Protocol> > m_client_socket;
	ProtocolProcesser protocolprocesser;
	void start_accept(){
		io_service.post(boost::asio::detail::bind_handler(*this, boost::system::error_code()));
	}

public:
	typedef typename Protocol::endpoint endpoint_type;

	acceptor_server_op(boost::asio::io_service & io, const endpoint_type & endpoint, ProtocolProcesser protocolprocesser_)
		:io_service(io), m_acceptor_socket(new boost::asio::basic_socket_acceptor<Protocol>(io, endpoint)), protocolprocesser(protocolprocesser_)
	{
		start_accept();
	}

	acceptor_server_op(boost::shared_ptr<boost::asio::basic_socket_acceptor<Protocol> > acceptor_socket, ProtocolProcesser protocolprocesser_)
		:io_service(acceptor_socket->get_io_service()), m_acceptor_socket(acceptor_socket), protocolprocesser(protocolprocesser_)
	{
		start_accept();
	}


	void operator()(const boost::system::error_code & ec)
	{
		BOOST_ASIO_CORO_REENTER(this)
		{
			do{
				m_client_socket.reset(new boost::asio::basic_stream_socket<Protocol>(io_service));
				BOOST_ASIO_CORO_YIELD m_acceptor_socket->async_accept(*m_client_socket, *this);
				if (!ec)
					BOOST_ASIO_CORO_FORK acceptor_server_op<Protocol, ProtocolProcesser>(*this)(ec);
			}while (is_parent());
			protocolprocesser(m_client_socket);
		}
	}

};

}

template<typename Protocol, class ProtocolProcesser>
void acceptor_server(boost::asio::io_service & io, const boost::asio::ip::basic_endpoint<Protocol> & endpoint, ProtocolProcesser protocolprocesser)
{
	detail::acceptor_server_op<Protocol, ProtocolProcesser>(io, endpoint, protocolprocesser);
}

}


#endif // AV_ACCEPTOR_SERVER_H
