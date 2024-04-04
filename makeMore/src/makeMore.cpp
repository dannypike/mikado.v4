// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "windowsApi.h"
#include "makeMore.h"

namespace api = mikado::windowsApi;
namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

namespace mikado::makeMore {
   static MikadoErrorCode main(int argc, char *argv[]);

   //////////////////////////////////////////////////////////////////////////
   //
   BOOL WINAPI consoleCtrlHandler(DWORD ctrlType) {
      switch (ctrlType) {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
      case CTRL_CLOSE_EVENT:
         return TRUE;
      default:
         return FALSE;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static void outputBanner() {
      str_notice() << "MakeMore version " << MAKEMORE_VERSION_MAJOR << "." << MAKEMORE_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode MakeMore::start() {
      return MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode MakeMore::stop() {
      return MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   void MakeMore::onBrokerMessage(common::WebSocketPtr broker
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

      auto makeMore = make_shared<MakeMore>();
      auto options = make_shared<common::Configure>(common::appIdMakeMore, "Mikado Makemore MLP", &*makeMore);

      // There is a typical sequence of processing options, that we do for all of the applications
      auto rc = options->defaultProcessing(argc, argv, outputBanner);
      if (MikadoErrorCode::MKO_ERROR_NONE != rc) { // May be an MKO_STATUS, so we don't use MKO_IS_ERROR() here
         // Already output a message
         return rc;
      }

      // Set up Ctrl-C/break handler, suppress stdout debug-logging and display the banner
      windowsApi::apiSetupConsole(options, outputBanner);

      auto exitCode = MikadoErrorCode::MKO_ERROR_DID_NOT_RUN;
      makeMore->start();

      str_info() << "shutting down" << endl;
      makeMore->stop();

      return exitCode;
   }
    
} // namespace mikado::makeMore

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
    if (auto rc = common::commonInitialize(argc, argv, mikado::makeMore::outputBanner); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }
    if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
        return (int)rc;
    }

    auto exitCode = (int)mikado::makeMore::main(argc, argv);
    assert(STATUS_PENDING != (int)exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
    windowsApi::apiShutdown();
    common::commonShutdown();

    str_info() << "exiting with code " << exitCode << endl;
    return (int)exitCode;
}
