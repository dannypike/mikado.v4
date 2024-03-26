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
      scoped_lock lock(mux_);
      debugName_ = other.debugName_;
      handle_ = other.handle_;
      attached_ = other.attached_;
      managed_ = other.managed_;

      other.handle_ = NULL;
      other.attached_ = NULL;
      other.managed_ = false;
      other.debugName_.clear();
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle &WindowsHandle::operator=(WindowsHandle &&other) noexcept {
      scoped_lock lock(mux_);
      close();

      debugName_ = other.debugName_;
      handle_ = other.handle_;
      attached_ = other.attached_;
      managed_ = other.managed_;

      other.handle_ = NULL;
      other.attached_ = NULL;
      other.managed_ = false;
      other.debugName_.clear();

      return *this;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   void WindowsHandle::attach(HANDLE *handle) {
      std::scoped_lock lock(mux_);
      close();

      managed_ = true;
      attached_ = handle;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   void WindowsHandle::close() {
      std::scoped_lock lock(mux_);
      if (managed_) {
         managed_ = false;

         if (attached_) {
            CloseHandle(*attached_);
            *attached_ = NULL;
         }
         attached_ = nullptr;
      } else {
         CloseHandle(handle_);
         handle_ = NULL;
      }
   }


   ///////////////////////////////////////////////////////////////////////
   //
   bool WindowsHandle::isOpen() const {
      std::scoped_lock lock(mux_);
      return handle_ != NULL;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   WindowsHandle::operator HANDLE() const {
      scoped_lock lock(mux_);
      return managed_ ? *attached_ : handle_;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   HANDLE *WindowsHandle::ptrHANDLE() {
      scoped_lock lock(mux_);
      return managed_ ? attached_ : &handle_;
   }

} // namespace mikado::common
