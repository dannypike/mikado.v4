#pragma once
#if !defined(STORAGE_H)

#include "common/fyi.h"
#include "common/globals.h"

namespace mikado::storage {

   unsigned int const STORAGE_VERSION_MAJOR = 0;
   unsigned int const STORAGE_VERSION_MINOR = 1;

   class Storage : public common::BrokerFYI {
   public:
      common::MikadoErrorCode start() {
         return common::MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
      }
      void stop() {
      }

   protected:
      void onBrokerMessage(common::WebSocketPtr broker, ix::WebSocketMessagePtr const &msg);

   private:
      common::WebSocketPtr broker_;
   };
    
} // namespace mikado::storage

#endif // STORAGE_H
