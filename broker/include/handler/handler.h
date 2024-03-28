#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(HANDLER_H)

namespace mikado::broker {

   class Handler : public std::enable_shared_from_this<Handler> {
      typedef common::MikadoErrorCode MikadoErrorCode;
      typedef std::shared_ptr<ix::WebSocket> WebSocketPtr;
      typedef std::shared_ptr<ix::WebSocketServer> WebSocketServerPtr;
      typedef std::shared_ptr<ix::ConnectionState> ConnectionStatePtr;
      typedef std::shared_ptr<std::jthread> ThreadPtr;

   public:
      Handler() = default;
      ~Handler() = default;

      MikadoErrorCode configure(boost::program_options::variables_map const &cfg);
      MikadoErrorCode initialize();
      void shutdown();

   protected: 
      void onError(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);
      void onOpen(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);
      void onMessage(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);
      void onClose(ix::WebSocket &ws, ConnectionStatePtr state, ix::WebSocketMessagePtr const &msg);

   private:
      int port_ = 22304;
      std::string interface_ { "127.0.0.1" };
      WebSocketServerPtr server_;
      ThreadPtr serverThread_;
   };

   typedef std::shared_ptr<Handler> HandlerPtr;

} // namespace mikado::broker

#endif // HANDLER_H
