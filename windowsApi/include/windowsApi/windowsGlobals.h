#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(API_WINDOWS_GLOBALS_H)
#define API_WINDOWS_GLOBALS_H

namespace mikado::windowsApi {
   
   std::string getLastErrorAsString(DWORD *lastError = nullptr);
   mikado::common::MikadoErrorCode apiInitialize(int argc, char *argv[]);
   void apiShutdown();

} // namespace mikado::windowsApi

#endif // API_WINDOWS_GLOBALS_H
