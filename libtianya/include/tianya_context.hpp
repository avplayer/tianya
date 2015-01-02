//
// Copyright (C) 2014 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <memory>
#include <functional>
#include <atomic>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>

#include "util.hpp"


using boost::asio::ip::tcp;

struct context_info
{
	std::wstring title;
	std::wstring author;
	std::wstring post_time;
	std::string last_url;
	int hits;
	int replys;
	std::vector<std::wstring> context;
};

namespace impl {

class tianya_context : public std::enable_shared_from_this<tianya_context>
{
public:
	tianya_context(boost::asio::io_service& io)
		: m_io_service(io)
		, m_socket(io)
		, m_resolver(io)
		, m_abort(false)
	{}

public:
	void start(const std::string& post_url)
	{
		auto self = shared_from_this();
		m_post_url = post_url;
		m_state = state_unkown;
		url_info ui = parser_url(m_post_url);
		std::ostringstream port_string;
		port_string.imbue(std::locale("C"));
		port_string << ui.port;
		tcp::resolver::query query(ui.domain, port_string.str());
		m_resolver.async_resolve(query,
		[this, self, ui](const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
		{
			if (!error)
			{
				if (m_socket.is_open())
				{
					boost::system::error_code ignore_ec;
					m_socket.close(ignore_ec);
				}
				boost::asio::async_connect(m_socket, endpoint_iterator,
				[this, self, ui](const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator)
				{
					if (!error)
					{
						boost::asio::spawn(m_io_service,
						[this, self, ui](boost::asio::yield_context yield)
						{
							if (!process_handle(yield, ui))
								m_download_complete();
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

	template <class T>
	void connect_one_content_fetched(T&& t)
	{
		m_one_content_fetched.connect(t);
	}

	template <class T>
	void connect_download_complete(T&& t)
	{
		m_download_complete.connect(t);
	}

	// 存文件.
	void serialize_to_file(std::string name)
	{
		std::unique_ptr<FILE, decltype(&std::fclose)> fp(std::fopen(name.c_str(), "w+b"), &std::fclose);

		if (!fp)
			return;

		// UTF-16 big endian with BOM.
		std::fwrite("\xfe\xff", 2, 1, fp.get());

		for (auto& item : m_context_info.context)
		{
			const std::wstring& buffer = item;
			std::fwrite(buffer.c_str(), buffer.size(), 1, fp.get());
			// std::fputws(buffer.c_str(), fp.get());
		}
	}

protected:

	bool process_handle(boost::asio::yield_context& yield, const url_info& ui)
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
			return false;
		}

		std::size_t bytes_transferred = 0;
		m_response.consume(m_response.size());
		bytes_transferred = boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n", yield[ec]);
		if (ec)
		{
			m_socket.close(ec);
			return false;
		}

		// header读取.
		std::istream response_stream(&m_response);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r")
			std::cout << header << "\n";
		std::cout << "\n";

		std::string html_page_chareset = "utf-8";

		// 循环读取数据.
		while (!m_abort)
		{
			bytes_transferred = boost::asio::async_read_until(m_socket, m_response, "\n", yield[ec]);
			if (ec)
			{
				m_socket.close(ec);
				return false;
			}

			// 取出response的数据到message.
			const char* begin = boost::asio::buffer_cast<const char*>(m_response.data());
			std::string raw_html_line = std::string(begin, begin + bytes_transferred);
			m_response.consume(bytes_transferred);

			// 查找页面编码, 然后转成 std::wstring
			std::wstring html_line;

			html_line = ansi_wide(raw_html_line, html_page_chareset);

			// 开始状态分析.
			switch (m_state)
			{
			case state_unkown:
			{
				if (html_line.find(L"meta charset=") != std::wstring::npos)
					html_page_chareset = detail::get_chareset(raw_html_line); // 获取真正的编码.

				// 如果没有本贴作者的情况下, 先获取本帖作者.
				if (m_context_info.author.empty() && html_line.find(L"<meta name=\"author\" content=") != std::wstring::npos)
				{
					std::wstring::size_type pos = 0;
					if ((pos = html_line.find_last_of(L"\"")) != std::wstring::npos)
					{
						html_line = html_line.substr(0, pos);
						if ((pos = html_line.find_last_of(L"\"")) != std::wstring::npos)
						{
							html_line = html_line.substr(pos + 1);
							boost::trim(html_line);
							m_context_info.author = html_line;
						}
					}
				}

				if (html_line.find(L"<div class=\"bbs-content\"") != std::string::npos)
				{
					m_context.clear();
					m_state = state_div_bbs_content;
				}

				if (html_line.find(L"下页") != std::wstring::npos)
				{
					if (m_next_page_url.empty()) // first found next page, skip.
					{
						m_next_page_url = "skip";
					}
					else
					{
						std::wstring::size_type pos = 0;
						if ((pos = html_line.find_first_of(L"\"")) != std::wstring::npos)
						{
							html_line = html_line.substr(pos + 1);
							if ((pos = html_line.find_first_of(L"\"")) != std::wstring::npos)
							{
								html_line = html_line.substr(0, pos);
								boost::trim(html_line);
								m_next_page_url = wide_utf8(L"http://bbs.tianya.cn" + html_line);
								start(m_next_page_url);
								m_next_page_url = "";
								return true;
							}
						}
					}
				}
			}
			break;
			case state_div_bbs_content:
			{
				if (html_line.find(L"</div>") != std::string::npos)
				{
					m_state = state_author;
					break;
				}
				m_context += html_line;
			}
			break;
			case state_author:
			{
				if (html_line.find(L"author") != std::wstring::npos) // replytime
				{
					boost::wsmatch what;
					boost::wregex ex(L"author=\"(.*?)\"");
					if (boost::regex_search(html_line, what, ex))
					{
						std::wstring author = what[1];
						boost::trim(author);
						if (author == m_context_info.author)
						{
							ex.assign(L"replytime=\"(.*?)\"");
							if (boost::regex_search(html_line, what, ex))
							{
								std::wstring reply_time = what[1];
								boost::trim(m_context);
								boost::replace_all(m_context, L"<br>", L"\n");
								m_context_info.context.push_back(m_context);
								m_one_content_fetched(m_context);
							}
						}
					}
					m_context.clear();
					m_state = state_unkown;
				}
			}
			break;
			}
		}
		return false;
	}

private:
	boost::asio::io_service& m_io_service;
	tcp::socket m_socket;
	tcp::resolver m_resolver;
	boost::asio::streambuf m_request;
	boost::asio::streambuf m_response;
	context_info m_context_info;
	std::wstring m_context;
	std::string m_post_url;
	std::string m_next_page_url;

	// 完全下载完成后发射这个信号.
	boost::signals2::signal<void()> m_download_complete;
	// 每获取到一帖发射这个信号.
	boost::signals2::signal<void(std::wstring)> m_one_content_fetched;

	enum {
		state_unkown,			//
		state_div_bbs_content,	// <div class="bbs-content
		state_author			// <a href="javascript:void(0);" class="reportme a-link" replyid="0" replytime="2010-05-16 22:22:13" author="害我心跳180" authorId="20558716">举报</a> |
	} m_state;
	std::atomic<bool> m_abort;
};

} // namespace impl

class tianya_context : boost::noncopyable
{
public:
	tianya_context(boost::asio::io_service& io)
	{
		m_impl = std::make_shared<impl::tianya_context>(std::ref(io));
	}

	~tianya_context()
	{
		m_impl->stop();
	}

	void start(std::string url)
	{
		m_impl->start(url);
	}

	void stop()
	{
		m_impl->stop();
	}

	template <class T>
	void connect_one_content_fetched(T&& t)
	{
		m_impl->connect_one_content_fetched(t);
	}

	template <class T>
	void connect_download_complete(T&& t)
	{
		m_impl->connect_download_complete(t);
	}


	std::shared_ptr<impl::tianya_context> m_impl;
};
