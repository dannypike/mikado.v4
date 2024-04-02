#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_BROKER_CONNECTION_H)
#define CMN_BROKER_CONNECTION_H

namespace mikado::common {

   class BrokerConnection : public std::enable_shared_from_this<BrokerConnection> {
   public:
      BrokerConnection() = default;
      virtual ~BrokerConnection() = default;

      MikadoErrorCode connect(ConfigurePtr options);
      MikadoErrorCode processOutgoing();
      MikadoErrorCode processIncoming(ix::WebSocketMessagePtr const &msg);
      void shutdown();

   private:
      ix::WebSocket wsc_;
      std::string appId_;
      std::string instanceId_;

      typedef boost::lockfree::spsc_queue<std::string, boost::lockfree::capacity<1024>> TestQueue;
      TestQueue sendQueue_;
   };

} // namespace mikado::common

#endif // CMN_BROKER_CONNECTION_H
