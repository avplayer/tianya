
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/asio/detail/handler_type_requirements.hpp>

#include <boost/bind.hpp>

namespace boost {
namespace detail {

template<class timeunit, class Handler>
class base_delayedcall_op{
public:
	typedef void result_type;

	base_delayedcall_op(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
		: io_service(_io_service)
		, timer( new boost::asio::deadline_timer(io_service))
		, m_handler(handler)
	{
		timer->expires_from_now(timeunit(timeunitcount));
 		timer->async_wait(*this);
	}

	void operator()(const boost::system::error_code& error)
	{
		io_service.post(m_handler);
	}

private:
	boost::asio::io_service &io_service;
	boost::shared_ptr<boost::asio::deadline_timer> timer;

	Handler m_handler;
};


template<class Handler>
base_delayedcall_op<boost::posix_time::seconds, Handler>
make_delayedcallsec_op(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
{
	return base_delayedcall_op<boost::posix_time::seconds, Handler>(_io_service, timeunitcount, handler);
}

template<class Handler>
base_delayedcall_op<boost::posix_time::milliseconds, Handler>
make_delayedcallms_op(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
{
	return base_delayedcall_op<boost::posix_time::milliseconds, Handler>(_io_service, timeunitcount, handler);
}

template<class Handler>
base_delayedcall_op<boost::posix_time::microseconds, Handler>
make_delayedcallus_op(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
{
	return base_delayedcall_op<boost::posix_time::microseconds, Handler>(_io_service, timeunitcount, handler);
}


} // namespace detail

template<class Handler>
void delayedcallsec(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
{
	detail::make_delayedcallsec_op(_io_service, timeunitcount, handler);
}

template<class Handler>
void delayedcallms(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
{
	detail::make_delayedcallms_op(_io_service, timeunitcount, handler);
}

template<class Handler>
void delayedcallus(boost::asio::io_service &_io_service, int timeunitcount, Handler handler)
{
	detail::make_delayedcallus_op(_io_service, timeunitcount, handler);
}

} // namespace boost
