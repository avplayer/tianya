
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
#ifdef _WIN32
			auto ret = ::SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
			BOOST_ASSERT(ret);
#endif
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

#ifdef _WIN32
#include "resource.h"

extern QPixmap qt_pixmapFromWinHICON(HICON);

static QIcon load_win32_icon()
{
	QIcon ico;
	HICON hicon = (HICON)::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TIANYA_ICON),
		IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
	ico = QIcon(qt_pixmapFromWinHICON(hicon));
	::DestroyIcon(hicon);
	return ico;
}
#endif

int main(int argc, char *argv[])
{
	std::locale::global(std::locale(""));
	// work arround VC
	SyncObjec syncobj;
	_syncobj = &syncobj;
	AsioQApplication app(argc, argv);
	app.setOrganizationName("avplayer");
	app.setOrganizationDomain("avplayer.org");
	app.setApplicationName("tianyaradar");
#ifdef _WIN32
	app.setWindowIcon(load_win32_icon());
#else
	app.setWindowIcon(QIcon(":/icon/tianya.svg"));
#endif

	// 创建 主窗口
	TianyaWindow mainwindow(app.get_io_service());

	mainwindow.showMaximized();

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
