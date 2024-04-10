#pragma once
#if !defined(API_WINDOWS_PIPE_H)
#define API_WINDOWS_PIPE_H

#include "windowsApi/handle.h"

namespace mikado::windowsApi {

   class WindowsPipe : public std::enable_shared_from_this<WindowsPipe>
   {
      typedef mikado::common::MikadoErrorCode MikadoErrorCode; // To reduce typing

   public:
      WindowsPipe(std::string const &debugName) {
         debugName_ = debugName;
      }
      ~WindowsPipe() {
         close();
      }
      std::string const &getDebugName() const {
         return debugName_;
      }

      MikadoErrorCode create(int *errorCode = nullptr, bool inheritable = true
         , std::source_location const &loc = std::source_location::current());
      void close();

      bool Read(void* buffer, size_t size, size_t& bytesRead);
      bool Write(const void* buffer, size_t size, size_t& bytesWritten);

      WindowsHandlePtr GetReadHandle() const {
         return read_;
      }
      WindowsHandlePtr GetWriteHandle() const {
         return write_;
      }

   protected:
      bool setHandleInformation(WindowsHandlePtr handle, int *errorCode, std::source_location const &loc);

   private:
      std::string debugName_;
      WindowsHandlePtr read_;
      WindowsHandlePtr write_;
   };

   typedef std::shared_ptr<WindowsPipe> WindowsPipePtr;

} // namespace mikado::windowsApi

#endif // API_WINDOWS_PIPE_H