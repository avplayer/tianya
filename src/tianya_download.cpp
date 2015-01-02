
#include <boost/asio.hpp>
#include "tianya_download.hpp"

tianya_download::tianya_download(boost::asio::io_service& io, const list_info&, QObject* parent)
	: QObject(parent)
	, m_io_service(io)
	, m_tianya_context(io)
{
}

tianya_download::~tianya_download()
{
}
