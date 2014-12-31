//
// Copyright (C) 2014 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <boost/bind.hpp>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>

#include "util.hpp"

using boost::asio::ip::tcp;

struct list_info
{
	std::string title;
	std::string post_url;
	std::string author;
	int hits;
	int replys;
	std::string post_time;
};

class tianya : public boost::noncopyable
{
public:
	tianya(boost::asio::io_service& io)
		: m_io_service(io)
		, m_socket(io)
		, m_resolver(io)
		, m_abort(false)
	{}
	~tianya()
	{
		// 存文件.
		std::string name = time_to_string(aux::gettime()) + ".txt";
		FILE* fp = std::fopen(name.c_str(), "w+b");
		if (!fp)
			return;
		for (auto& item : m_hits)
		{
			const list_info& data = item.second;
			std::size_t tab = 0;

			std::string buffer;

			tab = (120 - (data.title.size() - 3)) / 4;
			buffer = data.title;
			for (int i = 0; i < tab; i++)
				buffer += "\t";

			tab = (32 - (data.author.size() - 3)) / 4;
			buffer += data.author;
			for (int i = 0; i < tab; i++)
				buffer += "\t";

			std::string tmp = std::to_string(data.hits);
			tab = (32 - (tmp.size() - 3)) / 4;
			buffer += tmp;
			for (int i = 0; i < tab; i++)
				buffer += "\t";

			tmp = std::to_string(data.replys);
			tab = (32 - (tmp.size() - 3)) / 4;
			buffer += tmp;
			for (int i = 0; i < tab; i++)
				buffer += "\t";

			buffer += data.post_time;
			tab = (32 - (data.post_time.size() - 3)) / 4;
			for (int i = 0; i < tab; i++)
				buffer += "\t";

			buffer += data.post_url;
			buffer += "\n";
			std::fputs(buffer.c_str(), fp);
		}
		std::fclose(fp);
	}

public:
	void start(const std::string& post_url)
	{
		m_post_url = post_url;
		m_state = state_unkown;
		url_info ui = parser_url(m_post_url);
		std::ostringstream port_string;
		port_string.imbue(std::locale("C"));
		port_string << ui.port;
		tcp::resolver::query query(ui.domain, port_string.str());
		m_resolver.async_resolve(query,
		[this, ui](const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
		{
			if (!error)
			{
				if (m_socket.is_open())
				{
					boost::system::error_code ignore_ec;
					m_socket.close(ignore_ec);
				}
				boost::asio::async_connect(m_socket, endpoint_iterator,
				[this, ui](const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
				{
					if (!error)
					{
						boost::asio::spawn(m_io_service,
						[this, ui](boost::asio::yield_context yield)
						{
							process_handle(yield, ui);
						});
					}
				});
			}
		});
	}

	void stop()
	{
		m_abort = true;
		boost::system::error_code ignore_ec;
		m_socket.close(ignore_ec);
	}

protected:

	void process_handle(boost::asio::yield_context& yield, const url_info& ui)
	{
		boost::system::error_code ec;
		m_request.consume(m_request.size());
		std::ostream request_stream(&m_request);
		std::string path = ui.path;
		if (!ui.query.empty())
		{
			path += "?";
			path += ui.query;
		}
		request_stream << "GET " << path << " HTTP/1.0\r\n";
		request_stream << "Host: " << ui.domain << "\r\n";
		request_stream << "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.95 Safari/537.36\r\n";
		request_stream << "Accept: */*\r\n";
		request_stream << "Connection: close\r\n\r\n";
		boost::asio::async_write(m_socket, m_request, yield[ec]);
		if (ec)
		{
			m_socket.close(ec);
			return;
		}
		m_response.consume(m_response.size());
		std::size_t bytes_transferred = 0;
		bytes_transferred = boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n", yield[ec]);
		if (ec)
		{
			m_socket.close(ec);
			return;
		}

		// header读取.
		std::istream response_stream(&m_response);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
			std::cout << header << "\n";
		std::cout << "\n";

		// 循环读取数据.
		while (!m_abort)
		{
			bytes_transferred = boost::asio::async_read_until(m_socket, m_response, "\n", yield[ec]);
			if (ec)
			{
				m_socket.close(ec);
				return;
			}

			// 取出response的数据到message.
			const char* begin = boost::asio::buffer_cast<const char*>(m_response.data());
			std::string line = std::string(begin, begin + bytes_transferred);
			m_response.consume(bytes_transferred);

			// 转成ansi, 主要是因为下面要搜索"下一页"这个字符串.
			std::wstring html_line = utf8_wide(line);

			// 开始状态分析.
			switch (m_state)
			{
			case state_unkown:
			{
				if (html_line.find(L"<td class=\"td-title facered\">") != std::string::npos)
					m_state = state_found;
				if (html_line.find(L"下一页") != std::string::npos)
				{
					std::string::size_type pos = 0;
					if ((pos = html_line.find_first_of(L"\"")) != std::string::npos)
					{
						html_line = html_line.substr(pos + 1);
						if ((pos = html_line.find_first_of(L"\"")) != std::string::npos)
						{
							html_line = html_line.substr(0, pos);
							boost::trim(html_line);
							m_next_page_url = "http://bbs.tianya.cn" + wide_utf8(html_line);
							start(m_next_page_url);
							m_next_page_url = "";
							return;
						}
					}
				}
			}
			break;
			case state_found:
			{
				m_state = state_skip1;
			}
			break;
			case state_skip1:
			{
				m_state = state_skip2;
			}
			break;
			case state_skip2:
			{
				m_state = state_target;
			}
			break;
			case state_target:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find_first_of(L"\"")) != std::string::npos)
				{
					html_line = html_line.substr(pos + 1);
					if ((pos = html_line.find_first_of(L"\"")) != std::string::npos)
					{
						html_line = html_line.substr(0, pos);
						boost::trim(html_line);
						m_info.post_url = "http://bbs.tianya.cn" + wide_utf8(html_line);
						m_state = state_name;
						break;
					}
				}
				m_state = state_unkown;
			}
			break;
			case state_name:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find(L"<span")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(L">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.title = wide_ansi(html_line);
				m_state = state_skip4;
			}
			break;
			case state_skip4:
			{
				m_state = state_skip5;
			}
			break;
			case state_skip5:
			{
				m_state = state_author;
			}
			break;
			case state_author:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find(L"</a>")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(L">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.author = wide_ansi(html_line);
				m_state = state_hits;
			}
			break;
			case state_hits:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find(L"</td>")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(L">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.hits = std::atol(wide_ansi(html_line).c_str());
				m_state = state_replys;
			}
			break;
			case state_replys:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find(L"</td>")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(L">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.replys = std::atol(wide_ansi(html_line).c_str());
				m_state = state_time;
			}
			break;
			case state_time:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find_last_of(L"\"")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(L"\"")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.post_time = wide_ansi(html_line);
				m_hits.insert(std::make_pair(m_info.hits, m_info));
				m_replys.insert(std::make_pair(m_info.replys, m_info));
				m_state = state_unkown;
			}
			break;
			}
		}
	}

private:
	boost::asio::io_service& m_io_service;
	tcp::socket m_socket;
	tcp::resolver m_resolver;
	boost::asio::streambuf m_request;
	boost::asio::streambuf m_response;
	typedef std::multimap<int, list_info> ordered_info;
	ordered_info m_hits;
	ordered_info m_replys;
	std::string m_post_url;
	std::string m_next_page_url;
	enum {
		state_unkown,
		state_found,
		state_skip1,
		state_skip2,
		state_target,
		state_name,
		state_skip4,
		state_skip5,
		state_author,
		state_hits,
		state_replys,
		state_time
	} m_state;
	list_info m_info;
	bool m_abort;
};
