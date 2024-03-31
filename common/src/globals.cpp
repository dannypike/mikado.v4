// Copyright (c) 2024 Gamaliel Ltd
#include "common/globals.h"

using namespace std;
using namespace filesystem;

namespace mikado::common {

   path MikadoExeFolder;
   bool MikadoShutdownRequested = false;

   // AppIds are used to identify the sending application in websocket messages
   string appIdBroker { "broker" };
   string appIdGlobber{ "globber" };
   string appIdStorage{ "storage" };

   // Actions are used to identify the type of message in websocket messages
   string actionConnect{ "connect" };

   // fields are used in websocket messages
   extern std::string fieldAction{ "action" };
   extern std::string fieldAppId{ "appid" };
   extern std::string fieldTimestamp{ "timestamp" };
   extern std::string fieldType{ "field" };
   extern std::string fieldVersion{ "version" };

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
