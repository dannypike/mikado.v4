// Copyright (c) 2024 Gamaliel Ltd
#include "common/globals.h"

using namespace std;
using namespace filesystem;

namespace mikado::common {

   path MikadoExeFolder;

   MikadoErrorCode commonInitialize(int argc, char *argv[]) {
      if (argc < 1) {
         return MikadoErrorCode::MKO_ERROR_INVALID_ARGUMENTS;
      }
      MikadoExeFolder = path(argv[0]).parent_path();
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   void commonShutdown() {
   }

} // namespace mikado::common
