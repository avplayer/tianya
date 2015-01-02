//
// Copyright (C) 2014 Jack.
//
// Author: jack
// Email:  jack.wgm@gmail.com
//

#pragma once

#include <cstdlib> // for mbtowc, wctomb.
#include <cstring>
#include <cstdio>

#include <boost/locale.hpp>
#include <boost/locale/utf.hpp>

#ifdef _MSC_VER
#include <Mmsystem.h>
#pragma comment(lib, "Winmm.lib")
#endif // _MSC_VER

// 字符集编码转换接口声明.

// wide string to utf8 string.
inline std::string wide_utf8(const std::wstring& source);

// utf8 string to wide string.
inline std::wstring utf8_wide(std::string const& source);

// ansi string to utf8 string.
inline std::string ansi_utf8(std::string const& source);
inline std::string ansi_utf8(
	std::string const& source, const std::string& characters);

// utf8 string to ansi string.
inline std::string utf8_ansi(std::string const& source);
inline std::string utf8_ansi(
	std::string const& source, const std::string& characters);

// wide string to ansi string.
inline std::string wide_ansi(const std::wstring& source);
inline std::string wide_ansi(
	std::wstring const& source, const std::string& characters);

// ansi string to wide string.
inline std::wstring ansi_wide(const std::string& source);
inline std::wstring ansi_wide(
	const std::string& source, const std::string& characters);

// 字符集编码转换接口实现, 使用boost实现.
inline std::wstring ansi_wide(
	const std::string& source, const std::string& characters)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(ansi_utf8(source, characters));
}

inline std::wstring ansi_wide(const std::string& source)
{
	std::wstring wide;
	wide.reserve(source.size());
	wchar_t dest;
	std::size_t max = source.size();

	// reset mbtowc.
	std::mbtowc(NULL, NULL, 0);

	// convert source to wide.
	for (std::string::const_iterator i = source.begin();
		i != source.end();)
	{
		int length = std::mbtowc(&dest, &(*i), max);
		if (length < 1)
			break;
		max -= length;
		while (length--) i++;
		wide.push_back(dest);
	}

	return wide;
}

inline std::string ansi_utf8(
	std::string const& source, const std::string& characters)
{
	return boost::locale::conv::between(source, "UTF-8", characters);
}

inline std::string ansi_utf8(std::string const& source)
{
	std::wstring wide = ansi_wide(source);
	return wide_utf8(wide);
}

inline std::string wide_utf8(const std::wstring& source)
{
	return boost::locale::conv::utf_to_utf<char>(source);
}

inline std::wstring utf8_wide(std::string const& source)
{
	return boost::locale::conv::utf_to_utf<wchar_t>(source);
}

inline std::string utf8_ansi(
	std::string const& source, const std::string& characters)
{
	return boost::locale::conv::between(source, characters, "UTF-8");
}

inline std::string utf8_ansi(std::string const& source)
{
	return wide_ansi(utf8_wide(source));
}

inline std::string wide_ansi(
	std::wstring const& source, const std::string& characters)
{
	return utf8_ansi(wide_utf8(source), characters);
}

inline std::string wide_ansi(const std::wstring& source)
{
	std::size_t buffer_size = MB_CUR_MAX;
	std::vector<char> buffer(buffer_size, 0);
	std::string ansi;

	// reset wctomb.
	std::wctomb(NULL, 0);

	// convert source to wide.
	for (std::wstring::const_iterator i = source.begin();
		i != source.end(); i++)
	{
		int length = std::wctomb(&(*buffer.begin()), *i);
		if (length < 1)
			break;
		for (int j = 0; j < length; j++)
			ansi.push_back(buffer[j]);
	}

	return ansi;
}

namespace aux {

	inline int64_t gettime()
	{
#if defined (WIN32) || defined (_WIN32)
		static const unsigned __int64 epoch = 116444736000000000L;	/* Jan 1, 1601 */
		typedef union {
			unsigned __int64 ft_scalar;
			FILETIME ft_struct;
		} LOGGING_FT;

		static int64_t system_start_time = 0;
		if (system_start_time == 0)
		{
			LOGGING_FT nt_time;
			GetSystemTimeAsFileTime(&(nt_time.ft_struct));
			int64_t tim = (__int64)((nt_time.ft_scalar - epoch) / 10000i64);
			system_start_time = tim - timeGetTime();
#if 0
			struct timeval tv;
			gettimeofday(&tv, NULL);
			int64_t millsec = ((int64_t)tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
			system_start_time = millsec - timeGetTime();
#endif
		}
		return system_start_time + timeGetTime();
#elif defined (__linux__)
		struct timeval tv;
		gettimeofday(&tv, NULL);
		return ((int64_t)tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
#endif
	}
}

inline std::string time_to_string(int64_t time)
{
	std::string ret;
	std::time_t rawtime = time / 1000;
	struct tm * ptm = std::localtime(&rawtime);
	if (!ptm)
		return ret;
	char buffer[1024];
	std::sprintf(buffer, "%04d-%02d-%02dT%02d-%02d-%02d",
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	ret = buffer;
	return ret;
}

struct url_info
{
	url_info()
		: port(-1)
	{}

	std::string protocol;
	std::string domain;
	int port;
	std::string path;
	std::string query;
	std::string fragment;
};

template<typename ChatType>
url_info parser_url(const std::basic_string<ChatType>& url)
{
	url_info ret;
	boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
	boost::cmatch what;
	if (boost::regex_match(url.c_str(), what, ex))
	{
		ret.protocol = std::string(what[1].first, what[1].second);
		ret.domain = std::string(what[2].first, what[2].second);
		std::string port = std::string(what[3].first, what[3].second);
		if (port.empty())
		{
			if (ret.protocol == "http")
				ret.port = 80;
			else if (ret.protocol == "https")
				ret.port = 443;
		}
		else
		{
			ret.port = std::atol(port.c_str());
		}
		ret.path = std::string(what[4].first, what[4].second);
		ret.query = std::string(what[5].first, what[5].second);
		ret.fragment = std::string(what[6].first, what[6].second);
	}

	return ret;
}
