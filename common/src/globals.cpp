// Copyright (c) 2024 Gamaliel Ltd
#include "common/errorCodes.h"
#include "common/globals.h"
#include "common/logger.h"

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

   MikadoErrorCode commonInitialize(int argc, char *argv[], function<void()> outputBanner) {
      if (!ix::initNetSystem()) {
         if (outputBanner) {
            outputBanner();
         }
         str_error() << "Failed to initialize the websocket API" << endl;
         return MikadoErrorCode::MKO_ERROR_WEBSOCKET;
      }

      if (argc < 1) {
         return MikadoErrorCode::MKO_ERROR_INVALID_ARGUMENTS;
      }
      MikadoExeFolder = path(argv[0]).parent_path();
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   void commonShutdown() {
      if (!ix::uninitNetSystem()) {
         str_error() << "Failed to shut down the websocket API" << endl;
      }
   }

} // namespace mikado::common
