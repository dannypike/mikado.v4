#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(MKO_WINDOWS_STACK_TRACE_H)
#define MKO_WINDOWS_STACK_TRACE_H

using namespace std;

namespace mikado::windowsApi {

   class StackTrace {
   public:
      StackTrace(size_t skipFrames = 0, size_t maxFrames = 10, size_t maxSymbolLength = 256);
      std::string toString(std::string_view indent);
      bool failed() { return !exceptionWhat_.empty(); }
      std::string exceptionWhat() { return exceptionWhat_; }

   private:
      std::vector<std::string> stack_;
      std::string exceptionWhat_;
   };

} // namespace mikado::windowsApi

#endif // MKO_WINDOWS_STACK_TRACE_H
