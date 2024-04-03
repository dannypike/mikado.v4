// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "broker/handler.h"

namespace common = mikado::common;
namespace po = boost::program_options;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;

namespace mikado::broker {

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Handler::configureHandler(common::ConfigurePtr cfg) {
      port_ = cfg->get<int>(common::kBrokerPort);
      interface_ = cfg->get<string>(common::kBrokerHost, interface_);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Handler::initializeHandler() {

      server_ = make_shared<ix::WebSocketServer>(port_, interface_.c_str());
      if (auto result = server_->listen(); !result.first) {
         str_error() << "failed to listen on " << interface_
            << ":" << port_ << " - " << result.second << endl;
         return MikadoErrorCode::MKO_ERROR_SERVER_LISTEN;
      }

      server_->setOnClientMessageCallback(
         [this](ConnectionStatePtr state, ix::WebSocket &ws, ix::WebSocketMessagePtr const &msg) {
            try
            {
               onClientMessageCallback(state, ws, msg);
            }
            catch (const exception &e)
            {
               log_exception(e);
            }
         });

      MikadoErrorCode exitCode = MikadoErrorCode::MKO_ERROR_NONE;
      server_->start();
      return exitCode;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::shutdown() {
      // Shut down the server
      if (server_) {
         server_->stop();
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onConnectionCallback(weak_ptr<ix::WebSocket> webSocket
         , ConnectionStatePtr state) {
      stringstream ss;
      ss << "Remote ip: " << state->getRemoteIp() << endl;

      // Register with the client's socket to receive messages
      if (auto ws = webSocket.lock(); ws)
      {
         ws->setOnMessageCallback(
            [ws, state, this](ix::WebSocketMessagePtr const &msg) {
               try
               {
                  onClientMessageCallback(state, *ws, msg);
               }
               catch (const std::exception &e)
               {
                  log_exception(e);
               }
            });
      }
      else {
         str_warn() << "websocket pointer is invalid" << endl;
         return;
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onClientMessageCallback(ConnectionStatePtr state
         , ix::WebSocket &ws, ix::WebSocketMessagePtr const &msg) {

      switch (msg->type) {
      case ix::WebSocketMessageType::Error:
         onError(ws, state, msg);
         break;

      case ix::WebSocketMessageType::Open:
         onOpen(ws, state, msg);
         break;

      case ix::WebSocketMessageType::Close:
         onClose(ws, state, msg);
         break;

      case ix::WebSocketMessageType::Ping:
         ws.sendText("pong");
         break;

      case ix::WebSocketMessageType::Pong:
         ws.sendText("pong");
         break;

      case ix::WebSocketMessageType::Message:
         onMessage(ws, state, msg);
         break;

      default:
         str_error() << "unknown message type: " << (int)msg->type << endl;
         break;
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onError(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_error() << "websocket error: " << msg->str << endl;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onOpen(ix::WebSocket &ws, ConnectionStatePtr state
         , ix::WebSocketMessagePtr const &msg) {

      stringstream ss;
      ss << "New connection" << endl;

      // A connection state object is available, and has a default id
      // You can subclass ConnectionState and pass an alternate factory
      // to override it. It is useful if you want to store custom
      // attributes per connection (authenticated bool flag, attributes, etc...)
      ss << "id: " << state->getId() << endl;

      // The uri the client did connect to.
      ss << "Uri: " << msg->openInfo.uri << endl;

      ss << "Headers:" << endl;
      for (auto it : msg->openInfo.headers)
      {
         ss << it.first << ": " << it.second << endl;
      }
      str_info() << ss.rdbuf();
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onMessage(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_error() << "websocket msg: '" << msg->str << "'" << endl;
      MikadoErrorCode rc = MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onClose(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_error() << "websocket connection closed: " << msg->str << endl;
      MikadoErrorCode rc = MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

} // namespace mikado::broker
