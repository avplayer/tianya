
#pragma once

#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include <wchar.h>
#include <string>
#include <boost/locale.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

inline std::string wide_to_utf8(const std::wstring & wstr)
{
#if defined(_WIN32) || defined(_WIN64)
	int required_buffer_size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
	std::string outstr;
	outstr.resize(required_buffer_size);

	int converted_size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr.size(), &outstr[0], required_buffer_size, NULL, NULL);
	outstr.resize(converted_size);

	return outstr;
#else
	return boost::locale::conv::utf_to_utf<char>(wstr);
#endif
}

inline std::string ansi_utf8( std::string const &source, const std::string &characters = "GB18030" )
{
	return boost::locale::conv::between( source, "UTF-8", characters ).c_str();
}

// convert wide string for console output aka native encoding
// because mixing std::wcout and std::cout is broken on windows
inline std::string utf8_ansi( std::string const &source, const std::string &characters = "GB18030" )
{
	return boost::locale::conv::between( source, characters, "UTF-8" ).c_str();
}

// convert utf8 for console output aka native encoding
// for linux ,  it does nothing (linux console always use utf8)
// for windows, it convert to CP_ACP
inline std::string utf8_to_local_encode(const std::string & str)
{
#if defined(_WIN32) || defined(_WIN64)

	std::vector<WCHAR> wstr;

	int required_buffer_size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	wstr.resize(required_buffer_size);

	wstr.resize(MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], wstr.capacity()));

	// 接着转换到本地编码

	required_buffer_size = WideCharToMultiByte(CP_ACP, 0, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
	std::vector<char> outstr;
	outstr.resize(required_buffer_size);

	int converted_size = WideCharToMultiByte(CP_ACP, 0, wstr.data(), wstr.size(), &outstr[0], required_buffer_size, NULL, NULL);
	outstr.resize(converted_size);

	return std::string(outstr.data(),outstr.size());
#else
	return str;
#endif
}

// convert to utf8 from console input aka native encoding
// for linux ,  it does nothing (linux console always use utf8)
// for windows, it convert from CP_ACP
inline std::string local_encode_to_utf8(const std::string & str)
{
#if defined(_WIN32) || defined(_WIN64)

	std::vector<WCHAR> wstr;

	int required_buffer_size = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), NULL, 0);
	wstr.resize(required_buffer_size);

	wstr.resize(MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], wstr.capacity()));

	// 接着转换到UTF8
	required_buffer_size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], wstr.size(), NULL, 0, NULL, NULL);
	std::vector<char> outstr;
	outstr.resize(required_buffer_size);

	int converted_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), &outstr[0], required_buffer_size, NULL, NULL);
	outstr.resize(converted_size);

	return std::string(outstr.data(),outstr.size());
#else
	return str;
#endif
}

// VC 的　"字符串"　是 本地编码的，如果文件是　UTF8 BOM 的话．
// 但是　MinGW 的　"字符串"　是　utf8 的
// linux 下的编译器也全部是　utf8　的
// 但是windows控制台输出的时候，要求的是本地编码
// 而 linux 的控制台则要求的是　utf8　编码
// 于是，就需要把　"字符串"　给确定的转换为本地编码
// 这个代码就是干这个活用的
//#if defined(_MSC_VER) || ( !defined(_WIN32) || !defined(_WIN64) )
//#define literal_to_localstr(x) std::string(x)
//#else
static inline std::string literal_to_localstr(const char* str)
{
//#ifdef _MSC_VER
//#error "vc no use "
//#endif
	return utf8_to_local_encode(str);
}
//#endif
