
#include "tianya_list.hpp"

void tianya_list::process_handle(boost::asio::yield_context& yield, const url_info& ui)
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

	std::size_t bytes_transferred = 0;
	m_response.consume(m_response.size());
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

	std::string html_page_chareset = "utf-8";

	// 循环读取数据.
	while (!m_abort)
	{
		bytes_transferred = boost::asio::async_read_until(m_socket, m_response, "\n", yield[ec]);
#ifdef _WIN32
		Sleep(0);
#endif
		if (ec)
		{
			m_socket.close(ec);
			return;
		}

		// 取出response的数据到message.
		std::string raw_html_line;
		raw_html_line.resize(bytes_transferred);
		m_response.sgetn(&raw_html_line[0], bytes_transferred);

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
						m_next_page_url = wide_utf8(L"http://bbs.tianya.cn" + html_line);
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
					m_info.post_url = wide_utf8(L"http://bbs.tianya.cn" + html_line);
					m_info.title.clear();
					m_state = state_name;
					break;
				}
			}
			m_state = state_unkown;
		}
		break;
		case state_name:
		{
			while (true)
			{
				boost::wsmatch what;
				boost::wregex ex(L"</{0,1}(font|span|strong)(.*?)>");
				if (boost::regex_search(html_line, what, ex))
				{
					std::wstring str = what.str();
					boost::replace_all(html_line, str, L"");
				}
				else
				{
					break;
				}
			}
			boost::trim(html_line);
			if (html_line.find(L"</a") != std::string::npos)
			{
				m_state = state_skip5;
				boost::trim_if(m_info.title, boost::is_any_of(" \t\r"));
				break;
			}
			m_info.title += html_line;
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
			m_info.author = html_line;
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
			m_info.hits = std::atol(wide_utf8(html_line).c_str());
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
			m_info.replys = std::atol(wide_utf8(html_line).c_str());
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
