
#pragma once

#include <map>
#include <queue>
#include <boost/config.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/foreach.hpp>

namespace boost {
namespace detail {

template<class RealHandler, typename T>
class timed_out_handler_wrapper
{
public:
	timed_out_handler_wrapper(asio::io_service& io_service,
		const asio::deadline_timer::duration_type& timedout,
		RealHandler handler,
		const boost::shared_ptr<bool> &timed_outed)
		: m_io_service(io_service)
		, m_timer(make_shared<boost::asio::deadline_timer>(boost::ref(io_service)))
		, m_realhandler(make_shared<RealHandler>(handler))
		, m_timed_outed(timed_outed)
	{
		m_timer->expires_from_now(timedout);
		m_timer->async_wait(*this);
	}

	void operator()(boost::system::error_code ec)
	{

		if (ec == asio::error::operation_aborted)
		{
			// 计时器被取消，啥也不错， 因为回调已经执行了.
			return;
		}

		// 超时了

		// 标记
		*m_timed_outed = true;

		// 执行回调
		m_io_service.post(
			asio::detail::bind_handler(
				m_realhandler,
				system::errc::make_error_code(system::errc::timed_out),
				T()
			)
		);
	}

	void operator()(boost::system::error_code ec, const T &t)
	{
		// forward to real hander
		m_realhandler(ec, t);

		// 取消定时.
		m_timer->cancel(ec);
	}

private:
	asio::io_service& m_io_service;

	boost::shared_ptr<RealHandler> m_realhandler;

	boost::shared_ptr<boost::asio::deadline_timer> m_timer;

	boost::shared_ptr<bool> m_timed_outed;
};

} // namespace detail

/*
 * async_coro_queue 是一个用于协程的异步列队。
 *
 * ListType 只要任意支持 pop_front/push_back 的容器就可以。
 * 如 std::deque / std::list /boost::circular_buffer
 *
 * NOTE: 使用线程安全的列队做模板参数就可以让本协程列队用于多个线程跑的 io_service.
 */

template<typename ListType>
class async_coro_queue : boost::noncopyable{
public: // typetraits
	typedef typename ListType::value_type value_type;
	typedef typename ListType::size_type size_type;
	typedef typename ListType::reference reference;
	typedef typename ListType::const_reference const_reference;
private:
	// async_pop 的回调原型
	typedef	boost::function<
		void(boost::system::error_code ec, value_type)
	> async_pop_handler_type;

	// async_wait 的回调原型
	typedef	boost::function <
		void(boost::system::error_code ec)
	> async_wait_handler_type;

public:
	// 构造函数
	async_coro_queue(boost::asio::io_service & io_service)
	  :m_io_service(io_service)
	{
	}

#ifdef  BOOST_NO_CXX11_VARIADIC_TEMPLATES
	// 构造函数的一个重载，为列队传入额外的参数
	template<typename T>
	async_coro_queue(boost::asio::io_service & io_service, T t)
	  :m_io_service(io_service), m_list(t)
	{
	}
	// 利用 C++11 的 泛模板参数写第三个构造函数重载
#else
	template<typename ...T>
	async_coro_queue(boost::asio::io_service & io_service, T&&... t)
	  : m_io_service(io_service), m_list(std::forward<T>(t)...)
	{
	}
#endif

private:
	boost::system::error_code make_canceled()
	{
		return system::errc::make_error_code(system::errc::operation_canceled);
	}
	template<class Handler>
	void async_pop_impl(Handler handler)
	{
		if (m_list.empty())
		{
			// 进入睡眠过程.
			m_handlers.push(
				std::make_pair(
					make_shared<bool>(false), async_pop_handler_type(handler)
				)
			);
		}
		else
		{
			m_io_service.post(
				boost::asio::detail::bind_handler(
					handler, boost::system::error_code(), m_list.front()
				)
			);
			m_list.erase(m_list.begin());
		}
	}

	template<class Handler>
	void async_pop_impl(Handler handler, boost::asio::deadline_timer::duration_type timeout)
	{
		if (m_list.empty())
		{
			boost::shared_ptr<bool> timed_outed = make_shared<bool>(false);
			// 进入睡眠过程.
			m_handlers.push(
				std::make_pair(
					timed_outed,
					detail::timed_out_handler_wrapper<Handler, value_type>(
						m_io_service, timeout, handler, timed_outed
					)
				)
			);
		}
		else
		{
			m_io_service.post(
				boost::asio::detail::bind_handler(
					handler, boost::system::error_code(), m_list.front()
				)
			);
			m_list.pop_front();
		}
	}

	template<class Handler>
	void async_wait_impl(Handler handler)
	{
		if (m_list.empty())
		{
			// 进入睡眠过程.
			m_wait_handlers.push_back(handler);
		}
		else
		{
			m_io_service.post(
				boost::asio::detail::bind_handler(
					handler, boost::system::error_code()
				)
			);
		}
	}

// 公开的接口。
public:

	/*
	 *  回调的类型是 void pop_handler(boost::system::error_code ec, value_type)
	 *
	 *  value_type 由容器（作为模板参数）决定。
	 *  例子是

		// ec 如果有错误,  只可能是 boost::asio::error::operation_aborted

		void pop_handler(boost::system::error_code ec, value_type value)
		{
			// DO SOME THING WITH value

			// start again
			list.async_pop(pop_handler);
		}

		// 然后在其他地方
		list.push(value); 即可唤醒 pop_handler

	 *  NOTE: 如果列队里有数据， 回调将投递(不是立即回调，是立即投递到 io_service), 否则直到有数据才回调.
     */
	template<class RealHandler>
	inline BOOST_ASIO_INITFN_RESULT_TYPE(RealHandler,
		void(boost::system::error_code, value_type))
	async_pop(BOOST_ASIO_MOVE_ARG(RealHandler) handler)
	{
		using namespace boost::asio;

		//BOOST_ASIO_CONNECT_HANDLER_CHECK(RealHandler, handler) type_check;
		boost::asio::detail::async_result_init<
			RealHandler, void(boost::system::error_code, value_type)> init(
			BOOST_ASIO_MOVE_CAST(RealHandler)(handler));

		async_pop_impl<
			BOOST_ASIO_HANDLER_TYPE(RealHandler, void(boost::system::error_code, value_type))
		>(init.handler);
		return init.result.get();
	}
	/**
     * 用法同 async_pop, 但是增加了一个超时参数
     */

	template<class Handler>
	inline BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
		void(boost::system::error_code, value_type))
	async_pop(BOOST_ASIO_MOVE_ARG(Handler) handler, boost::asio::deadline_timer::duration_type timeout)
	{
		using namespace boost::asio;

		//BOOST_ASIO_CONNECT_HANDLER_CHECK(RealHandler, handler) type_check;
		boost::asio::detail::async_result_init<
			Handler, void(boost::system::error_code, value_type)> init(
			BOOST_ASIO_MOVE_CAST(Handler)(handler));

		async_pop_impl<
			BOOST_ASIO_HANDLER_TYPE(Handler, void(boost::system::error_code, value_type))
		>(init.handler, timeout);
		return init.result.get();
	}

	/*
	*  回调的类型是 void wait_handler(boost::system::error_code ec)
	*
	*  例子是

	// ec 如果有错误,  只可能是 boost::asio::error::operation_aborted

	void wait_handler(boost::system::error_code ec)
	{
		// now async_pop ;)
		list.async_pop(pop_handler);
	}

	// 然后在其他地方
	list.push(value); 即可唤醒 wait_handler

	*  NOTE: 如果列队里有数据， 回调将投递(不是立即回调，是立即投递到 io_service), 否则直到有数据才回调.
	*/
	template<class Handler>
	inline BOOST_ASIO_INITFN_RESULT_TYPE(Handler,
		void(boost::system::error_code))
	async_wait(BOOST_ASIO_MOVE_ARG(Handler) handler)
	{
		using namespace boost::asio;

		//BOOST_ASIO_CONNECT_HANDLER_CHECK(RealHandler, handler) type_check;
		boost::asio::detail::async_result_init<
			Handler, void(boost::system::error_code)> init(
			BOOST_ASIO_MOVE_CAST(Handler)(handler));

		async_wait_impl<
			BOOST_ASIO_HANDLER_TYPE(Handler, void(boost::system::error_code))
		>(init.handler);
		return init.result.get();
	}
	/**
	 * 向列队投递数据。
	 * 如果列队为空，并且有协程正在休眠在 async_pop 上， 则立即唤醒此协程，并投递数据给此协程
     */
	void push(const value_type &value)
	{
		// 有handler 挂着！
		if (!m_wait_handlers.empty())
		{
			// 如果 m_list 不是空， 肯定是有严重的 bug
			BOOST_ASSERT(m_list.empty());

			BOOST_FOREACH(const async_wait_handler_type & h, m_wait_handlers)
			{
				m_io_service.post(
					boost::asio::detail::bind_handler(
						h, boost::system::error_code()
					)
				);
			}
			m_wait_handlers.clear();
		}
		
		// 有handler 挂着！
		if (!m_handlers.empty())
		{
			// 如果 m_list 不是空， 肯定是有严重的 bug
			BOOST_ASSERT(m_list.empty());

			if (! *(m_handlers.front().first))
			{
				m_io_service.post(
					boost::asio::detail::bind_handler(
						m_handlers.front().second,
						boost::system::error_code(),
						value
					)
				);
				m_handlers.pop();
			}else
			{
				m_handlers.pop();
				return push(value);
			}
		}
		else
		{
			m_list.push_back(value);
		}
	}

	/**
	 * cancele all async operations.
     */
	void cancele()
	{
		if (!m_handlers.empty())
		{
			BOOST_FOREACH(const async_wait_handler_type & h, m_wait_handlers)
			{
				m_io_service.post(
					boost::asio::detail::bind_handler(
						h, make_canceled()
					)
				);
			}
			m_wait_handlers.clear();
		}

		while (!m_handlers.empty())
		{
			if (! *(m_handlers.front().first))
			{
				m_io_service.post(
					boost::asio::detail::bind_handler(
						m_handlers.front().second,
						make_canceled(),
						value_type()
					)
				);
			}
			m_handlers.pop();
		}
	}

	/*
     * clear all items in queue
     */
	void clear()
	{
		cancele();
		m_list.clear();
	}

	bool empty()
	{
		return m_list.empty();
	}

private:

	boost::asio::io_service & m_io_service;
	ListType m_list;
	// 保存 async_pop 回调函数
	std::queue<
		std::pair<boost::shared_ptr<bool>, async_pop_handler_type>
	> m_handlers;

	std::vector<async_wait_handler_type> m_wait_handlers;
};

} // namespace boost
