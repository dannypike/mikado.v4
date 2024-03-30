#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(API_WINDOWS_GLOBALS_H)
#define API_WINDOWS_GLOBALS_H

namespace mikado::windowsApi {
   typedef mikado::common::MikadoErrorCode MikadoErrorCode;

   std::string getLastErrorAsString(DWORD *lastError = nullptr);
   MikadoErrorCode apiInitialize(int argc, char *argv[]);
   MikadoErrorCode apiSetupConsole(std::string const &consoleTitle
      , bool restoreTitle, std::function<void()> showBanner);
   void apiShutdown();

} // namespace mikado::windowsApi

#endif // API_WINDOWS_GLOBALS_H
