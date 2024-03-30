#pragma once
#if !defined(GLOBBER_H)

namespace mikado::globber {

   unsigned int const GLOBBER_VERSION_MAJOR = 0;
   unsigned int const GLOBBER_VERSION_MINOR = 1;

   class Globber : public common::BrokerFYI {
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

} // namespace mikado::globber

#endif // GLOBBER_H
