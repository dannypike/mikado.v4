#include "common/errorCodes.h"
#include "common/globals.h"
#include "common/logger.h"
#include "windowsApi/windowsGlobals.h"
#include "globber.h"

namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

namespace mikado::globber {

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "Globber version " << GLOBBER_VERSION_MAJOR << "." << GLOBBER_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode main(int argc, char *argv[]) {

      common::MikadoLog::MikadoLogger.setOutputStdout(false);
      outputBanner();

      po::options_description options;
      options.add_options()
         ("include", po::value<vector<string>>()->multitoken(), "include a file or folder (regex)")
         ("exclude", po::value<vector<string>>()->multitoken(), "exclude a file or folder (regex)")
         ("default", po::value<string>(), "default action (include|exclude)")
         ("help", "produce help message")
         ;

      po::variables_map args;
      store(po::command_line_parser(argc, argv).
         options(options).run(), args);
      notify(args);

      // If anything asks for help, that's the only thing we do
      if (args.count("help")) {
         common::MikadoLog::MikadoLogger.setOutputODS(false);
         common::MikadoLog::MikadoLogger.setOutputStdout(true);
         outputBanner();
         str_notice() << "Usage: globber [options]" << endl << endl
            << "Where [options] are:" << endl;
         cout << options;
         return MikadoErrorCode::MKO_ERROR_NONE;
      }


      return MikadoErrorCode::MKO_ERROR_NONE;
   }
    
} // namespace mikado::globber

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
    if (auto rc = common::commonInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }
    if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }

    int exitCode = (int)mikado::globber::main(argc, argv);
    assert(STATUS_PENDING != exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
    windowsApi::apiShutdown();
    common::commonShutdown();
    return exitCode;
}
