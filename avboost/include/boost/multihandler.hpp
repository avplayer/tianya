#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace boost{


template<class Handler>
class multihandler_op{
	int org_count;
	boost::shared_ptr<int>	m_count;
	Handler m_handler;
public:
	multihandler_op(int count, Handler handler )
	  : org_count(count), m_count(boost::make_shared<int>(count)), m_handler(handler)
	  {
	  }


	void operator()()
	{
		-- (*m_count);
		if (*m_count==0)
		{
			m_handler();
			*m_count = org_count;
		}
	}

	template<class Arg1>
	void operator()(Arg1 arg1)
	{
		-- (*m_count);
		if (*m_count==0)
		{
			m_handler(arg1);
			*m_count = org_count;
		}
	}

	template<class Arg1, class Arg2>
	void operator()(Arg1 arg1, Arg2 arg2)
	{
		-- (*m_count);
		if (*m_count==0)
		{
			m_handler(arg1, arg2);
			*m_count = org_count;
		}
	}

};

template<class Handler>
multihandler_op<Handler> bindmultihandler(int count, Handler handler)
{
	return multihandler_op<Handler>(count, handler);
}

}