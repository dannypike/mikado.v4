#pragma once
// Copyright (c) 2024 Gamaliel Ltd
#if !defined(BRK_APP_H)
#define BRK_APP_H

#include "broker/appInstanceId.h"
#include "broker/appId.h"

namespace mikado::broker {

   class App : public std::enable_shared_from_this<App> {
      typedef common::MikadoErrorCode MikadoErrorCode;

   public:
      App(AppId const &appId, AppInstanceId const &instanceId);
      ~App();

      MikadoErrorCode start();
      MikadoErrorCode stop();
      MikadoErrorCode setWebSocket(common::WebSocketPtr ws) {
         ws_ = ws;
         return MikadoErrorCode::MKO_ERROR_NONE;
      }
      common::WebSocketPtr getWebSocket() const {
         return ws_;
      }
      AppId appId() const {
         return appId_;
      }
      AppInstanceId instanceId() const {
         return instanceId_;
      }
      void setProcess(windowsApi::WindowsProcessPtr process) {
         process_ = process;
      }
      windowsApi::WindowsProcessPtr getProcess() const {
         return process_;
      }

   private:
      AppId appId_;
      AppInstanceId instanceId_;
      windowsApi::WindowsProcessPtr process_;
      common::WebSocketPtr ws_;
   };
   typedef std::shared_ptr<App> AppPtr;

} // namespace mikado::broker

#endif // BRK_APP_H
