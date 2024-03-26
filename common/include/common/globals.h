#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_GLOBALS_H)
#define CMN_GLOBALS_H

#include "errorCodes.h"

namespace mikado::common {
   extern std::filesystem::path MikadoExeFolder;

   MikadoErrorCode commonInitialize(int argc, char *argv[]);
   void commonShutdown();
} // namespace mikado::common

#endif // CMN_GLOBALS_H
