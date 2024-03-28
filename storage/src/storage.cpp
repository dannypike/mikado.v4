#include "common/algorithms.h"
#include "common/errorCodes.h"
#include "common/globals.h"
#include "common/logger.h"
#include "windowsApi/windowsFileMonitor.h"
#include "windowsApi/windowsGlobals.h"
#include "storage.h"

namespace api = mikado::windowsApi;
namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

namespace mikado::storage {
   static MikadoErrorCode main(int argc, char *argv[]);

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "Storage version " << STORAGE_VERSION_MAJOR << "." << STORAGE_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode showHelp(po::options_description const &options) {
      common::MikadoLog::MikadoLogger.setOutputODS(false);
      common::MikadoLog::MikadoLogger.setOutputStdout(true);
      outputBanner();
      str_notice() << "Usage: storage [options]" << endl << endl
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
         //("default", po::value<string>(), "default action (include|exclude)")
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

      // Don't output anything to the console. We do this now just in case there were
      // any errors in the configuration.
      common::MikadoLog::MikadoLogger.setOutputStdout(false);
      outputBanner();

      auto exitCode = MikadoErrorCode::MKO_STATUS_NOOP;

      str_info() << "shutting down" << endl;

      str_info() << "exiting with code " << (int)exitCode << endl;
      return exitCode;
   }
    
} // namespace mikado::storage

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
    if (auto rc = common::commonInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }
    if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }

    int exitCode = (int)mikado::storage::main(argc, argv);
    assert(STATUS_PENDING != exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
    windowsApi::apiShutdown();
    common::commonShutdown();
    return exitCode;
}
