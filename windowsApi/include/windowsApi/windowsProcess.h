#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(API_WINDOWSPROCESS_H)
#define API_WINDOWSPROCESS_H

#include "common/errorCodes.h"
#include "windowsHandle.h"
#include "windowsPipe.h"

namespace mikado::windowsApi {

   class WindowsProcess : public std::enable_shared_from_this<WindowsProcess> {

      typedef mikado::common::MikadoErrorCode MikadoErrorCode; // To reduce typing

   public:
      WindowsProcess(std::string const &appName);
      virtual ~WindowsProcess();
      MikadoErrorCode shutdown();
      MikadoErrorCode run();

      MikadoErrorCode setFolder(std::filesystem::path const &folder);
      MikadoErrorCode setExecutable(std::filesystem::path const &filename);
      MikadoErrorCode addArgument(std::string const &arg);
      MikadoErrorCode waitForProcess(DWORD *exitCode = nullptr);
      DWORD getExitCode() const { return exitCode_; }
      void setShutdownTimeout(unsigned timeout) { shutdownTimeout_ = timeout; }
      unsigned getShutdownTimeout() const { return shutdownTimeout_; }

      bool isRunning() const {
         return isRunning_;
      }
      bool startFailed() const {
         return startFailed_;
      }

   protected:
      MikadoErrorCode doRun();
      MikadoErrorCode closeProcessHandles();

   private:
      std::recursive_mutex mux_;

      bool shutdownRequested_ = false;
      unsigned shutdownTimeout_ = 5000;
      bool isRunning_ = false;
      bool startFailed_ = false;
      bool waitForProcess_ = false;
      DWORD exitCode_ = (DWORD)-1;

      std::string appName_;
      std::filesystem::path startFolder_;
      std::filesystem::path exeFilename_;
      std::vector<std::string> args_;
      
      std::string cmdLine_;
      WindowsPipePtr pipeStderr_;
      WindowsPipePtr pipeStdout_;
      WindowsPipePtr pipeStdin_;
      PROCESS_INFORMATION procInfo_;
      STARTUPINFO startInfo_;
      WindowsHandlePtr proc_;
      WindowsHandlePtr thread_;
   };

   typedef std::shared_ptr<WindowsProcess> WindowsProcessPtr;

} // namespace mikado::windowsApi

#endif // API_WINDOWSPROCESS_H
