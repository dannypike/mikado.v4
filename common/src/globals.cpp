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

   // Strings used in boost::program_options descriptions
   string const poAppStart{ "app-start" };
   string const poBrokerHost{ "broker-host" };
   string const poBrokerPort{ "broker-port" };
   string const poBrokerTimeout{ "broker-timeout" };
   string const poConsoleRestoreOnExit{ "console-restore-on-exit" };
   string const poConsoleQuiet{ "console-quiet" };
   string const poConsoleTitle{ "console-title" };
   string const poDefault{ "default" };
   string const poExclude{ "exclude" };
   string const poHelp{ "help" };
   string const poInclude{ "include" };
   string const poRoot{ "root" };

   // Strings used in the configuration files
   string const cfgAppId{ "appId" };
   string const cfgArgs{ "args" };
   string const cfgStartFolder{ "startFolder" };
   string const cfgExePath{ "exePath" };
   string const cfgPath{ "path" };

   // fields are used in websocket messages
   extern std::string fieldAction{ "action" };
   extern std::string fieldAppId{ "appid" };
   extern std::string fieldTimestamp{ "timestamp" };
   extern std::string fieldType{ "field" };
   extern std::string fieldVersion{ "version" };

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
