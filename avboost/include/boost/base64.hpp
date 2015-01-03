#pragma once

#include <string>
#include <algorithm>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/algorithm/string.hpp>

namespace boost {

typedef	archive::iterators::transform_width< 
			archive::iterators::binary_from_base64<boost::archive::iterators::remove_whitespace< char* > >, 8, 6, char>
				base64decodeIterator;

typedef	archive::iterators::base64_from_binary<
			archive::iterators::transform_width<std::string::iterator , 6, 8, char> >
				base64encodeIterator;

// BASE64 解码.
inline std::string base64_decode(std::string str)
{
	static int shrik_map[] = {0, 2, 1};

	// 移除尾部的  == 后面的 \r\n\r\n
	while ( boost::is_any_of("\r\n.")(* str.rbegin()))
		str.erase(str.length()-1);
	// 统计结尾的 = 数目
	std::size_t	num = 0;
	if (str.find_last_of("=")!=std::string::npos)
		num = str.find_last_of("=") - str.find_first_of("=") + 1;

	BOOST_ASSERT(num < 3);

	std::size_t num_to_shrik = shrik_map[num];

	// convert base64 characters to binary values
	std::string  result(base64decodeIterator(str.c_str()), base64decodeIterator(str.c_str() + str.length()));

	result.erase(result.length() -1 - num_to_shrik,  num_to_shrik);
	return result;
}

// BASE64 编码.
inline std::string base64_encode(std::string src)
{
	std::vector<char> result(src.length()/3*4+6);
	unsigned one_third_len = src.length()/3;

	unsigned len_rounded_down = one_third_len*3;
	unsigned j = len_rounded_down + one_third_len;

	std::string	base64str;

	// 3 的整数倍可以编码.
	std::copy(base64encodeIterator(src.begin()), base64encodeIterator(src.begin() + len_rounded_down), result.begin());

	base64str = result.data();

	// 结尾 0 填充以及使用 = 补上
	if (len_rounded_down != src.length())
	{
		std::string tail;
		tail.resize(4, 0);
		unsigned i=0;
		for(; i < src.length() - len_rounded_down; ++i)
		{
			tail[i] = src[len_rounded_down+i];
		}

		std::string tailbase;
		tailbase.resize(5);

		std::copy(base64encodeIterator(tail.begin()), base64encodeIterator(tail.begin() + 3), tailbase.begin());
		for (int k=0;k<(3-i);k++){
			tailbase[3-k] = '=';
		}
		
		base64str += tailbase.data();
	}
	return base64str;
}

// BASE64 编码.
template<int N,  class OStreamIterator>
void base64_encode(std::string src, OStreamIterator ostream_iterator)
{
	unsigned one_third_len = src.length()/3;

	unsigned len_rounded_down = one_third_len*3;
	unsigned j = len_rounded_down + one_third_len;

	std::string	base64str;

	typedef boost::archive::iterators::insert_linebreaks<base64encodeIterator, N, char> base64encode_linebreaks_Iterator;
	// 3 的整数倍可以编码.
	std::copy(base64encode_linebreaks_Iterator(src.begin()), base64encode_linebreaks_Iterator(src.begin() + len_rounded_down),
		ostream_iterator
	);

	// 结尾 0 填充以及使用 = 补上
	if (len_rounded_down != src.length())
	{
		std::string tail;
		tail.resize(4, 0);
		unsigned i=0;
		for(; i < src.length() - len_rounded_down; ++i)
		{
			tail[i] = src[len_rounded_down+i];
		}

		std::string tailbase;
		tailbase.resize(5);

		std::copy(base64encode_linebreaks_Iterator(tail.begin()), base64encode_linebreaks_Iterator(tail.begin() + 3), tailbase.begin());
		for (int k=0;k<(3-i);k++){
			tailbase[3-k] = '=';
		}
		
		std::string base64str(tailbase.data());
		std::copy(base64str.begin(), base64str.end(), ostream_iterator);
	}
}

}
