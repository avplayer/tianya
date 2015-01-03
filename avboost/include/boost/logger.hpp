#pragma once
#include <iostream>
#include <sstream>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

class tm;
namespace boost{

	class logger;

	class logger_formater : boost::noncopyable
	{
	public:
		logger_formater(logger& _logger, std::string& level)
			: level_(level)
			, m_logger(_logger)
		{}

		logger_formater(logger_formater&&tmp)
			: level_(tmp.level_)
			, m_logger(tmp.m_logger)
			, oss_(tmp.oss_.str())
		{}

		~logger_formater();

		template <class T>
		logger_formater& operator << (T const& v)
		{
			oss_ << v;
			return *this;
		}

		std::ostringstream oss_;
		std::string& level_;
		logger& m_logger;
	};


	namespace {
		static std::string LOGGER_DEBUG_STR = "DEBUG";
		static std::string LOGGER_INFO_STR = "INFO";
		static std::string LOGGER_WARN_STR = "WARNING";
		static std::string LOGGER_ERR_STR = "ERROR";
	}

	class logger : boost::noncopyable
	{

	public:
		boost::signals2::signal<void(std::string, std::string)> write_log;

		logger_formater err(){
			return logger_formater(*this, LOGGER_ERR_STR);
		}
		logger_formater info(){
			return logger_formater(*this, LOGGER_INFO_STR);
		}
		logger_formater warn(){
			return logger_formater(*this, LOGGER_WARN_STR);
		}

		logger_formater dbg(){
			return logger_formater(*this, LOGGER_DEBUG_STR);
		}
	};

	inline logger_formater::~logger_formater()
	{
		std::string message = oss_.str();
		m_logger.write_log(level_, message);
	}

} // namespace
