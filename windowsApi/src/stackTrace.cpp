#include "common.h"
#include "windowsApi/globals.h"
#include "windowsApi/stackTrace.h"

using namespace std;

namespace mikado::windowsApi {

   ///////////////////////////////////////////////////////////////////////////
   //
   StackTrace::StackTrace(size_t skipFrames, size_t maxFrames, size_t maxSymbolLength) {
      void **stackFrame = NULL;
      IMAGEHLP_LINE64 *line = NULL;
      SYMBOL_INFO *symbolBuffer = NULL;
      try
      {
         // DbgHelp is not thread-safe, so we need to synchronize access to it
         lock_guard<mutex> lock(DbgHelpMux);

         auto process = GetCurrentProcess();

         // Get the stackFrame frameCount
         line = (IMAGEHLP_LINE64 *)calloc(1, sizeof(IMAGEHLP_LINE64));
         line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

         stackFrame = (void **)malloc(maxFrames * sizeof(stackFrame));
         auto frameCount = CaptureStackBackTrace(skipFrames, maxFrames, stackFrame, NULL);

         // Get the symbols
         size_t bufferLen = sizeof(SYMBOL_INFO) + ((maxSymbolLength + 1) * sizeof(char));
         symbolBuffer = (SYMBOL_INFO *)malloc(bufferLen);
         symbolBuffer->SizeOfStruct = sizeof(SYMBOL_INFO);
         symbolBuffer->MaxNameLen = maxSymbolLength;

         // Walk up the stackFrame
         for (auto frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            ostringstream ss;
            DWORD displacement = 0;
            DWORD64 address = (DWORD64)(stackFrame[frameIndex]);

            if (SymFromAddr(process, address, 0, symbolBuffer)) {
               if (SymGetLineFromAddr64(process, address, &displacement, line)) {
                  ss << line->FileName << "(" << line->LineNumber << "): " << symbolBuffer->Name << "()";
                  stack_.emplace_back(ss.str());
                  continue;
               }
            }
            auto lastError = GetLastError();
            ss << "no file/line number info @ call #" << frameCount - frameIndex
               << "; GetLastError() is '" << getLastErrorAsString(&lastError) << "'";
            stack_.emplace_back(ss.str());
         }
      }
      catch (const std::exception &e)
      {
         exceptionWhat_ = e.what();
      }

      if (line) {
         free(line);
      }
      if (stackFrame) {
         free(stackFrame);
      }
      if (symbolBuffer) {
         free(symbolBuffer);
      }
   }
   
   ///////////////////////////////////////////////////////////////////////////
   //
   string StackTrace::toString(string_view indent) {
      ostringstream ss;
      for (auto &line : stack_) {
         ss << indent << line << endl;
      }
      return ss.str();
   }

} // namespace mikado::windowsApi
