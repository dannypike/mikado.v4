// Copyright (c) 2024 Gamaliel Ltd
#include "common/globals.h"

using namespace std;
using namespace filesystem;

namespace mikado::common {

   path MikadoExeFolder;
   bool MikadoShutdownRequested = false;

   // The appIds are used to identify each application over the web sockets and elsewhere   
   string appIdBroker { "broker" };
   string appIdGlobber{ "globber" };
   string appIdStorage{ "storage" };

   MikadoErrorCode commonInitialize(int argc, char *argv[]) {
      if (argc < 1) {
         return MikadoErrorCode::MKO_ERROR_INVALID_ARGUMENTS;
      }
      MikadoExeFolder = path(argv[0]).parent_path();
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   void commonShutdown() {
   }

   ostream &operator<<(ostream &os, enum MikadoErrorCode code) {
      return os << static_cast<int>(code);
   }

} // namespace mikado::common
