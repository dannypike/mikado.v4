#include "common.h"
#include "torchBox/netBase.h"

using namespace std;

namespace mikado::torchBox {

   typedef common::MikadoErrorCode MikadoErrorCode;

   ///////////////////////////////////////////////////////////////////////////
   //
   NetBase::NetBase(string const &name) {
      name_ = name;
   }

} // namespace mikado::torchBox