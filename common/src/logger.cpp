// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "windowsApi.h"

namespace bio = boost::iostreams;
using namespace std;

namespace mikado::common {

	MikadoLog MikadoLog::MikadoLogger;
	string MikadoLog::StripFolder;

   bool MikadoLog::Redirector::outputODS_ = true;
   bool MikadoLog::Redirector::outputStdout_ = true;

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoLog::MikadoLog() : cnull_(nullptr) {
		// MikadoLogger is supposed to be a singleton
		static bool MikadoLogAlreadyCreated = false;
		assert(!MikadoLogAlreadyCreated);
		MikadoLogAlreadyCreated = true;

		buffer_.open(Redirector());
		oldClog_ = clog.rdbuf(&buffer_);
	}

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoLog::~MikadoLog() {
		if (oldClog_) {
			clog.rdbuf(oldClog_);
			oldClog_ = nullptr;
		}
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::streamLog(string const &logType, source_location const &loc) {
		logHeader(clog, loc);
		clog << "): <" << this_thread::get_id() << "-";
		return logTrailer(clog, logType, loc);
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::streamDebug(source_location const &loc) {
		static string logType("DEBUG>: ");
		return true ? streamLog(logType, loc) : cnull_;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::streamNotice(source_location const &loc) {
		// Don't output the filename etc. Only output the message itself.
		return clog;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::streamInfo(source_location const &loc) {
		static string logType("INFO>:  ");
		return true ? streamLog(logType, loc) : cnull_;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::streamWarn(source_location const &loc) {
		static string logType("WARN>:  ");
		return true ? streamLog(logType, loc) : cnull_;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::streamError(source_location const &loc) {
		static string logType("ERROR>: ");
		return true ? streamLog(logType, loc) : cnull_;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::logException(exception const &ex, source_location const &loc) {
      string what(ex.what());
      logHeader(clog, loc);
      return clog << "): <EXCEPTION>: " << common::trim(what)
         << endl << windowsApi::StackTrace(3).toString(common::kSixSpaces);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::logException(runtime_error const &ex, source_location const &loc) {
      string what(ex.what());
      
      logHeader(clog, loc);
		return clog << "): <RUNTIME ERROR>: " << common::trim(what)
         << endl << windowsApi::StackTrace(3).toString(common::kSixSpaces);
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::logHeader(ostream &str, source_location const &loc) {
		return str << loc.file_name() << "(" << loc.line();
	}

   ///////////////////////////////////////////////////////////////////////
   //
   ostream &MikadoLog::logTrailer(ostream &str, string const &logType, source_location const &loc) {
		return str
			//<< cleanFunction(loc.function_name())
			<< logType;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   string MikadoLog::cleanFunction(string const &fullName) {
		auto paren = fullName.find_last_of('(');
		if (string::npos != paren) {
			auto space = fullName.rfind(' ', paren);
			if ((string::npos != space) && (paren > (space = ++space))) {
				return fullName.substr(space, paren - space);
			}
		}
		return fullName;
	}

   ///////////////////////////////////////////////////////////////////////
   //
   std::streamsize MikadoLog::Redirector::write(const char *s, std::streamsize n) {
		string text(s, n);

      if (outputODS_) {
         OutputDebugStringA(text.c_str());
      }

      if (outputStdout_) {
	      // Strip the unwanted characters from the start of the debug message
		   // for outputting to the console
		   if (!StripFolder.empty() && text.starts_with(StripFolder)) {
			   text = text.substr(StripFolder.size() - 1);
			   int padCount = 0;
			   if (auto pos = text.find("): <"); string::npos != pos) {
				   string padding(max(0, 30 - (int)pos), '.');
				   cout << padding << text;
			   }
         }
		   else
		   {
			   cout << text;
		   }
      }

		return n;
	}

} // namespace mikado::common
