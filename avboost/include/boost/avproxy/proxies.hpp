#pragma once

#include "detail/proxy_chain.hpp"
#include "detail/proxy_tcp.hpp"
#include "detail/proxy_socks5.hpp"
#include "detail/proxy_http.hpp"

#ifdef BOOST_ENABLE_SSL
#include "detail/proxy_ssl.hpp"
#endif
