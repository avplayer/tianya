
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/base64.hpp>

#include "smtp.hpp"
namespace mx {
namespace detail {
smtp::smtp( boost::asio::io_service& _io_service, std::string user, std::string passwd, std::string _mailserver )
	: io_service( _io_service )
	, m_work(io_service)
	, m_mailaddr( user )
	, m_passwd( passwd )
	, m_mailserver( _mailserver )
	, m_mailserver_query( "smtp.qq.com", "25" )
{
	// 计算 m_AUTH
	std::vector<char> authtoken( user.length() + passwd.length() + 2 );
	std::copy( user.begin(), user.end(), &authtoken[1] );
	std::copy( passwd.begin(), passwd.end(), &authtoken[user.length() + 2] );

	//ADQ2NDg5MzQ5MAB0ZXN0cGFzc3dk
	m_AUTH = boost::base64_encode( std::string( authtoken.data(), authtoken.size() ) );

	if( m_mailserver.empty() ) { // 自动从　mailaddress 获得.
		if( m_mailaddr.find( "@" ) == std::string::npos )
			m_mailserver = "smtp.qq.com"; // 如果　邮箱是 qq 号码（没@），就默认使用 smtp.qq.com .
		else
			m_mailserver =  std::string( "smtp." ) + m_mailaddr.substr( m_mailaddr.find_last_of( "@" ) + 1 );
	}

	boost::cmatch what;

	if( boost::regex_search( m_mailserver.c_str(), what, boost::regex( "(.*):([0-9]+)" ) ) ) {
		m_mailserver_query = boost::asio::ip::tcp::resolver::query( what[1].str(), what[2].str() );
	} else {
		m_mailserver_query = boost::asio::ip::tcp::resolver::query( m_mailserver, "25" );
	}
}

void smtp::server_cap_handler( std::string cap )
{
	boost::cmatch what;

	// 如果有 STARTTLS ,  就执行 STARTTLS 开启 TLS 加密
	if( cap == "STARTTLS" ) {
		using namespace boost::asio::ssl;
		using namespace boost::asio::ip;
		m_sslctx.reset( new context( context::tlsv1_client ) );
		m_sslsocket.reset(
			new stream<tcp::socket&>( *m_socket, *m_sslctx )
		);
	}
}

}

smtp::smtp( boost::asio::io_service& _io_service, std::string user, std::string passwd, std::string _mailserver )
	: impl_smtp( _io_service, user, passwd, _mailserver )
{
}

}
