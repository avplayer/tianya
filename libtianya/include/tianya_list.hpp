//
// Copyright (C) 2014 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <memory>
#include <functional>

#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "util.hpp"

using boost::asio::ip::tcp;

struct list_info
{
	std::wstring title;
	std::string post_url;
	std::wstring author;
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

	// 存文件.
	void serialize_to_file(std::string name)
	{
		std::unique_ptr<FILE, decltype(&std::fclose)> fp(std::fopen(name.c_str(), "w+b"), &std::fclose);

		if (!fp)
			return;

		for (auto& item : m_hits)
		{
			const list_info& data = item.second;
			const int tabstop = 4;
			std::size_t tab = 0;

			std::wstring buffer;
			std::string gbk;

			gbk = wide_ansi(data.title, "GBK");
			tab = 30 - (gbk.size() / tabstop);
			buffer = data.title;
			for (int i = 0; i < tab; i++)
				buffer += L"\t";

			gbk = wide_ansi(data.author, "GBK");
			tab = 8 - (gbk.size() / tabstop);
			buffer += data.author;
			for (int i = 0; i < tab; i++)
				buffer += L"\t";

			std::string tmp = std::to_string(data.hits);
			tab = 8 - (tmp.size() / tabstop);
			buffer += ansi_wide(tmp);
			for (int i = 0; i < tab; i++)
				buffer += L"\t";

			tmp = std::to_string(data.replys);
			tab = 8 - (tmp.size() / tabstop);
			buffer += ansi_wide(tmp);
			for (int i = 0; i < tab; i++)
				buffer += L"\t";

			buffer += ansi_wide(data.post_time);
			tab = 8 - (data.post_time.size()/ tabstop);
			for (int i = 0; i < tab; i++)
				buffer += L"\t";

			buffer += ansi_wide(data.post_url);
			buffer += L"\n";

			std::fputs(wide_utf8(buffer).c_str(), fp.get());
		}
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

	template<class T>
	void connect_hit_item_fetched(T&& t)
	{
		m_sig_hit_item_fetched.connect(t);
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
			std::string html_line = std::string(begin, begin + bytes_transferred);
			m_response.consume(bytes_transferred);

			// 转成ansi, 主要是因为下面要搜索"下一页"这个字符串.
			html_line = utf8_ansi(html_line);

			if (html_line.find("玄幻 悬疑 情感新作《艾斯拉的救赎》 <连载>") != std::string::npos)
			{
				int a = 0;
			}

			// 开始状态分析.
			switch (m_state)
			{
			case state_unkown:
			{
				if (html_line.find("<td class=\"td-title facered\">") != std::string::npos)
					m_state = state_found;
				if (html_line.find("下一页") != std::string::npos)
				{
					std::string::size_type pos = 0;
					if ((pos = html_line.find_first_of("\"")) != std::string::npos)
					{
						html_line = html_line.substr(pos + 1);
						if ((pos = html_line.find_first_of("\"")) != std::string::npos)
						{
							html_line = html_line.substr(0, pos);
							boost::trim(html_line);
							m_next_page_url = "http://bbs.tianya.cn" + html_line;
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
				if ((pos = html_line.find_first_of("\"")) != std::string::npos)
				{
					html_line = html_line.substr(pos + 1);
					if ((pos = html_line.find_first_of("\"")) != std::string::npos)
					{
						html_line = html_line.substr(0, pos);
						boost::trim(html_line);
						m_info.post_url = "http://bbs.tianya.cn" + html_line;
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
				boost::replace_all(html_line, "<font color=#ff0000>", "");
				boost::replace_all(html_line, "<font color=red>", "");
				boost::replace_all(html_line, "</font>", "");
				boost::replace_all(html_line, "<span class=title_red>", "");
				if ((pos = html_line.find("<span")) != std::string::npos)
					html_line = html_line.substr(0, pos);
				boost::trim(html_line);
				m_info.title = ansi_wide(html_line);
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
				if ((pos = html_line.find("</a>")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.author = ansi_wide(html_line);
				m_state = state_hits;
			}
			break;
			case state_hits:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find("</td>")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.hits = std::atol(html_line.c_str());
				m_state = state_replys;
			}
			break;
			case state_replys:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find("</td>")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of(">")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.replys = std::atol(html_line.c_str());
				m_state = state_time;
			}
			break;
			case state_time:
			{
				std::string::size_type pos = 0;
				if ((pos = html_line.find_last_of("\"")) != std::string::npos)
				{
					html_line = html_line.substr(0, pos);
					if ((pos = html_line.find_last_of("\"")) != std::string::npos)
						html_line = html_line.substr(pos + 1);
				}
				boost::trim(html_line);
				m_info.post_time = html_line;
				m_hits.insert(std::make_pair(m_info.hits, m_info));
				m_replys.insert(std::make_pair(m_info.replys, m_info));

				// 发射信号告诉上层 m_hits 改变了.
				m_sig_hit_item_fetched(boost::ref(m_info));

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
	boost::signals2::signal<void(const list_info&)> m_sig_hit_item_fetched;
	bool m_abort;
};
