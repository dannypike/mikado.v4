// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"

namespace ba = boost::algorithm;
namespace json = boost::json;
namespace po = boost::program_options;
using namespace std;
using namespace std::filesystem;

namespace mikado::common {

   ///////////////////////////////////////////////////////////////////////////
   //
   Configure::Configure(string const &appId, string const &consoleTitle, BrokerFYI *fyi)
      : appId_ { appId }
      , options_{ make_shared<po::options_description>() }
      , values_{ make_shared<po::variables_map>() } {

      // Add the options that are defined for all Mikado apps
      addOptions()
         (common::kAppId.c_str(), po::value<string>(), "the type of the application (broker, globber, storage etc)")
         (common::kBrokerHost.c_str(), po::value<string>(), "broker host")
         (common::kBrokerPort.c_str(), po::value<int>(), "broker port")
         (common::kBrokerTimeout.c_str(), po::value<unsigned>()->default_value(2), "broker timeout in seconds")
         (common::kConsoleQuiet.c_str(), po::value<bool>()->default_value(true), "suppress output to console")
         (common::kConsoleRestoreOnExit.c_str(), po::value<bool>()->default_value(false), "save and restore the title on exit")
         (common::kConsoleTitle.c_str(), po::value<string>()->default_value(consoleTitle), "set the console title")
         (common::kHelp.c_str(), "produce help message")
         (common::kInstanceId.c_str(), po::value<string>(), "a uuid identifying the specific instance of the application (there may be multiple copies running)")
         ;

      // If we are not the broker, then create a Websocket object to connect to it
      if (!boost::iequals(appId_, appIdBroker)) {
         brokerWS_ = make_shared<ix::WebSocket>();
         brokerWS_->setOnMessageCallback([this, fyi](ix::WebSocketMessagePtr const &msg) {
            try
            {
               if (fyi) {
                  fyi->onBrokerMessage(brokerWS_, msg);
               }
            }
            catch (const std::exception &e)
            {
               log_exception(e);
            }
         });
      }
   }
   
   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::importFile(path const &cfgFilename) {
      if (cfgFilename.empty() || !exists(cfgFilename.string())) {
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }

      try
      {
         str_debug() << "reading configuration file " << cfgFilename << endl;
         auto parsedFile = po::parse_config_file<char>(cfgFilename.string().c_str(), *options_);
         po::store(parsedFile, *values_, true);
      }
      catch (const std::exception &e)
      {
         log_exception(e);
         return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::importCommandline(int argc, char *argv[]) {

      try
      {
         auto parsedCmdline = po::parse_command_line(argc, argv, *options_);
         po::store(parsedCmdline, *values_, true);
      }
      catch (const std::exception &e)
      {
         log_exception(e);
         return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::notify() {
      po::notify(*values_);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::showHelp(string const &exeName
         , function<void()> showBanner) {

      MikadoLog::MikadoLogger.setOutputODS(false);
      MikadoLog::MikadoLogger.setOutputStdout(true);
      showBanner();
  
      str_notice() << "Usage: " << exeName << " [options]" << endl << endl
         << "Where [options] are:" << endl
         << description() << endl
         << "Hit Ctrl-Break to stop" << endl;

      // Make the program exit, though this is not an error
      return MikadoErrorCode::MKO_STATUS_NOOP;
   }

   ///////////////////////////////////////////////////////////////////////////
   // Signing on to the broker
   //
   MikadoErrorCode Configure::processBrokerMessage(ix::WebSocketMessagePtr const &msg) {
      switch (msg->type) {
      case ix::WebSocketMessageType::Open:
         str_debug() << "Broker connection is open" << endl;
         return MikadoErrorCode::MKO_STATUS_NOOP;

      case ix::WebSocketMessageType::Close:
         str_debug() << "Broker connection is closed" << endl;
         return MikadoErrorCode::MKO_ERROR_WEBSOCKET;

      case ix::WebSocketMessageType::Message:
      {
         // We have received a message from the broker, use it to reinitialize the configuration
         istringstream istrm(msg->str);
         values_ = make_shared<po::variables_map>();
         po::store(po::parse_config_file(istrm, *options_), *values_, true);
         notify();
         return MikadoErrorCode::MKO_STATUS_BROKER_AVAILABLE;
      }

      default:
         str_error() << "Received a non-message from the broker; type=" << msg->type << endl;
         break;
      }
      return MikadoErrorCode::MKO_ERROR_WEBSOCKET;
   }

   ///////////////////////////////////////////////////////////////////////////
   // Check to see if the broker is available to retrieve the configuration
   //
   MikadoErrorCode Configure::checkBroker() {

      MikadoErrorCode status = MikadoErrorCode::MKO_STATUS_WAITING;

      // Save the broker settings, because we will be reinitializing the configuration
      // if we successfully connect to the broker
      brokerHost_ = get<string>("broker-host");
      brokerPort_ = get<int>("broker-port");
      brokerTimeout_ = get<unsigned>("broker-timeout");

      if (brokerHost_.empty() || (0 == brokerPort_)) {
         brokerWS_.reset();
         str_info() << "Broker host and/or port not set in configuration" << endl;
      }
      if (!brokerWS_) {
         // We're not using the broker, so carry on with the configuration that we have
         return MikadoErrorCode::MKO_STATUS_NOBROKER;
      }

      string url { "ws://" + get<string>("broker-host") + ":" + to_string(get<int>("broker-port")) };
      brokerWS_->setUrl(url);
      brokerWS_->disablePerMessageDeflate();   // It's on the same machine, so we don't need this

      // Get ready to receive any response from the broker
      brokerWS_->setOnMessageCallback([this, &status](ix::WebSocketMessagePtr const &msg) {
         if (auto rc = processBrokerMessage(msg); MKO_IS_ERROR(rc)) {
            str_error() << "Failed to process broker message, error = " << rc << endl;
            status = rc;
         }
      });
      
      // Try to connect
      brokerWS_->start();

      // Tell the broker who we are
      json::value hail = { { "appid", appId_ } };
      brokerWS_->send(serialize(hail));

      // Wait for the broker to respond
      auto startAt = boost::posix_time::second_clock::local_time();
      while ((status == MikadoErrorCode::MKO_STATUS_WAITING)
         && ((boost::posix_time::second_clock::local_time() - startAt).total_seconds() < brokerTimeout_)) {

         // Give the broker enough time to respond to our hail
         Sleep(50);
      }

      return MikadoErrorCode::MKO_STATUS_BROKER_AVAILABLE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::defaultProcessing(int argc, char *argv[]
      , function<void()> showBanner, bool useBroker) {

      try
      {
         // If the configuration file exists, we read it first
         //path exePath(argv[0]);
         boost::dll::fs::error_code ec;
         path exePath(boost::dll::program_location(ec).string());
         path exeName{ exePath.filename() };
         path cfgFilename{ exePath.parent_path() / "cfg" / exeName };
         cfgFilename.replace_extension(".cfg");

         if (auto rc = importFile(cfgFilename); MKO_IS_ERROR(rc)) {
            str_error() << "Failed to import configuration file " <<
               cfgFilename << ", error = " << rc << endl;
            return rc;
         }

         // Then we read the command line
         if (auto rc = importCommandline(argc, argv); MKO_IS_ERROR(rc)) {
            str_error() << "Failed to import command line, error = " << rc << endl;
            return rc;
         }

         // Then we notify the options
         notify();
      
         // If anything asks for help, that's the only thing we do
         if (has(common::kHelp)) {
            return showHelp(exeName.string(), showBanner);
         }

         // If we are not the broker, see if we can connect to one
         if (useBroker && !boost::iequals(appId_, common::appIdBroker)) {
            // If the command-line includes broker connections, then we connect to the broker
            // to get a new configuration
            auto rc = checkBroker();
            if (MKO_IS_ERROR(rc)) {
               str_error() << "fatal error, trying to contact the broker, error = " << rc << endl;
               return rc;
            }
            if (MikadoErrorCode::MKO_STATUS_BROKER_AVAILABLE == rc) {
               str_info() << "broker has responded, configuration updated" << endl;
               // We have received a new configuration from the broker
               return MikadoErrorCode::MKO_STATUS_BROKER_CONFIGURATION;
            }
            else if (MikadoErrorCode::MKO_STATUS_NOBROKER == rc) {
               str_info() << "broker is not configured, using local configuration" << endl;
               // We have received a new configuration from the broker
               return MikadoErrorCode::MKO_STATUS_NOBROKER;
            }
         }
      }
      catch (const std::exception &e)
      {
         str_error() << e.what() << endl;
         return MikadoErrorCode::MKO_ERROR_INVALID_CONFIG;
      }

      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::common
