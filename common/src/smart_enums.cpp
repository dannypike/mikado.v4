// Copyright () 2024 Gamaliel Ltd
#include "common/errorCodes.h"

using namespace std;

namespace mikado::common {

   ostream &operator<<(ostream &os, enum MikadoErrorCode code) {
      auto sCode = MikadoErrorCode_traits::to_string_or_empty(code);
      auto vCode = MikadoErrorCode_traits::to_underlying(code);
      return os << sCode << "(" << vCode << ")";
   }

} // namespace mikado::common

namespace ix {

   ostream &operator<<(ostream &os, WebSocketMessageType code) {
      auto sCode = WebSocketMessageType_traits::to_string_or_empty(code);
      auto vCode = WebSocketMessageType_traits::to_underlying(code);
      return os << sCode << "(" << vCode << ")";
   }

} // namespace ix
