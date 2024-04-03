// Copyright (c) 2024 Gamaliel Ltd

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

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "Globber version " << GLOBBER_VERSION_MAJOR << "." << GLOBBER_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode configureMonitor(common::ConfigurePtr cfg
         , windowsApi::WindowsFileMonitorPtr monitor) {

      // The default action if it passes all of the filters
      string defaultAction;
      if (cfg->tryGet(common::kDefault, defaultAction)) {
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
      vector<string> globs;
      if (cfg->tryGet(common::kInclude, globs)) {
         for (auto &include : globs) {
            if (auto rc = monitor->includeGlob(include); MKO_IS_ERROR(rc)) {
               return rc;
            }
         }
      }
      if (cfg->tryGet(common::kExclude, globs)) {
         for (auto &exclude : globs) {
            if (auto rc = monitor->excludeGlob(exclude); MKO_IS_ERROR(rc)) {
               return rc;
            }
         }
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   void Globber::onBrokerMessage(common::WebSocketPtr broker
         , ix::WebSocketMessagePtr const &msg) {

      broker_ = broker;
      switch (msg->type) {
      case ix::WebSocketMessageType::Message:
         str_info() << "Broker message: " << msg->str << endl;
         break;

      case ix::WebSocketMessageType::Open:
         str_info() << "Broker connection opened" << endl;
         break;

      case ix::WebSocketMessageType::Close:
         str_info() << "Broker connection closed" << endl;
         break;

      case ix::WebSocketMessageType::Error:
         str_error() << "Broker connection error: " << msg->errorInfo.reason << endl;
         break;

      default:
         str_error() << "Broker connection unknown message type" << endl;
         break;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static MikadoErrorCode main(int argc, char *argv[]) {
      
      Globber globber;
      auto options = make_shared<common::Configure>(common::appIdGlobber, "Mikado Globber", &globber);
      options->addOptions()
         ("root", po::value<path>()->multitoken(), "the root directory to monitor")
         ("include", po::value<vector<string>>()->multitoken(), "include a file or subfolder (regex)")
         ("exclude", po::value<vector<string>>()->multitoken(), "exclude a file or subfolder (regex)")
         ("default", po::value<string>(), "default action (include|exclude)")
         ;

      // There is a typical sequence of processing options, that we do for all of the applications
      auto rc = options->defaultProcessing(argc, argv, outputBanner, false);
      if (MikadoErrorCode::MKO_ERROR_NONE != rc) { // May be an MKO_STATUS, so we don't use MKO_IS_ERROR() here
         // Already output a message
         return rc;
      }

      // Use a single lock-free queue to receive updates from all of the monitors
      // (in the first version, we only have a single monitor)
      auto updateQueue = make_shared<api::WindowsFileMonitor::UpdateQueue>();
      auto monitor = make_shared<windowsApi::WindowsFileMonitor>(updateQueue);
      FileMonitors.push_back(monitor); 
      if (auto rc = configureMonitor(options, monitor); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // Get the root directory of the filtering
      path rootFolder(path(argv[0]).parent_path());
      rootFolder = options->get<path>("root", rootFolder);

      // Set up Ctrl-C/break handler, suppress stdout debug-logging and display the banner
      windowsApi::apiSetupConsole(options, outputBanner);

      // Run the monitor thread
      auto exitCode = MikadoErrorCode::MKO_ERROR_DID_NOT_RUN;
      jthread monitorThread([](windowsApi::WindowsFileMonitorPtr monitor, path rootFolder, MikadoErrorCode *exitCode){
         *exitCode = monitor->run(rootFolder);
         }, monitor, rootFolder, &exitCode);

      // Connect to the broker
      auto broker = make_shared<common::BrokerConnection>();
      if (auto rc = broker->connect(options); MKO_IS_ERROR(rc)) {
         return rc;
      }

      // Wait for the monitor to detect changes to files and write them to cout
      while (!common::MikadoShutdownRequested) {
         if (!updateQueue->consume_all(
            [](api::WindowsFileMonitor::QueueDataPtr queueData) {
               cout << queueData->json << endl;
               })) {
            this_thread::sleep_for(10ms);

            // Push any outgoing messages to the broker
            broker->processOutgoing();
         }
      }

      str_info() << "shutting down" << endl;
      broker->shutdown();
      for (auto mm : FileMonitors) {
         mm->stopMonitoring();
      }
      monitorThread.join();

      str_info() << "exiting with code " << exitCode << endl;
      return exitCode;
   }
    
} // namespace mikado::globber

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {

   if (auto rc = common::commonInitialize(argc, argv, mikado::globber::outputBanner); MKO_IS_ERROR(rc)) {
      return (int)rc;
   }
   if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
      return (int)rc;
   }

   auto exitCode = mikado::globber::main(argc, argv);
   assert(STATUS_PENDING != (int)exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
   windowsApi::apiShutdown();
   common::commonShutdown();

   str_info() << "exiting with code " << exitCode << endl;
   return (int)exitCode;
}
