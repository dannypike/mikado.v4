// Copyright (c) 2024 Gamaliel Ltd

#include "common/algorithms.h"
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
   string appIdMakeMore{ "makeMore" };
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
   string const kCudaDevice{ "cuda-device" };
   string const kDefault{ "default" };
   string const kDevice{ "device" };
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
   string const kTest{ "test" };
   string const kWindowBroker { "Mikado Broker" };

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

#ifdef _WIN32

///////////////////////////////////////////////////////////////////////
//
int parseCommandline(wstring const &cmdLine, vector<string> &args
   , vector<char *> *asArgv) {

   int argc;
   LPWSTR *wargv = CommandLineToArgvW(cmdLine.c_str(), &argc);
   for (auto ii = 0; ii < argc; ++ii) {
      args.emplace_back(mikado::common::toString(wargv[ii]));
      if (asArgv) {
         asArgv->push_back(&args.back()[0]);
      }
   }
   LocalFree(wargv);
   return argc;
}

//////////////////////////////////////////////////////////////////////////
// For GUI apps, we need a WinMain
//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance
   , LPSTR lpCmdLine, int nCmdShow)
{
   // Convert the lpCmdline into argv & argv
   vector<string> args;
   vector<char *> argv;
   wstring cmdline{ move(mikado::common::toString(lpCmdLine)) };
   int argc = parseCommandline(cmdline, args, &argv);

   // And then call the normal main() function
   extern int main(int argc, char *argv[]);
   return main(argc, &argv[0]);
}

#endif // _WIN32
