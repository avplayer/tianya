#pragma once

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>

namespace avproxy {
class proxy_chain;
namespace detail {

	// 为proxy_chian提供便利的添加语法的辅助类.
	struct proxychain_adder{
		proxychain_adder( proxy_chain & _chain)
			:chain(&_chain)
		{
		}

		template<class Proxy>
		proxychain_adder operator()(const Proxy & proxy );

		operator proxy_chain& (){
			return *chain;
		}
		operator proxy_chain ();

	private:
		proxy_chain * chain;
	};
}


namespace detail{
	class proxy_wrapper;
	class proxy_type_erasure_base
	{
	public:
		typedef std::function< void (const boost::system::error_code&) > handler_type;
		friend class proxy_wrapper;

	private:
		virtual void async_connect(handler_type) = 0;
	};

	template<class RealProxy>
	class proxy_adaptor : public proxy_type_erasure_base
	{
		friend class proxy_wrapper;
		proxy_adaptor(const RealProxy & realproxy)
			: realobj(realproxy)
		{
		}

		proxy_adaptor(RealProxy && realproxy)
			: realobj(realproxy)
		{
		}

		virtual void async_connect(proxy_type_erasure_base::handler_type handler) override
		{
			realobj.async_connect(handler);
		}
		RealProxy realobj;
	};

	class proxy_wrapper
	{
	public:
		proxy_wrapper(const proxy_wrapper& other) = default;
		proxy_wrapper(proxy_wrapper&& other) = default;

		proxy_wrapper& operator=(const proxy_wrapper& other) = default;
		proxy_wrapper& operator=(proxy_wrapper&& other) = default;

		template<typename RealProxy, typename std::enable_if<!std::is_same<typename std::decay<RealProxy>::type, proxy_wrapper>::value, int>::type = 0 >
		proxy_wrapper(RealProxy&& realproxy)
		{
			_impl.reset(new proxy_adaptor<typename std::remove_reference<RealProxy>::type>(realproxy));
		}

		template<typename RealProxy, typename std::enable_if<!std::is_same<typename std::decay<RealProxy>::type, proxy_wrapper>::value, int>::type = 0 >
		proxy_wrapper(const RealProxy& realproxy)
		{
			_impl.reset(new proxy_adaptor<typename std::remove_reference<RealProxy>::type>(realproxy));
		}

	public:
		template<class Handler>
		void async_connect(Handler handler){
			_impl->async_connect(handler);
		}

	private:
		std::shared_ptr<proxy_type_erasure_base> _impl;
	};

}

class proxy_chain : public std::vector<detail::proxy_wrapper>
{
public:
	proxy_chain(boost::asio::io_service& _io):io_(_io){}
	proxy_chain(proxy_chain&& other) = default;
	proxy_chain(const proxy_chain& other) = default;

	void pop_front()
	{
		erase(begin());
	}

	detail::proxychain_adder add(){
		return detail::proxychain_adder(*this);
	}

	template<class Proxy>
	proxy_chain add(const Proxy & proxy)
	{
		push_back(proxy);
		return *this;
	}

	boost::asio::io_service& get_io_service(){
		return io_;
	}

private:
	boost::asio::io_service&	io_;
	friend struct detail::proxychain_adder;
};


template<class Proxy>
detail::proxychain_adder detail::proxychain_adder::operator()( const Proxy & proxy )
{
	// 拷贝构造一个新的.
	chain->add(proxy);
	return *this;
}

inline
detail::proxychain_adder::operator proxy_chain()
{
	return *chain;
}

} // namespace avproxy
