
#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <boost/unordered_map.hpp>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/locale.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "boost/base64.hpp"

// that is for output to main.cpp
struct mailcontent {
	std::string		from;
	std::string		to;
	std::string		subject;
	std::string		content_type;                                 // without encoding part
	std::string		content;                                      // already decoded to UTF-8, and the best selected content
};

struct InternetMailFormat;
// MIME 格式的邮件内容.
typedef std::vector<InternetMailFormat> MIMEcontent;

// Internet Mail Format 简化格式.
struct InternetMailFormat {
	// 邮件头部.
	typedef	boost::unordered::unordered_map<std::string, std::string> header_type;
	header_type header;
	bool have_multipart;

	// 头部结束了就是 body了，如果是 MIME 消息，请从 body 构造 MIMEcontent
	boost::variant<std::string, MIMEcontent> body;

	InternetMailFormat() {
		have_multipart = false;
	}

	std::function<void(std::ostream*)> custom_data;
};

namespace detail {

inline std::string ansi_utf8( std::string const &source, const std::string &characters = "GB18030" )
{
	return boost::locale::conv::between( source, "UTF-8", characters ).c_str();
}

inline void mail_address_split( std::vector<std::string>	& out_mails, std::string in_mailline )
{
	boost::split( out_mails, in_mailline, boost::is_any_of( ";," ) );
}

inline std::string imf_base64inline_decode( std::string str )
{
	boost::cmatch what;
	boost::regex ex( "=\\?([^?])?\\?=" );

	while( boost::regex_search( str.c_str(), what, ex ) ) {
		std::string matched_encodedstring;
		matched_encodedstring = what[0];
		ex.set_expression( "(.*)\\?([^?])\\?(.*)" );
		boost::cmatch what2;


		if( boost::regex_search( what[1].str().c_str(), what2, ex ) ) {
			std::string result, encode, charset;
			charset = what2[1];	// 	gb18030
			encode = what2[2]; //  B

			if( encode == "B" ) {
				result = ansi_utf8( boost::base64_decode( what2[3].str() ), charset );
				boost::replace_all( str, matched_encodedstring, result );
			}
		} else {
			boost::replace_all( str, matched_encodedstring, "decodeerror" );
		}
	}

	return str;
}

// if the about to append string will cause the line
// excede 78 char line length limit, break it now
// that's required by IMF
inline void checked_newline_append( std::string & to, std::string add )
{
	std::size_t xpos = 0;
	// 检查最后一个
	std::size_t pos = to.find_last_of( "\r\n" );

	if( pos ) {
		xpos = to.length() - ( pos + 2 );
	} else
		xpos = to.length();

	if( ( xpos + add.length() ) >= 78 )
		to += "\r\n\t";

	to += add;
}

inline std::string imf_base64inline_encode( std::string str )
{
	return boost::str( boost::format( "=?utf8?B?%s?=" ) % boost::base64_encode( str ) );
}

// "小牛 <xxx@qq.com> 这样的文本变成  "=?utf8?B?Cg==?=  <xxx@qq.com>"
inline void imf_mailaddr_base64inline_encode( std::string & out_addressline, std::string str )
{
	std::vector<std::string> mails;
	mail_address_split( mails, str );

	BOOST_FOREACH( std::string & mail, mails ) {
		boost::cmatch	what;
		boost::regex	ex( "[\"]?([^\\s\"]*)?\"?\\s*(<.*@.*?>)" );

		// 有的是 u@d 的形式,  有的是 "name" <u@d> 的形式呢
		if( boost::regex_search( mail.c_str(), what, ex ) ) {
			std::string tmp;
			// 是的话, 咱就可以编码了,
			std::string mailaddress = what[2];
			std::string name = what[1];

			boost::trim_if( name, boost::is_any_of( "\" " ) );
			boost::trim( mailaddress );

			checked_newline_append( out_addressline,
									boost::str( boost::format( "\"%s\" %s; " ) % imf_base64inline_encode( what[1] ) % mailaddress )
								  );
		} else
			checked_newline_append( out_addressline, mail + "; " );
	}
}


inline std::pair<std::string, std::string> process_line( const std::string & line )
{
	boost::regex ex( "([^:]*)?:[ \t]+(.*)?" );
	boost::cmatch what;

	if( boost::regex_match( line.c_str(), what, ex, boost::match_all ) ) {
		std::string key = what[1];
		std::string val = what[2];
		boost::to_lower( key );
		return std::make_pair( key, imf_base64inline_decode( val ) );
	}

	throw( boost::bad_expression( "not matched" ) );
}
}

// 从 string 构造一个 MIMEcontent
template<class InputStream>
void mime_read_stream( MIMEcontent& mimecontent, InputStream &in, const std::string &boundary );

// 从 in stream 构造一个 InternetMailFormat
template<class InputStream>
void imf_read_stream( InternetMailFormat& imf, InputStream &in );

inline void imf_decode_multipart( InternetMailFormat &imf );

// 从 in stream 构造一个 InternetMailFormat
template<class InputStream>
void imf_read_stream( InternetMailFormat& imf, InputStream &in )
{
	std::string line;
	std::string pre_line;
	std::string body;
	imf.have_multipart = false;

	std::getline( in, pre_line );

	// 要注意处理 folding
	// 状态机开始!
	// 以行为单位解析
	while( !in.eof() ) {
		line.clear();

		std::getline( in, line );
		boost::trim_right( line );

		if( !line.empty() && boost::is_space()( line[0] ) ) { // 开头是 space ,  则是 folding
			boost::trim_left( line );
			pre_line += line;
			continue;
		} else {
			if( !pre_line.empty() ) {

				// 之前是 fold  的，所以 pre_line 才是正确的一行.
				try {
					imf.header.insert( detail::process_line( pre_line ) );
				} catch( const boost::bad_expression & e )
					{}
			} else if( pre_line.empty() && !line.empty() ) {
				// 应该是 body 模式了.
				body = line + "\r\n";

				while( !in.eof() ) {
					line.clear();
					std::getline( in, line );
					body += line + "\n";
				}

				// 依据 content-franster-encoding 解码 body
				std::string content_transfer_encoding = imf.header["content-transfer-encoding"];

				if( boost::to_lower_copy( content_transfer_encoding ) == "base64" ) {
					imf.body = boost::base64_decode( body );
				} else {
					imf.body = body;
				}

				imf_decode_multipart( imf );
				return;
			}

			pre_line =  line;
		}
	}
}

// 从 string 构造一个 MIMEcontent
template<class InputStream>
void mime_read_stream( MIMEcontent& mimecontent, InputStream &in, const std::string & boundary )
{
	// 以 boundary 为分割线，完成后的分割，又可以 InternetMailFormat 的格式解析
	// 等待进入 multipart 部分.
	// 以行模式进行解析
	while( !in.eof() ) {
		std::string line;
		std::getline( in, line );

		if( boost::trim_copy( line ) == std::string( "--" ) + boundary ) {
			std::string part;

			// 进入 multipart 部分.
			while( !in.eof() ) {
				std::string line;
				std::getline( in, line );

				if( ( boost::trim_copy( line ) == std::string( "--" ) + boundary ) || ( boost::trim_copy( line ) == std::string( "--" ) + boundary + "--" ) ) {
					// 退出 multipart
					// part 就是一个 IMF 格式了
					std::stringstream partstream( part );
					InternetMailFormat imf;
					imf_read_stream( imf, partstream );
					mimecontent.push_back( imf );
					part.clear();
					continue;
				}

				part += ( line + "\n" );
			}
		}
	}
}

inline void imf_decode_multipart( InternetMailFormat &imf )
{
	std::string content_type = imf.header["content-type"];

	boost::cmatch what;
	boost::regex ex( "multipart/.*;.*?boundary=\"(.*)?\"" );

	if( boost::regex_search( content_type.c_str(), what, ex ) ) {
		imf.have_multipart = true;
		std::string boundary = what[1];
		MIMEcontent mime;

		std::stringstream bodystream( boost::get<std::string>( imf.body ) );
		mime_read_stream( mime, bodystream, boundary );
		imf.body = mime;
	}

	return;
}

// 从 InternetMailFormat 输出 stream
template<class OutputStream>
void imf_write_stream( InternetMailFormat imf, OutputStream &out )
{

	// 暂时不支持 MIME 格式.
	BOOST_ASSERT( imf.have_multipart == false );

	std::string headerdata;

	// 遍历头部.
	BOOST_FOREACH( auto hl, imf.header ) {
		headerdata += hl.first + ": ";

		if( hl.first == "from" || hl.first == "to" ) {
			detail::imf_mailaddr_base64inline_encode( headerdata, hl.second );
		} else if( hl.first == "subject" ) {
			detail::checked_newline_append( headerdata, detail::imf_base64inline_encode( hl.second ) );
		} else {
			detail::checked_newline_append( headerdata, hl.second );
		}

		headerdata += "\r\n";
	}

	out <<  headerdata << "content-transfer-encoding: base64\r\n\r\n";

	// 好了, 头部都生成了, 接下来是, 身体.

	// 身体直接 base64 编码得了.
	boost::base64_encode<78>( boost::get<std::string>( imf.body ), std::ostream_iterator<char>( out ) );
}


