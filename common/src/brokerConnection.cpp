// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"

namespace blf = boost::lockfree;
namespace json = boost::json;
using namespace std;
using namespace std::filesystem;

namespace mikado::common {

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode BrokerConnection::connect(ConfigurePtr options) {
      auto port = options->get<int>(kBrokerPort);
      auto host = options->get<string>(kBrokerHost);
      appId_ = options->get<string>(kAppId);
      instanceId_ = options->get<string>(kInstanceId);

      stringstream url;
      url << "ws://" << host << ":" << port << "/";
      url.flush();

      wsc_.setUrl(url.str());
      wsc_.setPingInterval(30);
      wsc_.disablePerMessageDeflate();
      wsc_.setOnMessageCallback(
         [this](ix::WebSocketMessagePtr const &msg) {
            try
            {
               processIncoming(msg);
            }
            catch (const std::exception &e)
            {
               log_exception(e);
            }
         });

      str_info() << "Connecting to " << wsc_.getUrl() << endl;
      wsc_.start();

      json::value jv = { {
            // The header
            {fieldVersion, MKO_WEBSOCKET_PROTOCOL_VERSION },
            {fieldType, actionConnect},
            {fieldAppId, appId_},
            {fieldInstanceId, instanceId_},
            {fieldTimestamp, formatTimestamp()}
         }, {
            // The data
            {fieldAction, actionConnect},
         }
      };
      auto text = serialize(jv);

      str_info() << "sign-on message will be: " << text << endl;
      sendQueue_.push(text);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   void BrokerConnection::shutdown() {
      if (wsc_.getReadyState() == ix::ReadyState::Open) {
         wsc_.stop();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode BrokerConnection::processOutgoing() {
      if (wsc_.getReadyState() == ix::ReadyState::Open) {
         if (0 < sendQueue_.consume_all(
               [this](string &text) {
                  str_info() << "sending: '" << text << "'" << endl;
                  wsc_.send(text);
               })) {
            return MikadoErrorCode::MKO_ERROR_NONE;
         }
      }
      return MikadoErrorCode::MKO_STATUS_NOOP;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode BrokerConnection::processIncoming(ix::WebSocketMessagePtr const &msg) {
      if (msg->type == ix::WebSocketMessageType::Open) {
         str_info() << "Connected to " << msg->openInfo.uri << endl;
      }
      else if (msg->type == ix::WebSocketMessageType::Close) {
         str_info() << "Disconnected from " << msg->closeInfo.reason << endl;
      }
      else if (msg->type == ix::WebSocketMessageType::Message) {
         str_info() << "Received message: " << msg->str << endl;
      }
      else if (msg->type == ix::WebSocketMessageType::Error) {
         str_error() << "Error: " << msg->errorInfo.reason << endl;
      }
      else {
         return MikadoErrorCode::MKO_STATUS_NOOP;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

} // namespace mikado::common
