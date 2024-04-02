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

   // String constants
   string const kAppId{ "appId" };
   string const kArgs{ "args" };
   string const kBrokerHost{ "broker-host" };
   string const kBrokerPort{ "broker-port" };
   string const kBrokerTimeout{ "broker-timeout" };
   string const kConsoleQuiet{ "console-quiet" };
   string const kConsoleRestoreOnExit{ "console-restore-on-exit" };
   string const kConsoleTitle{ "console-title" };
   string const kDefault{ "default" };
   string const kExclude{ "exclude" };
   string const kExePath{ "exePath" };
   string const kHelp{ "help" };
   string const kInclude{ "include" };
   string const kInstanceId{ "instanceId" };
   string const kRoot{ "root" };
   string const kStartApp{ "start-app" };
   string const kStartComspec{ "start-comspec" };
   string const kStartFolder{ "start-folder" };
   string const kStartRetry{ "start-retry" };

   // fields are used in websocket messages
   string fieldAction{ "action" };
   string fieldAppId{ "appid" };
   string fieldInstanceId{ "instanceid" };
   string fieldTimestamp{ "timestamp" };
   string fieldType{ "field" };
   string fieldVersion{ "version" };

   // Actions are used to identify the type of message in websocket messages
   string actionConnect{ "connect" };

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
