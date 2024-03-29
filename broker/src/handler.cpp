// Copyright (c) 2024 Gamaliel Ltd
#include "common.h"
//#include "windowsApi.h"
#include "handler/handler.h"

//namespace api = mikado::windowsApi;
namespace common = mikado::common;
namespace po = boost::program_options;
typedef common::MikadoErrorCode MikadoErrorCode;
using namespace std;

namespace mikado::broker {

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Handler::configure(common::ConfigurePtr cfg) {
      port_ = cfg->get<int>("port");
      interface_ = cfg->get<string>("interface", interface_);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode Handler::initialize() {

      server_ = make_shared<ix::WebSocketServer>(port_, interface_.c_str());
      if (auto result = server_->listen(); !result.first) {
         str_error() << "failed to list on port " << port_ << ", interface "
            << interface_ << ": " << result.second << endl;
         return MikadoErrorCode::MKO_ERROR_SERVER_LISTEN;
      }

      server_->setOnClientMessageCallback(
         [this](ConnectionStatePtr state, ix::WebSocket &ws, ix::WebSocketMessagePtr const &msg) {
            
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
         });

      // Run the monitor thread
      MikadoErrorCode exitCode = MikadoErrorCode::MKO_ERROR_NONE;
      serverThread_ = make_shared<jthread>(
         [](WebSocketServerPtr server, MikadoErrorCode *exitCode) {
            server->start();

            // Do we want to support pings from the client?
            //server->enablePong();

            server->wait();
            *exitCode = MikadoErrorCode::MKO_ERROR_NONE;
         }, server_, &exitCode);

      return exitCode;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::shutdown() {
      // Shut down the server
      if (server_) {
         server_->stop();
      }

      // Wait for the thread to join
      serverThread_.reset();
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onError(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_error() << "websocket error: " << msg->str << endl;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onOpen(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_info() << "websocket connection open: " << state->getId() << endl;

      // The uri the client did connect to.
      str_debug() << "Uri: " << msg->openInfo.uri << std::endl;

      str_debug() << "Headers:" << std::endl;
      for (auto it : msg->openInfo.headers)
      {
         str_debug() << "   " << it.first << ": " << it.second << std::endl;
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onMessage(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_error() << "websocket msg: '" << msg->str << "'" << endl;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void Handler::onClose(ix::WebSocket &ws
      , ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg) {
      str_error() << "websocket connection closed: " << msg->str << endl;
   }

} // namespace mikado::broker
