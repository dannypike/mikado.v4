// Copyright (c) Gamaliel Ltd
#include "common/algorithms.h"
#include "common/errorCodes.h"
#include "common/logger.h"
#include "windowsApi/windowsGlobals.h"
#include "windowsApi/windowsProcess.h"

using namespace std;

namespace mikado::windowsApi {

   // To save typing
   namespace bt = boost::posix_time;
   namespace common = mikado::common;
   namespace windowsApi = mikado::windowsApi;
   typedef common::MikadoLog MikadoLog;
   typedef common::MikadoErrorCode MikadoErrorCode; // To save typing

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsProcess::WindowsProcess(string const &appName) : appName_ { appName }
   {
      ZeroMemory(&procInfo_, sizeof(procInfo_));
      ZeroMemory(&startInfo_, sizeof(startInfo_));
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsProcess::~WindowsProcess() {
      shutdown();
      ZeroMemory(&startInfo_, sizeof(startInfo_));
      ZeroMemory(&procInfo_, sizeof(procInfo_));
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::shutdown() {
      MikadoErrorCode result = MikadoErrorCode::MKO_ERROR_NONE;

      shutdownRequested_ = true;
      if (proc_ && proc_->isOpen()) {
         str_info() << "shutting down " << exeFilename_ << ", pid=" << procInfo_.dwProcessId << endl;
         
         result = waitForProcess();
      }

      closeProcessHandles();
      return result;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::waitForProcess(DWORD *exitCode) {
      if (!wasStarted_) {
         return MikadoErrorCode::MKO_ERROR_PROCESS_NOT_STARTED;
      }

      auto running = true;
      auto result = MikadoErrorCode::MKO_STATUS_NOOP;
      boost::posix_time::ptime since = bt::second_clock::local_time();
      while (running) {
         auto handle = (HANDLE)*proc_;
         if (!GetExitCodeProcess(handle, &exitCode_)) {
            str_info() << "GetExitCodeProcess(" << procInfo_.dwProcessId << ") failed with error "
               << windowsApi::getLastErrorAsString() << endl;
            running = false;
         }
         else if (STILL_ACTIVE == exitCode_) {
            Sleep(50);

            if ((0 < shutdownTimeout_)
                  && bt::milliseconds(shutdownTimeout_) <= (bt::second_clock::local_time() - since)) {
               str_error() << "Process " << procInfo_.dwProcessId << " did not exit after "
                  << shutdownTimeout_ << "ms, killing it" << endl;

               running = false;
               if (auto rc = TerminateProcess(handle, 0); ERROR_SUCCESS != rc) {
                  str_error() << "TerminateProcess failed with error: " << getLastErrorAsString() << endl;
               }
               result = MikadoErrorCode::MKO_ERROR_PROCESS_TERMINATED;
            }
         }
         else {
            running = false;
            result = MikadoErrorCode::MKO_ERROR_NONE;
            str_debug() << "Process " << procInfo_.dwProcessId << " exited with code " << exitCode_ << endl;
         }
      }
      if (exitCode) {
         *exitCode = exitCode_;
      }
      return result;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::closeProcessHandles() {
      auto rc = (proc_ && proc_->isOpen()) || (thread_ && thread_->isOpen())
         ? MikadoErrorCode::MKO_STATUS_NOOP
         : MikadoErrorCode::MKO_ERROR_NONE
         ;
      proc_.reset();
      thread_.reset();
      pipeStderr_.reset();
      pipeStdout_.reset();
      pipeStdin_.reset();
      return rc;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::setFolder(std::filesystem::path const &folder) {
      if (exists(folder)) {
         startFolder_ = folder;
      }
      else if (!folder.empty()) {
         str_error() << "folder not found: " << folder;
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::setExecutable(std::filesystem::path const &filename) {
      exeFilename_ = filename;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::addArgument(string const &arg) {
      if (arg.empty()) {
         args_.push_back("\"\"");
      }
      else {
         args_.push_back(arg);
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsProcess::run() {
      pipeStderr_ = make_shared<WindowsPipe>("stderr");
      if (auto rc = pipeStderr_->create(); MKO_IS_ERROR(rc)) {
         return rc;
      }
      pipeStdout_ = make_shared<WindowsPipe>("stdout");
      if (auto rc = pipeStdout_->create(); MKO_IS_ERROR(rc)) {
         return rc;
      }
      pipeStdin_ = make_shared<WindowsPipe>("stdin");
      if (auto rc = pipeStdin_->create(); MKO_IS_ERROR(rc)) {
         return rc;
      }

      startInfo_.cb = sizeof(startInfo_);
      startInfo_.hStdError = *pipeStderr_->GetReadHandle();
      startInfo_.hStdOutput = *pipeStdout_->GetReadHandle();
      startInfo_.hStdInput = *pipeStdout_->GetWriteHandle();
      startInfo_.dwFlags |= STARTF_USESTDHANDLES;

      string currentFolder = startFolder_.string();

      // Build the command-line from the args vector
      stringstream ss;
      for (auto aa : args_) {
         auto quoted = (aa.empty() || (string::npos != aa.find(' ')));
         if (quoted) {
            ss << " \"" << aa << "\"";
         }
         else {
            ss << " " << aa;
         }
      }
      ss.flush();
      cmdLine_ = ss.str();

      auto appPath = common::lexicalPath(exeFilename_, true, &startFolder_);
      if (!exists(appPath)) {
         str_error() << "executable not found: " << appPath << endl;
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }
      BOOL started = CreateProcess(
         appPath.string().c_str(),  // full path to the executable 
         (LPSTR)cmdLine_.c_str(),   // command line excluding the EXE name
         NULL, // process security attributes 
         NULL, // primary thread security attributes 
         TRUE, // handles are inherited 
         0,    // creation flags 
         NULL, // use parent's environment 
         currentFolder.c_str(),  // starting current directory
         &startInfo_,            // STARTUPINFO pointer 
         &procInfo_              // receives PROCESS_INFORMATION 
         );
      if (!started) {
         str_error() << "CreateProcess(" << appPath << ") in folder " << currentFolder
            << " failed with error: " << getLastErrorAsString() << endl;
         return MikadoErrorCode::MKO_ERROR_PROCESS_NOT_STARTED;
      }
      wasStarted_ = true;
      str_info() << "CreateProcess(" << appPath << ") in folder " << currentFolder << " succeeded, pid="
         << procInfo_.dwProcessId << endl;

      // Take ownership of the process handle
      auto debugName = string("process-") + exeFilename_.filename().string();
      proc_ = make_shared<WindowsHandle>(debugName);
      proc_->attach(&procInfo_.hProcess);
      thread_ = make_shared<WindowsHandle>(debugName);
      thread_->attach(&procInfo_.hThread);

      MikadoErrorCode result = MikadoErrorCode::MKO_ERROR_PROCESS_FAILED;
      if (auto rc = waitForProcess(&exitCode_); MKO_IS_ERROR(rc)) {
         str_error() << "waitForProcess(" << (HANDLE)*proc_ << ") failed with error: " << rc << endl;
         return rc;
      }
      str_info() << "Process " << procInfo_.dwProcessId << " exited with code " << exitCode_ << endl;
      closeProcessHandles();
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::windowsApi
