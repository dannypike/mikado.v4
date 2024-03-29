// Copyright (c) Gamaliel Ltd

#include "common.h"
#include "windowsApi/windowsGlobals.h"
#include "windowsApi/windowsHandle.h"

using namespace std;

namespace mikado::windowsApi {

   // To save typing
   namespace common = mikado::common;
   typedef common::MikadoErrorCode MikadoErrorCode;

   static string SaveConsoleTitle;

   //////////////////////////////////////////////////////////////////////////
   //
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

   //////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode apiInitialize(int argc, char *argv[]) {
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   //////////////////////////////////////////////////////////////////////////
   //
   static BOOL WINAPI consoleCtrlHandler(DWORD ctrlType) {
      switch (ctrlType) {
      case CTRL_C_EVENT:
      case CTRL_BREAK_EVENT:
      case CTRL_LOGOFF_EVENT:
      case CTRL_SHUTDOWN_EVENT:
      case CTRL_CLOSE_EVENT:
         common::MikadoShutdownRequested = true;
         return TRUE;

      default:
         return FALSE;
      }
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode apiSetupConsole(string const &consoleTitle
      , function<void()> showBanner) {

      // Don't output anything to the console. We do this now just in case there were
      // any errors in the configuration.
      common::MikadoLog::MikadoLogger.setOutputStdout(false);

      if (!consoleTitle.empty()) {
         // Save the current console title, so we can restore it at the end
         DWORD size = 16;
         char *buf = static_cast<char *>(malloc(1));
         while (((size <<= 1) < 100000)) {

            buf = static_cast<char *>(realloc(buf, size));
            if (auto len = GetConsoleTitle(buf, size); 0 != len) {
               SaveConsoleTitle.assign(buf, len);
               break;
            }
         };
         free(buf);

         // Then set the new console title
         SetConsoleTitle(consoleTitle.c_str());
      }

      if (showBanner) {
         showBanner();
      }

      // Stop the program on Ctrl-C
      SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   void apiShutdown() {
      // Restore the console title
      if (!SaveConsoleTitle.empty()) {
         SetConsoleTitle(SaveConsoleTitle.c_str());
      }
   }

} // namespace mikado::windowsApi
