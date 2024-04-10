#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(API_WINDOWS_GLOBALS_H)
#define API_WINDOWS_GLOBALS_H

namespace mikado::common {
   typedef std::shared_ptr<class Configure> ConfigurePtr;
} // namespace common

namespace mikado::windowsApi {
   typedef mikado::common::MikadoErrorCode MikadoErrorCode;

   std::string getLastErrorAsString(DWORD *lastError = nullptr);
   MikadoErrorCode apiInitialize(int argc, char *argv[]);
   MikadoErrorCode apiSetupConsole(common::ConfigurePtr options, std::function<void()> showBanner);
   void apiShutdown();
   extern std::mutex DbgHelpMux;

} // namespace mikado::windowsApi

#endif // API_WINDOWS_GLOBALS_H
