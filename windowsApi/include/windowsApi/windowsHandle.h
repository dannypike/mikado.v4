#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(API_WINDOWS_HANDLE_H)

namespace mikado::windowsApi {

   class WindowsHandle : public std::enable_shared_from_this<WindowsHandle> {
   public :
      WindowsHandle(std::string const &debugName);
      WindowsHandle(HANDLE handle, std::string const &debugName);
      WindowsHandle(HANDLE *handle, std::string const &debugName);
      WindowsHandle(WindowsHandle &&other) noexcept;
      WindowsHandle(WindowsHandle const &) = delete;
      WindowsHandle& operator=(WindowsHandle const &) = delete;
      WindowsHandle& operator=(WindowsHandle &&other) noexcept;
      ~WindowsHandle();

      std::string const &getDebugName() const {
         return debugName_;
      }

      void attach(HANDLE *handle);
      void close();

      bool isOpen() const;
      operator HANDLE() const;
      HANDLE *ptrHANDLE();

   private:
      static std::recursive_mutex mux_;
      std::string debugName_;
      HANDLE handle_ = NULL;
      HANDLE *attached_ = nullptr;
      bool managed_ = false;
   };

   typedef std::shared_ptr<WindowsHandle> WindowsHandlePtr;

}  // namespace mikado::windowsApi

#endif // API_WINDOWS_HANDLE_H