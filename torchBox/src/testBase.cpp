#include "common.h"
#include "torchBox/testBase.h"

using namespace std;

namespace mikado::torchBox {

   typedef common::MikadoErrorCode MikadoErrorCode;

   ///////////////////////////////////////////////////////////////////////////
   //
   TestBase::TestBase(string const &testName) {
      testName_ = testName;
   }

} // namespace mikado::torchBox