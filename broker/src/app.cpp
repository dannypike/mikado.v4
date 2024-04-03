// Copyright (c) 2024 Gamaliel Ltd

#include "common.h"
#include "windowsApi.h"
#include "broker/appInstanceId.h"
#include "broker/appId.h"
#include "broker/app.h"

using namespace std;

namespace mikado::broker {

   typedef common::MikadoErrorCode MikadoErrorCode;

   ///////////////////////////////////////////////////////////////////////////
   //
   App::App(AppId const &appId, AppInstanceId const &instanceId)
      : appId_(appId)
      , instanceId_(instanceId) {
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   App::~App() {
      stop();
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode App::start() {
      return MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

   ///////////////////////////////////////////////////////////////////////////
   //
   MikadoErrorCode App::stop() {
      return MikadoErrorCode::MKO_ERROR_NOT_IMPLEMENTED;
   }

} // namespace mikado::broker
