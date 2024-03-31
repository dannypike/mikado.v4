#include "common.h"
#include "windowsApi.h"
#include "broker.h"

namespace windowsApi = mikado::windowsApi;
namespace common = mikado::common;
namespace po = boost::program_options;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

namespace mikado::broker {
   static MikadoErrorCode main(int argc, char *argv[]);

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "Broker version " << BROKER_VERSION_MAJOR << "." << BROKER_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode main(int argc, char *argv[]) {

      auto options = make_shared<common::Configure>(common::appIdBroker, "Mikado Broker", nullptr);
      options->addOptions()
         ;

      // There is a typical sequence of processing options, that we do for all of the applications
      auto rc = options->defaultProcessing(argc, argv, outputBanner);
      if (MikadoErrorCode::MKO_ERROR_NONE != rc) { // May be an MKO_STATUS, so we don't use MKO_IS_ERROR() here
         // Already output a message
         return rc;
      }

      // Start the WebSocket handler
      HandlerPtr handler = make_shared<Handler>();
      if (auto rc = handler->configureHandler(options); MKO_IS_ERROR(rc)) {
         return rc;
      }
      if (auto rc = handler->initializeHandler(); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // Set up Ctrl-C/break handler, suppress stdout debug-logging and display the banner
      windowsApi::apiSetupConsole(options, outputBanner);

      str_notice() << "Broker is listening for connections on " << handler->getUrl() << endl;

      // Test the startup protocol (normally disabled)
      bool runTestConnector = false;
      if (runTestConnector) {
         common::testConnect(options);
      }
      
      // Dummy loop until we get the system up and running
      auto exitCode = MikadoErrorCode::MKO_ERROR_NONE;
      while (!common::MikadoShutdownRequested) {
         if (runTestConnector && common::testProcess()) {
            continue;
         }

         this_thread::sleep_for(100ms);
      }

      str_notice() << "Broker is shutting down" << endl;
      if (runTestConnector) {
         common::testShutdown();
      }
      handler->shutdown();

      return exitCode;
   }
    
} // namespace mikado::broker

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
   auto rc = common::commonInitialize(argc, argv, mikado::broker::outputBanner);
   if (MKO_IS_ERROR(rc)) {
      return (int)rc;
   }
   if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
      return (int)rc;
   }

   int exitCode = (int)mikado::broker::main(argc, argv);
   assert(STATUS_PENDING != exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
   windowsApi::apiShutdown();
   common::commonShutdown();

   str_info() << "exiting with code " << exitCode << endl;
   return exitCode;
}
