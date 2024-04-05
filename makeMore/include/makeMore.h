#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(MKO_MAKEMORE_H)
#define MKO_MAKEMORE_H

#include "common/fyi.h"
#include "common/globals.h"

namespace mikado::makeMore {

   unsigned int const MAKEMORE_VERSION_MAJOR = 0;
   unsigned int const MAKEMORE_VERSION_MINOR = 1;

   class MakeMore : public std::enable_shared_from_this<MakeMore>, public common::BrokerFYI {
   public:
      static void outputBanner();

      common::MikadoErrorCode configure(common::ConfigurePtr cfg);
      common::MikadoErrorCode start();
      common::MikadoErrorCode stop();

   protected:
      void onBrokerMessage(common::WebSocketPtr broker, ix::WebSocketMessagePtr const &msg);

   private:
      common::WebSocketPtr broker_;
   };
    
} // namespace mikado::makeMore

#endif // MKO_MAKEMORE_H
