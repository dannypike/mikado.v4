// Copyright (c) Gamaliel Ltd

#include "common/algorithms.h"
#include "common/errorCodes.h"
#include "windowsApi/windowsGlobals.h"

using namespace std;

namespace mikado::windowsApi {

   // To save typing
   namespace common = mikado::common;
   typedef common::MikadoErrorCode MikadoErrorCode;

   string getLastErrorAsString(DWORD *lastError)
   {
      DWORD errorCode = lastError ? *lastError : GetLastError();
      if (errorCode == 0)
         return string(); //No error message has been recorded

      LPSTR messageBuffer = nullptr;
      size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

      string message(messageBuffer, size);
      LocalFree(messageBuffer);

      message = common::trim(message) + "<" + to_string(errorCode) + ">";
      return message;
   }

   MikadoErrorCode apiInitialize(int argc, char *argv[]) {
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   void apiShutdown() {
   }

} // namespace mikado::windowsApi
