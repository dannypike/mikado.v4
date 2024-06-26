// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "windowsApi.h"
#include "broker.h"

namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
using namespace std;
using namespace std::filesystem;

namespace mikado::broker {
   typedef common::MikadoErrorCode MikadoErrorCode;

   //////////////////////////////////////////////////////////////////////////
   //
   void outputBanner() {
      str_notice() << "Broker version " << BROKER_VERSION_MAJOR << "." << BROKER_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   ostream &operator<<(ostream &os, broker::AppId const &appId) {
      return os << appId.toString();
   }

   //////////////////////////////////////////////////////////////////////////
   //
   ostream &operator<<(ostream &os, broker::AppInstanceId const &appInstanceId) {
      return os << appInstanceId.toString();
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode main(int argc, char *argv[]) {

      auto options = make_shared<common::Configure>(common::appIdBroker, "Mikado Broker", nullptr);
      options->addOptions()
         (common::kStartApp.c_str(), po::value<vector<string>>(), "Application(s) that the broker should start")
         (common::kStartComspec.c_str(), po::value<vector<string>>(), "Arguments for Command Prompt, if not running as a standalone app")
         (common::kStartRetry.c_str(), po::value<int>(), "Delay time (ms) between successive attempts to restart the app")
         ;

      // There is a typical sequence of processing options, that we do for all of the applications
      auto rc = options->defaultProcessing(argc, argv, outputBanner);
      if (MikadoErrorCode::MKO_ERROR_NONE != rc) { // May be an MKO_STATUS, so we don't use MKO_IS_ERROR() here
         // Already output a message
         return rc;
      }

      // Start the WebSocket server
      HandlerPtr handler = make_shared<Handler>();
      if (auto rc = handler->configureHandler(options); MKO_IS_ERROR(rc)) {
         return rc;
      }
      if (auto rc = handler->initializeHandler(); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // The AppManager starts applications
      auto appManager = make_shared<AppManager>();
      if (auto rc = appManager->configureAppManager(options); MKO_IS_ERROR(rc)) {
         return rc;
      }      

      // Set up Ctrl-C/break handler, suppress stdout debug-logging and display the banner
      windowsApi::apiSetupConsole(options, outputBanner);

      str_notice() << "Broker is listening for connections on " << handler->getUrl() << endl;

      // Create the GUI
      auto gui = make_shared<Gui>(options);

      // "Event" loop
      auto exitCode = MikadoErrorCode::MKO_ERROR_NONE;
      while (!common::MikadoShutdownRequested) {
         // Should we start any apps?
         if (auto rc = appManager->runAppManager(options); MKO_IS_ERROR(rc)) {
            exitCode = rc;
            break;
         }

         bool workDone = false;
         if (!gui->processWin32Messages(workDone)) {
            exitCode = MikadoErrorCode::MKO_STATUS_EXITOK;
            break;
         }

         this_thread::sleep_for(100ms);
      }

      str_notice() << "Broker is shutting down with exit code " << exitCode << endl;
      handler->shutdown();

      return exitCode;
   }

} // namespace mikado::broker
