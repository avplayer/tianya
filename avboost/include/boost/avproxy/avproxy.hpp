
#pragma once

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#ifdef BOOST_ENABLE_SSL
#include <boost/asio/ssl.hpp>
#endif


#include "detail/connect_condition.hpp"
#include "detail/proxy_chain.hpp"

#include "proxy_connect.hpp"
#include "proxies.hpp"
#include "auto_proxychain.hpp"
