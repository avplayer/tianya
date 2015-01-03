
#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace boost{
namespace invoke_wrapper{

	template<typename Signature>
	class invoke_once;

#define SV_TEXT(z, n, text) text ## n
#define SV_ARG(z, n, _) Arg ## n arg ## n

#define SINGLE_INVOKER(z, n, _) \
	template<typename R, BOOST_PP_ENUM(BOOST_PP_INC(n), SV_TEXT, typename Arg)> \
	struct invoke_once<R( BOOST_PP_ENUM(BOOST_PP_INC(n), SV_TEXT, Arg) )> \
			{ \
		typedef R(Signature)(BOOST_PP_ENUM(BOOST_PP_INC(n), SV_TEXT, Arg)); \
		boost::shared_ptr<bool> m_invoked; \
		boost::shared_ptr< \
			boost::function<Signature> \
										> m_handler; \
		\
		invoke_once(std::function<Signature> _handler) \
			: m_handler(boost::make_shared< boost::function<Signature> >(_handler)) \
			, m_invoked(boost::make_shared<bool>(false)) \
				{} \
		\
		R operator()(BOOST_PP_ENUM(BOOST_PP_INC(n), SV_ARG, nil)) \
				{ \
			if(!*m_invoked) \
															{ \
				*m_invoked = true; \
				(*m_handler)(BOOST_PP_ENUM(BOOST_PP_INC(n), SV_TEXT, arg)); \
															} \
				} \
		typedef R result_type; \
			};


	BOOST_PP_REPEAT_FROM_TO(0, 10, SINGLE_INVOKER, nil)


} // namespcae invoke_wrapper
} // namespace boost

