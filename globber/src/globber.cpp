#include "common.h"
#include "windowsApi.h"
#include "globber.h"

namespace api = mikado::windowsApi;
namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

namespace mikado::globber {
   static MikadoErrorCode main(int argc, char *argv[]);
   static vector<api::WindowsFileMonitorPtr> FileMonitors;
   static bool IsRunning = true;

   //////////////////////////////////////////////////////////////////////////
   //
   BOOL WINAPI consoleCtrlHandler(DWORD ctrlType) {
      switch (ctrlType) {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
      case CTRL_CLOSE_EVENT:
         for (auto mm : FileMonitors) {
            mm->stopMonitoring();
         }
         IsRunning = false;
         return TRUE;
      default:
         return FALSE;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "Globber version " << GLOBBER_VERSION_MAJOR << "." << GLOBBER_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode showHelp(po::options_description const &options) {
      common::MikadoLog::MikadoLogger.setOutputODS(false);
      common::MikadoLog::MikadoLogger.setOutputStdout(true);
      outputBanner();
      str_notice() << "Usage: globber [options]" << endl << endl
         << "Where [options] are:" << endl;
      cout << options << endl
         << "Hit Ctrl-C to stop" << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode configureMonitor(po::variables_map const &args
         , windowsApi::WindowsFileMonitorPtr monitor) {

      // The default action if it passes all of the filters
      if (args.count("default")) {
         auto defaultAction = args["default"].as<string>();
         if (boost::iequals(defaultAction, "include")) {
            monitor->setDefaultAction(windowsApi::WindowsFileMonitor::Action::Include);
         }
         else if (boost::iequals(defaultAction, "exclude")) {
            monitor->setDefaultAction(windowsApi::WindowsFileMonitor::Action::Exclude);
         }
         else {
            str_error() << "Invalid default action: " << defaultAction << endl;
            return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
         }
      }

      // The various filters
      if (args.count("include")) {
         for (auto &include : args["include"].as<vector<string>>()) {
            if (auto rc = monitor->includeGlob(include); MKO_IS_ERROR(rc)) {
               return rc;
            }
         }
      }
      if (args.count("exclude")) {
         for (auto &exclude : args["exclude"].as<vector<string>>()) {
            if (auto rc = monitor->excludeGlob(exclude); MKO_IS_ERROR(rc)) {
               return rc;
            }
         }
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode main(int argc, char *argv[]) {

      po::options_description options;
      options.add_options()
         ("root", po::value<string>()->multitoken(), "the root directory to monitor")
         ("include", po::value<vector<string>>()->multitoken(), "include a file or subfolder (regex)")
         ("exclude", po::value<vector<string>>()->multitoken(), "exclude a file or subfolder (regex)")
         ("default", po::value<string>(), "default action (include|exclude)")
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

      // Use a single lock-free queue to receive updates from all of the monitors
      // (in the first version, we only have a single monitor)
      auto updateQueue = make_shared<api::WindowsFileMonitor::UpdateQueue>();
      auto monitor = make_shared<windowsApi::WindowsFileMonitor>(updateQueue);
      FileMonitors.push_back(monitor); 
      if (auto rc = configureMonitor(args, monitor); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // Get the root directory of the filtering
      path rootFolder(path(argv[0]).parent_path());
      rootFolder = common::poGetString(args, "root", rootFolder.string().c_str());

      // Don't output anything to the console. We do this now just in case there were
      // any errors in the configuration.
      common::MikadoLog::MikadoLogger.setOutputStdout(false);
      outputBanner();

      // Stop the program on Ctrl-C
      auto exitCode = MikadoErrorCode::MKO_ERROR_DID_NOT_RUN;
      SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

      // Run the monitor thread
      jthread monitorThread([](windowsApi::WindowsFileMonitorPtr monitor, path rootFolder, MikadoErrorCode *exitCode){
         *exitCode = monitor->run(rootFolder);
         }, monitor, rootFolder, &exitCode);

      // Wait for the monitor to detect changes to files and write them to cout
      while (IsRunning) {
         if (!updateQueue->consume_one([](api::WindowsFileMonitor::QueueDataPtr queueData) {
               cout << queueData->json << endl;
               })) {
            this_thread::sleep_for(10ms);
         }
      }

      str_info() << "shutting down" << endl;
      monitorThread.join();

      str_info() << "exiting with code " << exitCode << endl;
      return exitCode;
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
