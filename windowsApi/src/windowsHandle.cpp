// Copyright (c) Gamaliel Ltd

#include "windowsApi/WindowsHandle.h"

using namespace std;

namespace mikado::windowsApi {

   recursive_mutex WindowsHandle::mux_;

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::WindowsHandle(string const &debugName) {
      debugName_ = debugName;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::WindowsHandle(HANDLE handle, string const &debugName) {
      handle_ = handle;
      debugName_ = debugName;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::WindowsHandle(HANDLE *handle, string const &debugName) {
      attached_ = handle;
      managed_ = true;
      debugName_ = debugName;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::~WindowsHandle() {
      close();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::WindowsHandle(WindowsHandle &&other) noexcept {
      moveFrom(other);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle &WindowsHandle::operator=(WindowsHandle &&other) noexcept {
      moveFrom(other);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   void WindowsHandle::moveFrom(WindowsHandle &other) noexcept {
      scoped_lock lock(mux_);
      close();

      debugName_ = other.debugName_;
      other.debugName_.clear();

      handle_ = other.handle_;
      other.handle_ = NULL;

      managed_ = other.managed_;
      if (other.managed_) {
         other.managed_ = false;
         attached_ = other.attached_;
         other.attached_ = nullptr;
      }
      else {
         attached_ = nullptr;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   //
   void WindowsHandle::attach(HANDLE *ptrHandle) {
      std::scoped_lock lock(mux_);
      close();

      managed_ = true;
      attached_ = ptrHandle;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   void WindowsHandle::close() {
      std::scoped_lock lock(mux_);
      if (managed_) {
         managed_ = false;

         if (attached_ && (NULL != *attached_) && (INVALID_HANDLE_VALUE != *attached_)) {
            CloseHandle(*attached_);
            *attached_ = NULL;
         }
      } else if ((NULL != handle_) && (INVALID_HANDLE_VALUE != handle_)) {
         CloseHandle(handle_);
         handle_ = NULL;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   //
   bool WindowsHandle::isOpen() const {
      std::scoped_lock lock(mux_);
      HANDLE const *handle = managed_ ? attached_ : &handle_;
      return handle && (NULL != *handle) && (INVALID_HANDLE_VALUE != *handle);
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::operator HANDLE() const {
      scoped_lock lock(mux_);
      return managed_ ? (attached_ ? *attached_ : INVALID_HANDLE_VALUE) : handle_;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   HANDLE *WindowsHandle::ptrHANDLE() {
      scoped_lock lock(mux_);
      return managed_ ? attached_ : &handle_;
   }

} // namespace mikado::windowsApi
