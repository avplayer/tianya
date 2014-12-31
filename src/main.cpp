
#include <thread>
#include <QApplication>

#include <tianya_list.hpp>

#include "tianyawindow.hpp"

class AsioQApplication : public QApplication
{
public:
	AsioQApplication(int& argc, char* argv[])
		: QApplication(argc, argv)
	{
		m_asio_thread = std::thread([this]()
		{
			m_work.reset(new boost::asio::io_service::work(m_asio));
			m_asio.run();
		});

		connect(this, &QApplication::aboutToQuit, this, [this]()
		{
			m_asio.post([this](){m_work.reset();});
		});
	}

	boost::asio::io_service& get_io_service(){return m_asio;}

	~AsioQApplication()
	{
		m_asio_thread.join();
	}

private:
	std::thread m_asio_thread;
	boost::asio::io_service m_asio;
	std::unique_ptr<boost::asio::io_service::work> m_work;
};

int main(int argc, char* argv[])
{
	AsioQApplication app(argc, argv);

	// 创建 主窗口
	TianyaWindow mainwindow(app.get_io_service());

	mainwindow.show();

	return app.exec();
}
