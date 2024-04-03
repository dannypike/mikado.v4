// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "windowsApi.h"
#include "broker.h"

namespace broker = mikado::broker;
namespace common = mikado::common;
namespace windowsApi = mikado::windowsApi;
using namespace std;

typedef common::MikadoErrorCode MikadoErrorCode;

//////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
   auto rc = common::commonInitialize(argc, argv, mikado::broker::outputBanner);
   if (MKO_IS_ERROR(rc)) {
      return (int)rc;
   }
   if (auto rc = windowsApi::apiInitialize(argc, argv); MKO_IS_ERROR(rc)) {
      return (int)rc;
   }

   auto exitCode = mikado::broker::main(argc, argv);
   assert(STATUS_PENDING != (int)exitCode);   // Not allowed to return 259 from any process in Windows, as it is reserved for the system.

   windowsApi::apiShutdown();
   common::commonShutdown();

   str_info() << "exiting with code " << exitCode << endl;
   return (int)exitCode;
}
