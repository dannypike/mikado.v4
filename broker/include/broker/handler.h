#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BRK_HANDLER_H)
#define BRK_HANDLER_H

namespace mikado::broker {

   class Handler : public std::enable_shared_from_this<Handler> {
      typedef common::MikadoErrorCode MikadoErrorCode;
      typedef std::shared_ptr<ix::WebSocket> WebSocketPtr;
      typedef std::shared_ptr<ix::WebSocketServer> WebSocketServerPtr;
      typedef std::shared_ptr<ix::ConnectionState> ConnectionStatePtr;

   public:
      Handler() = default;
      ~Handler() = default;

      MikadoErrorCode configureHandler(common::ConfigurePtr cfg);
      MikadoErrorCode initializeHandler();
      void shutdown();
      std::string getUrl() const {
         return "ws://" + interface_ + ":" + std::to_string(port_);
      }

   protected: 
      void onConnectionCallback(std::weak_ptr<ix::WebSocket> webSocket, ConnectionStatePtr state);
      void onClientMessageCallback(ConnectionStatePtr state, ix::WebSocket &ws, ix::WebSocketMessagePtr const &msg);
      void onError(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);
      void onOpen(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);
      void onMessage(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);
      void onClose(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);

   private:
      int port_ = 0;
      std::string interface_;
      WebSocketServerPtr server_;
   };

   typedef std::shared_ptr<Handler> HandlerPtr;

} // namespace mikado::broker

#endif // BRK_HANDLER_H
