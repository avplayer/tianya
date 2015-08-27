
#pragma once


namespace avproxy {

namespace detail{

struct avproxy_default_connect_condition{
	template <typename Iterator>
	Iterator operator()(
		const boost::system::error_code& ec,
		Iterator next)
	{
		return next;
	}
};

}

}