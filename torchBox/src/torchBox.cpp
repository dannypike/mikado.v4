// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "windowsApi.h"
#include "torchBox.h"
#include "torchBox/netMakeMore.h"
#include "torchBox/netMulMat.h"

namespace api = mikado::windowsApi;
namespace bt = boost::posix_time;
namespace common = mikado::common;
namespace po = boost::program_options;
namespace windowsApi = mikado::windowsApi;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;
using namespace std::filesystem;

///////////////////////////////////////////////////////////////////////
// Force the linker to import mimalloc before anything else, so that it
// can be used as the default allocator
bool _forceMimalloc = false;
void forceMimalloc() {
   _forceMimalloc = mi_option_is_enabled(mi_option_eager_commit);
}

///////////////////////////////////////////////////////////////////////
//
namespace mikado::torchBox {
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
   void TorchBox::outputBanner() {
      str_notice() << "TorchBox version " << TORCHBOX_VERSION_MAJOR << "." << TORCHBOX_VERSION_MINOR
         << " (" << __DATE__ << " " << __TIME__ << ")" << endl;
      str_notice() << "Copyright (c) 2024 Gamaliel Ltd" << endl << endl;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   common::MikadoErrorCode TorchBox::configure(common::ConfigurePtr cfg) {
      cfg_ = cfg;

      string defaultName { enum_hpp::to_string(c10Device_).value_or("cpu") };
      string deviceName = cfg->get<string>(common::kDevice, defaultName);
      string upperName(deviceName);
      boost::to_upper(upperName);
      c10Device_ = enum_hpp::from_string<c10::DeviceType>(upperName).value_or(c10::DeviceType::CPU);
      torchDevice_ = (torch::TensorOptions)c10Device_;   // They are the same values

      netNames_ = cfg->get<vector<string>>(common::kModel);

      if (c10Device_ == c10::DeviceType::CUDA) {
         if (!torch::cuda::is_available()) {
            torchDevice_ = (torch::TensorOptions) (c10Device_ = c10::DeviceType::CPU);
            str_error() << deviceName << " is not available. Falling back to '"
               << enum_hpp::to_string(c10Device_).value_or("cpu") << "'";
            return MikadoErrorCode::MKO_STATUS_FALLBACK_TO_DEFAULT;
         }
      }
      str_info() << "using '" << deviceName << "'" << endl;

      auto rc = MikadoErrorCode::MKO_STATUS_NOOP;
      for (auto netName : netNames_) {
         shared_ptr<NetBase> network;
         if (boost::iequals(netName, common::kMulMat)) {
            network = make_shared<NetMulMat>()->shared_from_this();
         }
         else if (boost::iequals(netName, common::kMakeMore)) {
            network = make_shared<NetMakeMore>();
         }
         else {
            str_error() << "unrecognized network name: '" << netName
               << "' in the configuration" << endl;
            continue;
         }
         network->setConfig(cfg_);
         network->setC10Device(c10Device_);
         network->setTorchDevice(torchDevice_);

         if (rc = network->configure(cfg); MKO_IS_ERROR(rc)) {
            return rc;
         }
         networks_.insert(make_pair(move(netName), network));
         rc = MikadoErrorCode::MKO_ERROR_NONE;
      }
      if (networks_.empty()) {
         str_error() << "no networks were configured" << endl;
         return MikadoErrorCode::MKO_ERROR_NO_NETWORKS;
      }
      return rc;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode TorchBox::verify() {

      for (auto pp : networks_) {
         if (auto rc = pp.second->verify(); MKO_IS_ERROR(rc)) {
            return rc;
         }
      }
      return MikadoErrorCode::MKO_STATUS_NOOP;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode TorchBox::train() {

      for (auto pp : networks_) {
         if (auto rc = pp.second->train(); MKO_IS_ERROR(rc)) {
            return rc;
         }
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   void TorchBox::onBrokerMessage(common::WebSocketPtr broker
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

      auto torchBox = make_shared<TorchBox>();
      auto options = make_shared<common::Configure>(common::appIdTorchBox, "Mikado TorchBox", &*torchBox);
      options->addOptions()
         (common::kDevice.c_str(), po::value<string>(), "case-sensitive C10 device name, e.g. 'CPU', 'CUDA' (default), ...")
         (common::kModel.c_str(), po::value<vector<string>>(), "name of a model to run, e.g. 'MulMat', 'MakeMore'")
         ;

      // Add the network options
      NetMakeMore::addOptions(options);

      // There is a typical sequence of processing options, that we do for all of the applications
      auto rc = options->defaultProcessing(argc, argv, TorchBox::outputBanner);
      if (MikadoErrorCode::MKO_ERROR_MAXSTATUS < rc) { // May be an MKO_STATUS, so we don't use MKO_IS_ERROR() here
         // Already output a message
         return rc;
      }

      // Set up Ctrl-C/break handler, suppress stdout debug-logging and display the banner
      windowsApi::apiSetupConsole(options, TorchBox::outputBanner);

      str_info() << "configuration stage" << endl;
      if (rc = torchBox->configure(options); MKO_IS_ERROR(rc)) {
         str_error() << "failed to configure TorchBox" << endl;
         return rc;
      }
      else if (MikadoErrorCode::MKO_ERROR_NONE != rc) {
         str_error() << "no networks were configured" << endl;
         return rc;
      }

      str_info() << "training stage" << endl;
      if (rc = torchBox->train(); MKO_IS_ERROR(rc)) {
         str_error() << "failed to train TorchBox" << endl;
         return rc;
      }

      str_info() << "verification stage" << endl;
      if (rc = torchBox->verify(); MKO_IS_ERROR(rc)) {
         str_error() << "failed to verify TorchBox" << endl;
         return rc;
      }

      str_info() << "verified all enabled networks" << endl;
      return MikadoErrorCode::MKO_ERROR_NONE;
   }
    
} // namespace mikado::torchBox

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
   if (auto rc = common::commonInitialize(argc, argv, mikado::torchBox::TorchBox::outputBanner); MKO_IS_ERROR(rc)) {
      return (int)rc;
   }
   if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
      return (int)rc;
   }

   auto exitCode = (int)mikado::torchBox::main(argc, argv);
   assert(STATUS_PENDING != (int)exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.
    
   windowsApi::apiShutdown();
   common::commonShutdown();

   str_info() << "exiting with code " << (MikadoErrorCode)exitCode << endl;
   return (int)exitCode;
}
