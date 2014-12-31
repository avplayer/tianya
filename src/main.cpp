
#include <thread>
#include <QApplication>

#include <tianya_list.hpp>

#include "tianyawindow.hpp"
#include "syncobj.hpp"

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

static SyncObjec * _syncobj;

int main(int argc, char *argv[])
{
	// work arround VC
	SyncObjec syncobj;
	_syncobj = &syncobj;
	AsioQApplication app(argc, argv);

	// 创建 主窗口
	TianyaWindow mainwindow(app.get_io_service());

	mainwindow.show();

	return app.exec();
}

void post_on_gui_thread(std::function<void()> func)
{
	_syncobj->do_post(func);
}


#ifdef _WIN32

#ifdef STATIC_QT5
#include <QtPlugin>
#include <windows.h>

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

Q_IMPORT_PLUGIN(QICOPlugin);

#ifdef _DEBUG
#pragma comment(lib, "Qt5PlatformSupportd.lib")
#pragma comment(lib, "qwindowsd.lib")
#pragma comment(lib, "qicod.lib")
#else
#pragma comment(lib, "Qt5PlatformSupport.lib")
#pragma comment(lib, "qwindows.lib")
#pragma comment(lib, "qico.lib")
#endif

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "winmm.lib")

#endif
#endif
