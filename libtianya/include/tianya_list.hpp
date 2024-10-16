//
// Copyright (C) 2014 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <memory>
#include <functional>

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
	std::wstring post_time;
};

class tianya_list : public boost::noncopyable
{
public:
	tianya_list(boost::asio::io_service& io)
		: m_io_service(io)
		, m_socket(io)
		, m_resolver(io)
		, m_abort(false)
	{}

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
						}, [](std::exception_ptr){});
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
	void connect_hit_item_fetched(T&& t)
	{
		m_sig_hit_item_fetched.connect(t);
	}

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

			buffer += data.post_time;
			tab = 8 - (data.post_time.size() / tabstop);
			for (int i = 0; i < tab; i++)
				buffer += L"\t";

			buffer += ansi_wide(data.post_url);
			buffer += L"\n";

			std::fputs(wide_utf8(buffer).c_str(), fp.get());
		}
	}

protected:

	void process_handle(boost::asio::yield_context& yield, const url_info& ui);

private:
	boost::asio::io_context& m_io_service;
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
