#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(CMN_FYI_H)
#define CMN_FYI_H

#include "common/globals.h"

namespace mikado::common {

   class BrokerFYI {
   public:
      virtual void onBrokerMessage(WebSocketPtr broker, ix::WebSocketMessagePtr const &msg) = 0;
   };

} // namespace mikado::common

#endif // CMN_FYI_H
