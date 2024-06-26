#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_LOGGER_H)
#define CMN_LOGGER_H

namespace mikado::common {

	class MikadoLog {
	public:
		MikadoLog();
		~MikadoLog();
		static std::string StripFolder;
		static MikadoLog MikadoLogger;

		typedef enum {
			MKO_LOG_NONE = 0,
			MKO_LOG_DEBUG = 1,
			MKO_LOG_INFO = 2,
			MKO_LOG_NOTICE = 3,
			MKO_LOG_WARN = 4,
			MKO_LOG_ERROR = 5,
		} LogLevel;

		typedef enum {
         MKO_LOG_OUTPUT_NONE = 0,
         MKO_LOG_OUTPUT_CONSOLE = 1,
         MKO_LOG_OUTPUT_FILE = 2,
         MKO_LOG_OUTPUT_SYSLOG = 3,
      } LogOutput;

	public:
		std::ostream &streamDebug(std::source_location const &loc = std::source_location::current());
		std::ostream &streamInfo(std::source_location const &loc = std::source_location::current());
		std::ostream &streamNotice(std::source_location const &loc = std::source_location::current());
		std::ostream &streamWarn(std::source_location const &loc = std::source_location::current());
		std::ostream &streamError(std::source_location const &loc = std::source_location::current());
      std::ostream &logException(std::exception const &ex, std::source_location const &loc = std::source_location::current());
      std::ostream &logException(std::runtime_error const &ex, std::source_location const &loc = std::source_location::current());

      void setOutputODS(bool value) {
         Redirector::setOutputODS(value);
      }
      void setOutputStdout(bool value) {
         Redirector::setOutputStdout(value);
      }

	protected:
		std::ostream &streamLog(std::string const &logType, std::source_location const &loc);
		std::ostream &logHeader(std::ostream &str, std::source_location const &loc);
		std::ostream &logTrailer(std::ostream &str, std::string const &logTye, std::source_location const &loc);
		std::string cleanFunction(std::string const &fullName);

	private:
		// Prevent copying.                        
		MikadoLog(MikadoLog const &) = delete;
		MikadoLog &operator=(MikadoLog const &) = delete;

		// Intercept clog for debug logging
		class Redirector : public boost::iostreams::sink {
		public:
			Redirector() {}

			std::streamsize write(const char *s, std::streamsize n);

         static void setOutputODS(bool value) {
            outputODS_ = value;
         }
         static void setOutputStdout(bool value) {
            outputStdout_ = value;
         }

      private:
         static bool outputODS_;
         static bool outputStdout_;
      };
		boost::iostreams::stream_buffer<Redirector> buffer_;
		std::streambuf *oldClog_ = nullptr;
		std::ostream cnull_;
	};
} // namespace mikado::common

#define str_debug(...) mikado::common::MikadoLog::MikadoLogger.streamDebug(__VA_ARGS__)
#define str_info(...) mikado::common::MikadoLog::MikadoLogger.streamInfo(__VA_ARGS__)
#define str_notice(...) mikado::common::MikadoLog::MikadoLogger.streamNotice(__VA_ARGS__)	// Leave out the chrome. Print only the message to the outputs
#define str_warn(...) mikado::common::MikadoLog::MikadoLogger.streamWarn(__VA_ARGS__)
#define str_error(...) mikado::common::MikadoLog::MikadoLogger.streamError(__VA_ARGS__)
#define log_exception(ex, ...) mikado::common::MikadoLog::MikadoLogger.logException(ex, __VA_ARGS__)

#endif // CMN_LOGGER_H
