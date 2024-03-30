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
   Configure::Configure(string const &appId, string const &consoleTitle)
      : appId_ { appId }
      , options_{ make_shared<po::options_description>() }
      , values_{ make_shared<po::variables_map>() } {

      // Add the options that are defined for all Mikado apps
      addOptions()
         (common::poBrokerHost.c_str(), po::value<string>()->default_value("127.0.0.1"), "broker host")
         (common::poBrokerPort.c_str(), po::value<int>()->default_value(22304), "broker port")
         (common::poBrokerTimeout.c_str(), po::value<unsigned>()->default_value(2), "broker timeout in seconds")
         (common::poConsoleTitle.c_str(), po::value<string>()->default_value(consoleTitle), "set the console title")
         (common::poConsoleRestoreOnExit.c_str(), po::value<bool>()->default_value(false), "save and restore the title on exit")
         (common::poHelp.c_str(), "produce help message")
         ;
   }
   
   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::importFile(path const &cfgFilename) {
      if (!exists(cfgFilename.string())) {
         return MikadoErrorCode::MKO_ERROR_PATH_NOT_FOUND;
      }

      auto parsedFile = po::parse_config_file<char>(cfgFilename.string().c_str(), *options_);
      po::store(parsedFile, *values_, true);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Configure::importCommandline(int argc, char *argv[]) {
      auto parsedCmdline = po::parse_command_line(argc, argv, *options_);
      po::store(parsedCmdline, *values_, true);
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
   // Check to see if the broker is available to retrieve the configuration
   //
   MikadoErrorCode Configure::checkBroker() {

      if (!has(common::poBrokerHost) && !has(common::poBrokerPort)) {
         // We're not using the broker, so carry on with the configuration that we have
         return MikadoErrorCode::MKO_ERROR_NONE;
      }

      MikadoErrorCode status = MikadoErrorCode::MKO_STATUS_WAITING;

      // Save the broker settings, because we will be reinitializing the configuration
      // if we successfully connect to the broker
      brokerHost_ = get<string>("broker-host");
      brokerPort_ = get<int>("broker-port");
      brokerTimeout_ = get<unsigned>("broker-timeout");

      broker_ = make_shared<ix::WebSocket>();
      string url { "ws://" + get<string>("broker-host") + ":" + to_string(get<int>("broker-port")) };
      broker_->setUrl(url);
      broker_->disablePerMessageDeflate();   // It's on the same machine, so we don't need this

      // Get ready to receive any response from the broker
      broker_->setOnMessageCallback([this, &status](ix::WebSocketMessagePtr const &msg) {
         if (msg->type != ix::WebSocketMessageType::Message) {
            str_warn() << "Received a non-message (type=" << msg->type
               << ", from the broker, will use local configuration" << endl;
            return;
         }

         // We have received a message from the broker, use it to reinitialize the configuration
         istringstream istrm(msg->str);
         values_ = make_shared<po::variables_map>();
         po::store(po::parse_config_file(istrm, *options_), *values_, true);
         notify();

         status = MikadoErrorCode::MKO_STATUS_BROKER_AVAILABLE;
         });
      
      // Try to connect
      broker_->start();

      // Tell the broker who we are
      json::value hail = { { "appid", appId_ } };
      broker_->send(serialize(hail));

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
      , function<void()> showBanner) {

      try
      {
         // If the configuration file exists, we read it first
         path exePath(argv[0]);
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
         if (has(common::poHelp)) {
            return showHelp(exeName.string(), showBanner);
         }

         // If we are not the broker, see if we can connect to one
         if (!boost::iequals(appId_, common::appIdBroker)) {
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
