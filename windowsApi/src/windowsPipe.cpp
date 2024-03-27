// Copyright (c) 2024 Gamaliel Ltd

#include "common/errorCodes.h"
#include "common/logger.h"
#include "windowsApi/windowsPipe.h"

using namespace std;

namespace mikado::windowsApi {

   // To save typing
   namespace common = mikado::common;
   typedef common::MikadoLog MikadoLog;
   typedef common::MikadoErrorCode MikadoErrorCode;

   ///////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode WindowsPipe::create(int *errorCode, bool inheritable, source_location const &loc) {
      // Inheritable pipe handles ?
      SECURITY_ATTRIBUTES saAttr;
      ZeroMemory(&saAttr, sizeof(saAttr));
      saAttr.nLength = sizeof(saAttr);
      saAttr.bInheritHandle = inheritable ? TRUE : FALSE;
      saAttr.lpSecurityDescriptor = NULL;

      read_ = make_shared<WindowsHandle>(getDebugName() + ".read-pipe");
      write_ = make_shared<WindowsHandle>(getDebugName() + ".write-pipe");
      if (!CreatePipe(read_->ptrHANDLE(), write_->ptrHANDLE(), &saAttr, 0)) {
         if (errorCode) {
            *errorCode = GetLastError();
         }
         close();
         return MikadoErrorCode::MKO_ERROR_CREATE_PIPE;
      }

      if (!setHandleInformation(read_, errorCode, loc)
         || !setHandleInformation(write_, errorCode, loc)) {
         return MikadoErrorCode::MKO_ERROR_SET_HANDLE_INFORMATION;
      }
      return MikadoErrorCode::MKO_ERROR_NONE;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   bool WindowsPipe::setHandleInformation(WindowsHandlePtr handle
         , int *errorCode, source_location const &loc) {
      if (SetHandleInformation(*handle, HANDLE_FLAG_INHERIT, 0)) {
         return true;
      }
      int rc = GetLastError();
      if (errorCode) {
         *errorCode = rc;
      }
      str_error(loc) << "failed (" << rc << ") to set handle information for "
         << handle->getDebugName() << endl;
      return false;
   }

   ///////////////////////////////////////////////////////////////////////
   //
   void WindowsPipe::close() {
      if (read_) {
         read_->close();
         read_.reset();
      }
      if (write_) {
         write_->close();
         write_.reset();
      }
   }

} // namespace mikado::windowsApi
