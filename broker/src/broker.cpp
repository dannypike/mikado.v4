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
   bool IsRunning = true;

   //////////////////////////////////////////////////////////////////////////
   //
   BOOL WINAPI consoleCtrlHandler(DWORD ctrlType) {
      switch (ctrlType) {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
      case CTRL_CLOSE_EVENT:
         IsRunning = false;
         return TRUE;
      default:
         return FALSE;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "Broker version " << BROKER_VERSION_MAJOR << "." << BROKER_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode showHelp(po::options_description const &options) {
      common::MikadoLog::MikadoLogger.setOutputODS(false);
      common::MikadoLog::MikadoLogger.setOutputStdout(true);
      outputBanner();
      str_notice() << "Usage: broker [options]" << endl << endl
         << "Where [options] are:" << endl;
      cout << options << endl
         << "Hit Ctrl-C to stop" << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode main(int argc, char *argv[]) {

      po::options_description options;
      options.add_options()
         ("interface", po::value<string>(), "interface to listen on (default: 127.0.0.1)")
         ("port", po::value<int>(), "port to listen on (default: 22304)")
         ("help", "produce help message")
         ;

      po::variables_map args;
      try
      {
         store(po::command_line_parser(argc, argv).
            options(options).run(), args);
         notify(args);
      }
      catch (const std::exception &e)
      {
         str_error() << e.what() << endl;
         return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
      }

      // If anything asks for help, that's the only thing we do
      if (args.count("help")) {
         return showHelp(options);
      }

      if (!ix::initNetSystem()) {
         outputBanner();
         str_error() << "Failed to initialize the websocket API" << endl;
         return MikadoErrorCode::MKO_ERROR_WEBSOCKET;
      }

      // Don't output anything to the console. We do this now just in case there were
      // any errors in the configuration.
      common::MikadoLog::MikadoLogger.setOutputStdout(false);
      outputBanner();

      // Stop the program on Ctrl-C
      auto exitCode = MikadoErrorCode::MKO_ERROR_DID_NOT_RUN;
      SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

      // Start the WebSocket server
      HandlerPtr server = make_shared<Handler>();
      if (auto rc = server->configure(args); MKO_IS_ERROR(rc)) {
         return rc;
      }
      if (auto rc = server->initialize(); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // Dummy loop until we get the system up and running
      exitCode = MikadoErrorCode::MKO_ERROR_NONE;
      while (IsRunning) {
         this_thread::sleep_for(100ms);
      }

      str_info() << "shutting down" << endl;
      server->shutdown();
      if (!ix::uninitNetSystem()) {
         str_error() << "Failed to shut down the websocket API" << endl;
         if (!MKO_IS_ERROR(exitCode)) { 
            exitCode = MikadoErrorCode::MKO_ERROR_WEBSOCKET;
         }
      }

      str_info() << "exiting with code " << (int)exitCode << endl;
      return exitCode;
   }
    
} // namespace mikado::broker

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
    if (auto rc = common::commonInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }
    if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }

    int exitCode = (int)mikado::broker::main(argc, argv);
    assert(STATUS_PENDING != exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
    windowsApi::apiShutdown();
    common::commonShutdown();
    return exitCode;
}
