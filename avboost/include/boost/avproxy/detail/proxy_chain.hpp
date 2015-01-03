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

	class proxy_base;
}

class proxy_chain {
public:
	proxy_chain(boost::asio::io_service& _io):io_(_io){}

	detail::proxy_base * back(){
		return m_chain.back().get();
	}
	
	// 克隆一个自己，然后弹出第一个元素.
	// 也就是克隆一个少一个元素的 proxy_chain
	proxy_chain clone_poped(){
		proxy_chain p(*this);
		p.m_chain.pop_back();
		return p;
	}

	detail::proxychain_adder add(){
		return detail::proxychain_adder(*this);
	}

	template<class Proxy>
	proxy_chain  add(const Proxy & proxy)
	{
		// 拷贝构造一个新的.
		detail::proxy_base * newproxy = new Proxy(proxy);
		add_proxy_base(newproxy);
		return *this;
	}

	boost::asio::io_service& get_io_service(){
		return io_;
	}
protected:
	void add_proxy_base(detail::proxy_base * proxy){
		m_chain.insert(m_chain.begin(), boost::shared_ptr<detail::proxy_base>(proxy) );
	}
private:
	boost::asio::io_service&	io_;
	std::vector<boost::shared_ptr<detail::proxy_base> > m_chain;

	friend struct detail::proxychain_adder;
};

namespace detail{
	class proxy_base {
	public:
		typedef boost::function< void (const boost::system::error_code&) > handler_type;
	public:
		template<class Handler>
		void resolve(Handler handler, proxy_chain subchain){
			_resolve(handler_type(handler), subchain);
		}
		
		template<class Handler>
		void handshake(Handler handler,proxy_chain subchain){
			_handshake(handler_type(handler), subchain);
		}

		virtual ~proxy_base(){}

	private:
		virtual void _resolve(handler_type, proxy_chain subchain) = 0;
		virtual void _handshake(handler_type, proxy_chain subchain) = 0;
	};
}

template<class Proxy>
detail::proxychain_adder detail::proxychain_adder::operator()( const Proxy & proxy )
{
	// 拷贝构造一个新的.
	proxy_base * newproxy = new Proxy(proxy);
	chain->add_proxy_base(newproxy);
	return *this;
}

inline
detail::proxychain_adder::operator proxy_chain()
{
	return *chain;
}

} // namespace avproxy
