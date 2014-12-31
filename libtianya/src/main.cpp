#include <iostream>
#include <map>
#include "../include/tianya_list.hpp"

#include "tianya_list.hpp"

void terminator(boost::asio::io_service& io, tianya& obj)
{
	obj.stop();
}

int main(int argc, char** argv)
{
	boost::asio::io_service io;
	std::string post_url = "http://bbs.tianya.cn/list.jsp?item=culture&grade=1&order=1";
	if (argc == 2)
		post_url = std::string(argv[1]);

	std::locale::global(std::locale(""));

	tianya obj(io);
	obj.start(post_url);

	boost::asio::signal_set terminator_signal(io);
	terminator_signal.add(SIGINT);
	terminator_signal.add(SIGTERM);
#if defined(SIGQUIT)
	terminator_signal.add(SIGQUIT);
#endif // defined(SIGQUIT)
	terminator_signal.async_wait(boost::bind(&terminator, boost::ref(io), boost::ref(obj)));

	io.run();

	std::string name = time_to_string(aux::gettime()) + ".txt";

	obj.serialize_to_file(name);

	return 0;
}
